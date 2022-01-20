#include "Setting.h"
#include "qdebug.h"
#include "MatConvertQImage.h"
#include "globalsetting.h"
#include "pch.h"
using namespace std;
using namespace cv;

Setting::Setting(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setparam = NULL;
    myGraphicsItem = NULL;
    setAttribute(Qt::WA_DeleteOnClose);//关闭窗口的时候执行析构函数
    m_thread = new CaptureThread(this);
    m1Scene = new QGraphicsScene(this);
    m1Item = NULL;
    m1Scene->setSceneRect(-ui.graphicsView->height()/2, -ui.graphicsView->height()/2, ui.graphicsView->height(), ui.graphicsView->height());
    //scene->addItem(myGraphicsItem);

    GlobalSetting::instance()->setImgPath("./chess3.bmp");
    connect(ui.rec_btn, SIGNAL(clicked()), this, SLOT(checkChessPoints()));
    connect(ui.mark_set_param_btn, SIGNAL(clicked()), this, SLOT(showSetParam()));
    connect(ui.mark_btn, SIGNAL(clicked()), this, SLOT(calibrate()));
    connect(ui.preview, SIGNAL(clicked()), this, SLOT(calibrate_photo()));
    connect(m_thread, SIGNAL(captured(QImage)), this, SLOT(image_process(QImage)), Qt::QueuedConnection);
    connect(ui.take_photo_btn, SIGNAL(clicked()), this, SLOT(start()));
    connect(ui.apply_btn, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui.resume_btn, SIGNAL(clicked()), this, SLOT(resetImage()));
    connect(ui.mark_choose_btn, SIGNAL(clicked()), this, SLOT(saveTemplate()));
    connect(ui.mark_preview_btn, SIGNAL(clicked()), this, SLOT(preview()));
    connect(ui.drawLine_btn, SIGNAL(clicked()), this, SLOT(drawLine()));
    connect(ui.drwaRect_btn, SIGNAL(clicked()), this, SLOT(drawRect()));
    connect(ui.centerLine_btn, SIGNAL(clicked()), this, SLOT(startDrawCenterLine()));
    connect(ui.range_btn, SIGNAL(clicked()), this, SLOT(startDrawRect()));
    connect(ui.sure_center, SIGNAL(clicked()), this, SLOT(endDrawCenterLine()));
    connect(ui.sure_range, SIGNAL(clicked()), this, SLOT(endDrawRect()));
    connect(ui.clear1_btn, SIGNAL(clicked()), this, SLOT(clear1()));
    connect(ui.clear2_btn, SIGNAL(clicked()), this, SLOT(clear2()));
}

Setting::~Setting()
{
    if (setparam) {
        delete setparam;
        setparam = NULL;
    }
    if (m_thread->isRunning()) {
        m_thread->stop();
    }
}


void Setting::transform()
{
    //判断坐标原点在右上角还是左下角
    double data1[4] = { 0,0,0,1 };
    double data2[4] = { 50,40,0,1 };
    Mat point1 = Mat(4, 1, CV_64F, data1);
    Mat point2 = Mat(4, 1, CV_64F, data2);
    Mat cameraIntri = GlobalSetting::instance()->getCameraIntri();
    Mat RT = GlobalSetting::instance()->getRT();
    Mat RT_ = Mat(3, 4, CV_64F);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            RT_.at<double>(i, j) = RT.at<double>(i, j);
        }
    }
    //point1为原点
    Mat point1InCamera = cameraIntri * RT_ * point1;
    //point2为末点
    Mat point2InCamera = cameraIntri * RT_ * point2;
    double x0 = point1InCamera.at<double>(0) / point1InCamera.at<double>(2);
    double y0 = point1InCamera.at<double>(1) / point1InCamera.at<double>(2);
    double x1 = point2InCamera.at<double>(0) / point2InCamera.at<double>(2);
    double y1 = point2InCamera.at<double>(1) / point2InCamera.at<double>(2);
    if (x0 > x1 && y0 < y1) {//原点在右上角
        GlobalSetting::instance()->Tranform = 0;
    }
    else if (x0<x1 && y0>y1) {//原点在左下角
        GlobalSetting::instance()->Tranform = 1;
    }
    else if (x0 < x1 && y0 < y1) {//原点在左上
        GlobalSetting::instance()->Tranform = 2;
    }
    else if(x0 > x1 && y0 > y1) {//原点在右下
        GlobalSetting::instance()->Tranform = 3;
    }
}

void Setting::showSetParam() {
    if (setparam) {
        delete setparam;
        setparam = NULL;
    }
    setparam = new SetParam();
    setparam->show();
}

void _cal_pointInBoard(cv::Size boardSize, float squareSize, vector<cv::Point3f>& corners) {
    for (int i = 0; i < boardSize.height; i++) {
        for (int j = 0; j < boardSize.width; j++) {
            corners.push_back(cv::Point3f(float(j * squareSize), float(i * squareSize), 0));
        }
    }
}

void Setting::calibrate()
{
    int row = ui.rows->value();
    int col = ui.cols->value();
    float squarSize = ui.squarSize->value();
    GlobalSetting::instance()->chessW = row;
    GlobalSetting::instance()->chessH = col;
    GlobalSetting::instance()->chessV = squarSize;
    Size size(row, col);
    cv::Mat src = imread(GlobalSetting::instance()->getImgPath().toStdString());
    if (!src.data) {
        QMessageBox::warning(this, tr("wrong"), tr("no image find while calibrating!"));
        return;
    }
    Mat gray;
    cvtColor(src, gray, CV_BGR2GRAY);
    threshold(gray, gray, 128, 255, THRESH_BINARY_INV);
    medianBlur(gray, gray, 9);

    //以下代码进行标定工作
    Mat cameraIntri, distCoffe;
    vector<vector<Point2f>> pointBufs;
    vector<vector<Point3f>> objectPoints(1);
    //标定板到相机的RT矩阵board2camera_R
    vector<Mat> board2camera_R, board2camera_T;
    //标定板参数信息
    SimpleBlobDetector::Params params;
    float squareSize = squarSize;
    params.minArea = 30;
    params.maxArea = 18000;
    params.minDistBetweenBlobs = 5;
    params.filterByColor = true;
    params.blobColor = 0;
    params.filterByArea = true;
    Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);

    //通过照片进行标定
    Mat img = gray;
    vector<Point2f> pointBuf;
    Size imgSize = img.size();
    if (findCirclesGrid(img, size, pointBuf, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, blobDetector)) {
        pointBufs.push_back(pointBuf);
        drawChessboardCorners(gray, size, Mat(pointBuf), true);
        QImage QGrayImgae = matConvertQImage.Mat2QImage(gray);
        QPixmap ConvertPixmap = QPixmap::fromImage(QGrayImgae);
        if (m1Item)
        {
            delete m1Item;
            m1Item = NULL;
        }
        m1Item = new MyGraphicsItem(&ConvertPixmap);
        showImage(ui.graphicsView, m1Scene, m1Item);
        m1Item->update();
    }
    else {
        return;
    }
    _cal_pointInBoard(size, squareSize, objectPoints[0]);
    objectPoints.resize(pointBufs.size(), objectPoints[0]);
    calibrateCamera(objectPoints, pointBufs, imgSize, cameraIntri, distCoffe, board2camera_R, board2camera_T);
    //将标定的结果给globalSetting
    GlobalSetting::instance()->setCameraIntri(cameraIntri);
    GlobalSetting::instance()->setDistCoffe(distCoffe);
    
    Mat board2camera_R_angle;
    Rodrigues(board2camera_R.at(0), board2camera_R_angle);
    cout << board2camera_R_angle.size() << endl;
    Mat RT = Mat(3, 4, CV_64F);
    hconcat(board2camera_R_angle, board2camera_T.at(0), RT);
    double linear[4] = { 0,0,0,1 };
    Mat linearMat = Mat(1, 4, CV_64F, linear);
    Mat RTlinear = Mat(4, 4, CV_64F);
    vconcat(RT, linearMat, RTlinear);
    GlobalSetting::instance()->setRT(RTlinear);

    double r13 = board2camera_R_angle.at<double>(0, 2);
    double r23 = board2camera_R_angle.at<double>(1, 2);
    double r33 = board2camera_R_angle.at<double>(2, 2);
    double t1 = board2camera_T.at(0).at<double>(0);
    double t2 = board2camera_T.at(0).at<double>(1);
    double t3 = board2camera_T.at(0).at<double>(2);
    double A_caliBoard = r13;
    double B_caliBoard = r23;
    double C_caliBoard = r33;
    double D_caliBoard = r13 * t1 + r23 * t2 + r33 * t3;

    GlobalSetting::instance()->A_caliBoard = A_caliBoard;
    GlobalSetting::instance()->B_caliBoard = B_caliBoard;
    GlobalSetting::instance()->C_caliBoard = C_caliBoard;
    GlobalSetting::instance()->D_caliBoard = D_caliBoard;
    transform();//判断标定板的原点在右上角还是左下角，由此来实现转换到坐标中心。
}

void Setting::checkChessPoints()
{
    int row = ui.rows->value();
    int col = ui.cols->value();
    float squarSize = ui.squarSize->value();
    Size size(row, col);
    std::string path = GlobalSetting::instance()->getImgPath().toStdString();
    cv::Mat src = imread(GlobalSetting::instance()->getImgPath().toStdString());
    if (!src.data) {
        QMessageBox::warning(this, tr("wrong"), tr("no image find while calibrating!"));
        if (gMLog) {
            gMLog->warning("no image find while calibrating!");
        }
        return;
    }
    Mat gray;
    cvtColor(src, gray, CV_BGR2GRAY);
    threshold(gray, gray, 128, 255, THRESH_BINARY_INV);
    medianBlur(gray, gray, 9);
    //cv::imshow("?", gray);
    //cv::waitKey(0);
    SimpleBlobDetector::Params params;
    float squareSize = squarSize;
    params.minArea = 30;
    params.maxArea = 18000;
    params.minDistBetweenBlobs = 5;
    params.filterByColor = true;
    params.blobColor = 0;
    params.filterByArea = true;
    Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);
    Mat img = gray;
    vector<Point2f> pointBuf;
    if (findCirclesGrid(img, size, pointBuf, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, blobDetector)) {
        if (gMLog) {
            gMLog->info("ok! find the points in chess");
        }
        QImage QGrayImgae = matConvertQImage.Mat2QImage(gray);
        QPixmap ConvertPixmap = QPixmap::fromImage(QGrayImgae);
        if (m1Item)
        {
            delete m1Item;
            m1Item = NULL;
        }
        m1Item = new MyGraphicsItem(&ConvertPixmap);
        showImage(ui.graphicsView, m1Scene, m1Item);
        m1Item->update();
        return;
    }
    else {
        QMessageBox::warning(this, tr("wrong"), tr("conld not find the chessPoints!"));
        if (gMLog) {
            gMLog->warning("conld not find the chessPoints!");
        }
        return;
    }
}

void Setting::piex2location(double x,double y)
{
    Mat cameraIntri_inv = GlobalSetting::instance()->getCameraIntri().inv();
    double A_caliBoard = GlobalSetting::instance()->A_caliBoard;
    double B_caliBoard = GlobalSetting::instance()->B_caliBoard;
    double C_caliBoard = GlobalSetting::instance()->C_caliBoard;
    double D_caliBoard = GlobalSetting::instance()->D_caliBoard;

    Mat undistCenterPoint = Mat::zeros(3, 1, CV_64F);
    undistCenterPoint.at<double>(0, 0) = x;
    undistCenterPoint.at<double>(1, 0) = y;
    undistCenterPoint.at<double>(2, 0) = 1;
    Mat centerPointInCameraOXY = Mat::zeros(3, 1, CV_64F);
    centerPointInCameraOXY = cameraIntri_inv * undistCenterPoint;
    double a = centerPointInCameraOXY.at<double>(0, 0);
    double b = centerPointInCameraOXY.at<double>(1, 0);
    double X = D_caliBoard * a / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
    double Y = D_caliBoard * b / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
    double Z = D_caliBoard / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
    Mat pointInCamera = Mat(4, 1, CV_64F);
    pointInCamera.at<double>(0) = X;
    pointInCamera.at<double>(1) = Y;
    pointInCamera.at<double>(2) = Z;
    pointInCamera.at<double>(3) = 1;
    Mat RT_inv = GlobalSetting::instance()->getRT().inv();
    Mat result = RT_inv * pointInCamera;
    double result_x;
    double result_y;
    if (GlobalSetting::instance()->Tranform == 0) {
        result_x = result.at<double>(0) - 20;
        result_y = result.at<double>(1) - 25;
        result_x = -result_x;
        result_y = -result_y;
    }
    else {
        result_x = result.at<double>(0) - 20;
        result_y = result.at<double>(1) - 25;
    }
}

void Setting::calibrate_photo()
{
    if (m_thread->isRunning()) {
        m_thread->stop();
        ui.take_photo_btn->setText(codec->toUnicode("预览"));
    }
    QPixmap ConvertPixmap = QPixmap::fromImage(imgshow);
    if (m1Item)
    {
        delete m1Item;
        m1Item = NULL;
    }
    m1Item = new MyGraphicsItem(&ConvertPixmap);
    showImage(ui.graphicsView, m1Scene, m1Item);
    cv::Mat chessImg = matConvertQImage.QImage2cvMat(imgshow);
    GlobalSetting::instance()->setImgPath("./chess3.bmp");
    imwrite("./chess3.bmp", chessImg);
    if (gMLog) {
        gMLog->info("calibrate photo!");
    }
    m1Item->update();
}

void Setting::image_process(QImage img)
{
    
    imgshow = img;
    //pixmap = QPixmap::fromImage(imgshow);
    /*myGraphicsItem->m_pix = pixmap;*/
    //int nwidth = ui.graphicsView->width();
    //int nheight = ui.graphicsView->height();
    //myGraphicsItem->setQGraphicsViewWH(nwidth, nheight);
    ////myGraphicsItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    //scene->update();
    ////scene->addPixmap(QPixmap::fromImage(imgshow));
    //ui.graphicsView->setScene(scene);
    ////ui.graphicsView->repaint();
    //ui.graphicsView->show();

    QPixmap ConvertPixmap = QPixmap::fromImage(imgshow);
    if (m1Item)
    {
        delete m1Item;
        m1Item = NULL;
    }
    m1Item = new MyGraphicsItem(&ConvertPixmap);
    if (ui.center_line->checkState() == Qt::CheckState::Checked) {
        m1Item->setCenterLine(true);
    }
    else
    {
        m1Item->setCenterLine(false);
    }

    if (ui.mark_area->checkState() == Qt::CheckState::Checked) {
        m1Item->setAroundLine(true);
    }
    else
    {
        m1Item->setAroundLine(false);
    }

    showImage(ui.graphicsView, m1Scene, m1Item);
    m1Item->setState(2);
    m1Item->update();
}

void Setting::start()
{
    if (m_thread->isRunning())
    {
        m_thread->stop();
        ui.take_photo_btn->setText(codec->toUnicode("预览"));
    }
    else
    {
        m_thread->start();
        ui.take_photo_btn->setText(codec->toUnicode("拍照"));
    }
}

void Setting::apply()
{
    int exposureTime = ui.spinBox->value();
    bool result = m_thread->setExposureTime(exposureTime);
    if (result) {
        qDebug() << "ok";
    }
}

void Setting::setMyGraphicsitem()
{
}

void Setting::saveTemplate()
{
    if (m1Item == NULL) {
        return;
    }
    int padding = 0;
    QPointF startPosition = m1Item->getStartPos();
    QPointF endPosition = m1Item->getEndPos();
    QRect ROI;
    ROI.setTopLeft(startPosition.toPoint());
    ROI.setBottomRight(endPosition.toPoint());
    int x0 = startPosition.toPoint().x();
    int y0 = startPosition.toPoint().y();
    int x1 = endPosition.toPoint().x();
    int y1 = endPosition.toPoint().y();
    int distance = (int)pow(pow((x1 - x0), 2) + pow((y1 - y0), 2), 0.5);
    int xxyy = (y1 - y0) > (x1 - x0) ? (x1 - x0) : (y1 - y0);
    padding = (distance - xxyy) / 2;
    GlobalSetting::instance()->setPadding(padding);
    GlobalSetting::instance()->setROI(ROI);
    GlobalSetting::instance()->templateImage = matConvertQImage.QImage2cvMat(imgshow);
    cv::imwrite("./templateImage.bmp", matConvertQImage.QImage2cvMat(imgshow));
}

void Setting::resetImage()
{
    if (m1Item == NULL) {
        return;
    }
    m1Item->setmScale(m1Item->get_m_scaleDafault());
    m1Item->setQGraphicsViewWH(nwidth, nheight);
}

void Setting::preview()
{
    string class_id = GlobalSetting::instance()->getTemplateName().toStdString();
    //string imgLocation = GlobalSetting::instance()->getTemplateImagePath().toStdString();
    //String saveTemplateDir = GlobalSetting::instance()->getSavePath().toStdString();
    int padding = GlobalSetting::instance()->getPadding();
    int num_feature = GlobalSetting::instance()->getFeatureNumber();
    int x = GlobalSetting::instance()->getROI().left();
    int y = GlobalSetting::instance()->getROI().top();
    int width = GlobalSetting::instance()->getROI().right() - x;
    int height = GlobalSetting::instance()->getROI().bottom() - y;
    int scorenumber = GlobalSetting::instance()->getScoreNumber();
    Rect roi(x+imgshow.width()/2, y+imgshow.height()/2, width, height);
    //判断路径下是否存在文件
    cv::Mat temp = cv::imread("./templateImage.bmp");
    if (temp.data == NULL) {
        QMessageBox::warning(this, tr("wrong"), tr("No TemplateImage find in path"));
        return;
    }
    try {
        train("./templateImage.bmp", "./", num_feature, roi, padding, class_id);
    }
    catch (cv::Exception e) {
        if (gMLog) {
            gMLog->critical("templateMatching is error with something wrong!");
        }
    }
    
    vector<recognizedObjectLocation> ObjLocations;
    ObjLocations = test("./templateImage.bmp", "./", num_feature, roi, padding, class_id,scorenumber);
    for (auto& i : ObjLocations) {
        TargetData targetData;
        targetData.setTopleftX(i.topleft.x);
        targetData.setTopleftY(i.topleft.y);
        targetData.setCenterX(i.center.x);
        targetData.setCenterY(i.center.y);
        targetData.setAngle(i.angle);
        if (m1Item != NULL) {
            m1Item->list.push_back(targetData);
            QPointF topleft = QPointF(i.topleft.x, i.topleft.y);
            QPointF topright = QPointF(i.topright.x, i.topright.y);
            QPointF bottomright = QPointF(i.bottomright.x, i.bottomright.y);
            QPointF bottomleft = QPointF(i.bottomleft.x, i.bottomleft.y);
            m1Item->resultPoints.push_back(topleft);
            m1Item->resultPoints.push_back(topright);
            m1Item->resultPoints.push_back(bottomright);
            m1Item->resultPoints.push_back(bottomleft);
        }
        else {
            QMessageBox::warning(this, tr("wrong"), tr("There is no image for templateMatching"));
        }
    }
    if (m1Item != NULL) {
        m1Item->update();
    }
    
    //piex2location(2361, 1825);
}

//void Setting::getTemplePoints(){
//    读取特征点文件
//    cv::FileStorage fs("111_templ.yaml", FileStorage::READ);
//    对应的角度尺度文件
//    cv::FileStorage angle_fs("111_info.yaml", FileStorage::READ);
//    if (fs.isOpened() & angle_fs.isOpened() == 0) {
//        QMessageBox::warning(this, tr("wrong"), tr("failed read temple file"));
//    }
//    vector<int>followPoints;
//    Mat template_pyramids, infos;
//    fs["template_pyramids"] >> template_pyramids;
//    angle_fs["infos"] >> infos;
//    
//    
//
//    for (auto i : Range(0,template_pyramids.size())) {}
//    
//   
//
//    
//}


void Setting::showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item)
{
    nwidth = qgraphicsView->width();
    nheight = qgraphicsView->height();
    item->setQGraphicsViewWH(nwidth, nheight);
    qgraphicsScene->clear();
    qgraphicsScene->addItem(item);
    //qgraphicsView->setSceneRect(QRectF(-(nwidth / 2), -(nheight / 2), nwidth, nheight));
    qgraphicsView->setScene(qgraphicsScene);
    //qgraphicsView->setFocus();
}

void Setting::drawRect()
{
    if (m1Item) {
        m1Item->setState(2);
    }
    
}

void Setting::drawLine()
{
    if (m1Item) {
        m1Item->setState(1);
    }
}

void Setting::startDrawRect()
{
    if (m1Item) {
        m1Item->setDrawRange(true);
    }
    
}

void Setting::startDrawCenterLine()
{
    if (m1Item) {
        m1Item->setDrawCenterLine(true);
    }
    
}

void Setting::endDrawRect()
{
    if (m1Item) {
        m1Item->setDrawRange(false);
        GlobalSetting::instance()->vec_range.clear();
        for (int i = 0; i < m1Item->vec_range.size(); i++) {
            GlobalSetting::instance()->vec_range.push_back(m1Item->vec_range.at(i));
        }
    }
}

void Setting::endDrawCenterLine()
{
    if (m1Item) {
        m1Item->setDrawCenterLine(false);
        GlobalSetting::instance()->vec_centerLine.clear();
        for (int i = 0; i < m1Item->vec_centerLine.size(); i++) {
            GlobalSetting::instance()->vec_centerLine.push_back(m1Item->vec_centerLine.at(i));
        }
    }
}

void Setting::clear1()
{
    if (m1Item) {
        m1Item->vec_centerLine.clear();
        GlobalSetting::instance()->vec_centerLine.clear();
    }
    
}

void Setting::clear2()
{
    if (m1Item) {
        m1Item->vec_range.clear();
        GlobalSetting::instance()->vec_range.clear();
    }
}



