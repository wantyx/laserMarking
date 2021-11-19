#include "laserMarking.h"
#include "qdebug.h"


using namespace std;

laserMarking::laserMarking(QWidget *parent)
    : QMainWindow(parent)
{
	setting = NULL;
    ui.setupUi(this);
	
	logSysInit("log.txt");//开启日志系统
	m_thread = new CaptureThread(this);
	scene = new QGraphicsScene(this);
	scene->setSceneRect(-ui.graphicsView->height()/2, -ui.graphicsView->height() / 2,ui.graphicsView->height(),ui.graphicsView->height());
	m1Item = NULL;
	//开启一个定时器
	m_showTimer = new QTimer(this);
	m_showTimer->setInterval(60);
	connect(m_showTimer, SIGNAL(timeout()), this, SLOT(updateShowText()));
	//初始化两个方式的通信
	m_pSerialPort = new QSerialPort;
	m_clientSocket = new QTcpSocket;

	//connect(ui.vision_position_btn, SIGNAL(clicked()), this, SLOT(get_position()));
	//绑定按钮所对应的执行函数
	connect(ui.setting_btn, SIGNAL(clicked()), this, SLOT(setting_net()));
	connect(ui.mark_test_btn, SIGNAL(clicked()), this, SLOT(mark_test()));
	connect(ui.take_pthoto_btn, SIGNAL(clicked()), this, SLOT(preview()));
	connect(m_thread, SIGNAL(captured(QImage)), this, SLOT(image_process(QImage)), Qt::QueuedConnection);
	connect(m_thread, SIGNAL(showWrongMessageInCameraThread(int)), this, SLOT(showWrongMessage(int)));
	connect(ui.stop_btn, SIGNAL(clicked()), this, SLOT(stop()));
	connect(ui.zoom_in_btn, SIGNAL(clicked()), this, SLOT(enlarge()));
	connect(ui.zoom_out_btn, SIGNAL(clicked()), this, SLOT(delarge()));
	connect(ui.resume_btn, SIGNAL(clicked()), this, SLOT(resetImg()));
	connect(ui.start_btn, SIGNAL(clicked()), this, SLOT(start()));
	connect(ui.count_clear_btn, SIGNAL(clicked()), this, SLOT(clear()));

}

laserMarking::~laserMarking()
{
	delete GlobalSetting::m_instance;
	m_thread->setTrigger(0);
}

/**
*打标测试 
* 1.通过piex2location坐标转换之后得到对应的标定板平面信息
* 2.测试是否连接上对应的下位机(打标机)，如果没有则连接并开启定时器
* 3.发送打标信息并清空当前帧的信息，等待定时器接收到markOK之后才能执行下一次的模板匹配工作
*/
void laserMarking::mark_test()
{
	try {
		int message = piex2location();
		switch (message)
		{
		case -1:throw 1;
		default:
			break;
		}
	}
	catch(int){
		if (gMLog) {
			gMLog->warning("graphItem is not loaded before method named piex2location");
		}
		return;
	}
	
	if(m_clientSocket->isOpen()) {
		//
	}
	else {
		if (gMLog) {
			gMLog->info("connect to %s:%s:", 
				GlobalSetting::instance()->getNetAddr().toStdString(),
				QString::number(GlobalSetting::instance()->getNetPort()).toStdString());
		}
		m_clientSocket->connectToHost(GlobalSetting::instance()->getNetAddr(), GlobalSetting::instance()->getNetPort());
		bool ret = m_clientSocket->waitForConnected(10000);
		if (ret) {
			m_showTimer->start();
		}
		else {
			QMessageBox::warning(this, tr("failed"), tr("please check if the port is the same!"));
			m_clientSocket->close();
			return;
		}
	}
	if (m_datalist.size() > 0) {
		int nCount = 0;
		QString sendString = "<T,";
		QString contentString = "";
		for (int i = 0; i < m_datalist.size(); i++) {
			nCount += 3;
			TargetData data = m_datalist.at(i);
			contentString += QString(",%1,%2,%3")
				.arg(data.centerX())
				.arg(data.centerY())
				.arg(data.angle());
		}
		sendString += QString::number(nCount) + contentString + "><X>";
		QByteArray sendArr = sendString.toLatin1();
		m_clientSocket->write(sendArr);
	}
	else {
		markOK = true;
	}
	numberMarked = m_datalist.size();
	ui.counts->setText(QString::number(numberMarked));
	m_datalist.clear();
	
}

/*
* 测试函数 
*/
void laserMarking::get_position()
{

	//string class_id = GlobalSetting::instance()->getTemplateName().toStdString();
	//string imgLocation = GlobalSetting::instance()->getTemplateImagePath().toStdString();
	//String saveTemplateDir = GlobalSetting::instance()->getSavePath().toStdString();
	//int padding = GlobalSetting::instance()->getPadding();
	//int num_feature = GlobalSetting::instance()->getFeatureNumber();
	string class_id = "gongjian1234";
	string imgLocation = "C:\\Users\\yangjie\\Desktop\\windows\\image.jpg";
	string saveTemplateDir = "C:\\Users\\yangjie\\Desktop\\windows\\";
	int padding = 25;
	int num_feature = 8;
	Rect roi(138, 620, 65, 35);
	//train(imgLocation, saveTemplateDir, num_feature, roi, padding, class_id);
	string testImgLocation = "C:\\Users\\yangjie\\Desktop\\windows\\image.jpg";
	vector<recognizedObjectLocation> ObjLocations;
	ObjLocations = test(testImgLocation, saveTemplateDir, num_feature, roi, padding, class_id);
	for (auto& i : ObjLocations) {
		TargetData targetData;
		targetData.setCenterX(i.center.x);
		targetData.setCenterY(i.center.y);
		targetData.setAngle(i.angle);
		m_datalist.push_back(targetData);
	}
	
}

/*
* 打开设置界面，此时必须关闭主界面的相机捕获线程，否则两个界面会抢占同一个相机，出现程序崩溃
*/
void laserMarking::setting_net()
{
	if (m_thread->isRunning()) {
		m_thread->stop();
		ui.take_pthoto_btn->setText(codec->toUnicode("预览"));
	}
	setting = new Setting();
	setting->show();
}

/*
* 坐标转换，从像素坐标系到标定板平面坐标系的转换函数
*/
int laserMarking::piex2location()
{
	Mat cameraIntri_inv = GlobalSetting::instance()->getCameraIntri().inv();
	double A_caliBoard = GlobalSetting::instance()->A_caliBoard;
	double B_caliBoard = GlobalSetting::instance()->B_caliBoard;
	double C_caliBoard = GlobalSetting::instance()->C_caliBoard;
	double D_caliBoard = GlobalSetting::instance()->D_caliBoard;

	/*Mat undistCenterPoint = Mat::zeros(3, 1, CV_64F);
	undistCenterPoint.at<double>(0, 0) = 1149;
	undistCenterPoint.at<double>(1, 0) = 207;
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
	TargetData targetData;
	targetData.setCenterX(result.at<double>(0));
	targetData.setCenterY(result.at<double>(1));

	m_datalist.push_back(targetData);
	int verify_x = (GlobalSetting::instance()->chessW) / 2 * GlobalSetting::instance()->chessV;
	int verify_y = (GlobalSetting::instance()->chessH) / 2 * GlobalSetting::instance()->chessV;
	if (GlobalSetting::instance()->Tranform == 0) {
		targetData.setCenterX((targetData.centerX() - verify_x));
		targetData.setCenterY(-(targetData.centerY() - verify_y));
	}
	else {
		targetData.setCenterX(-(targetData.centerX() - verify_x));
		targetData.setCenterY((targetData.centerY() - verify_y));
	}*/


	if (m1Item == NULL) return -1;
	if (m1Item->list.isEmpty()) return -2;
	for (auto i : m1Item->list) {
		Mat undistCenterPoint = Mat::zeros(3, 1, CV_64F);
		undistCenterPoint.at<double>(0, 0) = i.centerX();
		undistCenterPoint.at<double>(1, 0) = i.centerY();
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
		TargetData targetData;
		targetData.setCenterX(result.at<double>(0));
		targetData.setCenterY(result.at<double>(1));
		targetData.setAngle(i.angle());
		int verify_x = (GlobalSetting::instance()->chessW) / 2 * GlobalSetting::instance()->chessV;
		int verify_y = (GlobalSetting::instance()->chessH) / 2 * GlobalSetting::instance()->chessV;
		if (GlobalSetting::instance()->Tranform == 0) {
			targetData.setCenterX((targetData.centerX() - verify_x));
			targetData.setCenterY(-(targetData.centerY() - verify_y));
		}
		else {
			targetData.setCenterX(-(targetData.centerX() - verify_x));
			targetData.setCenterY((targetData.centerY() - verify_y));
		}
		targetData.setAngle(targetData.angle());
		m_datalist.push_back(targetData);
		if (gMLog) {
			gMLog->info("tragetData:%s,%s,%s", 
				QString::number(targetData.centerX()).toStdString(), 
				QString::number(targetData.centerY()).toStdString(),
				QString::number(targetData.angle()).toStdString()
				);
		}
	}
	
	return 0;
}

/*
* 相机预览与拍照功能
*/
void laserMarking::preview()
{
	if (m_thread->isRunning()) {
		m_thread->stop();
		ui.take_pthoto_btn->setText(codec->toUnicode("预览"));
		ui.realtime_display->setDisabled(true);
		ui.center_line->setDisabled(true);
		ui.marking_range->setDisabled(true);
		
	}
	else {
		m_thread->start();
		ui.take_pthoto_btn->setText(codec->toUnicode("拍照"));
		ui.realtime_display->setDisabled(false);
		ui.center_line->setDisabled(false);
		ui.marking_range->setDisabled(false);
		ui.mark_test_btn->setEnabled(true);
		ui.resume_btn->setEnabled(true);
		ui.zoom_in_btn->setEnabled(true);
		ui.zoom_out_btn->setEnabled(true);
	}
	
}

/*
* 相机捕获线程的槽函数，每一次捕获图片都会执行一次该函数
* 主要实现当前帧的模板匹配工作，同时判断中心线和打标范围的选择信息，ming主要实现当前槽函数必须结束之后才能再次触发，防止同一时间多次触发
*/
void laserMarking::image_process(QImage img)
{
	GlobalSetting::instance()->ming = false;
	imgshow = img;
	if (ui.realtime_display->checkState() == Qt::CheckState::Checked) {
		cvImg = matConvertQImage.QImage2cvMat(imgshow);
		imwrite("./test1111.bmp", cvImg);
		int featureNumber = GlobalSetting::instance()->getFeatureNumber();
		int padding = GlobalSetting::instance()->getPadding();
		int x = GlobalSetting::instance()->getROI().left();
		int y = GlobalSetting::instance()->getROI().top();
		int width = GlobalSetting::instance()->getROI().right() - x;
		int height = GlobalSetting::instance()->getROI().bottom() - y;
		Rect roi(x + imgshow.width() / 2, y + imgshow.height() / 2, width, height);
		std::string classid = GlobalSetting::instance()->getTemplateName().toStdString();
		
		vector<recognizedObjectLocation> ObjLocations;
		ObjLocations = test("./test1111.bmp", "./", featureNumber, roi, padding, classid);
		QPixmap ConvertPixmap = QPixmap::fromImage(imgshow);
		if (m1Item)
		{
			delete m1Item;
			m1Item = NULL;
		}
		m1Item = new MyGraphicsItem(&ConvertPixmap);
		for (auto& i : ObjLocations) {
			TargetData targetData;
			targetData.setTopleftX(i.topleft.x);
			targetData.setTopleftY(i.topleft.y);
			targetData.setCenterX(i.center.x);
			targetData.setCenterY(i.center.y);
			targetData.setAngle(i.angle);
			m1Item->list.push_back(targetData);
			
			//把所有的点放到vector中方便后面画图
			QPointF topleft = QPointF(i.topleft.x, i.topleft.y);
			QPointF topright = QPointF(i.topright.x, i.topright.y);
			QPointF bottomright = QPointF(i.bottomright.x, i.bottomright.y);
			QPointF bottomleft = QPointF(i.bottomleft.x, i.bottomleft.y);
			m1Item->resultPoints.push_back(topleft);
			m1Item->resultPoints.push_back(topright);
			m1Item->resultPoints.push_back(bottomright);
			m1Item->resultPoints.push_back(bottomleft);
		}
		//点击开始之后，进行打标
		if (started) {
			if (markOK) { //上一次的打标也结束了
		//延时
				markOK = false;
				
				QThread::sleep(ui.spinBox->value());
				mark_test();
			}
		}

	}
	else {
		QPixmap ConvertPixmap = QPixmap::fromImage(imgshow);
		if (m1Item)
		{
			delete m1Item;
			m1Item = NULL;
		}
		m1Item = new MyGraphicsItem(&ConvertPixmap);
	}
	if (ui.center_line->checkState() == Qt::CheckState::Checked) {
		m1Item->setCenterLine(true);
	}
	else
	{
		m1Item->setCenterLine(false);
	}

	if (ui.marking_range->checkState() == Qt::CheckState::Checked) {
		m1Item->setAroundLine(true);
		m1Item->setState(1);
	}
	else
	{
		m1Item->setAroundLine(false);
		m1Item->setState(1);
	}
	m1Item->setState(2);
	m1Item->update();
	showImage(ui.graphicsView, scene, m1Item);
	GlobalSetting::instance()->ming = true;
}

//停止按钮，修改相机的触发模式为连续
void laserMarking::stop()
{
	started = false;
	m_thread->setTrigger(0);
	//其余按钮全部enable
	ui.take_pthoto_btn->setEnabled(true);
	ui.resume_btn->setEnabled(true);
	ui.zoom_in_btn->setEnabled(true);
	ui.zoom_out_btn->setEnabled(true);
	ui.marking_range->setEnabled(true);
	ui.center_line->setEnabled(true);
	ui.realtime_display->setEnabled(true);
	ui.mark_test_btn->setEnabled(true);
	ui.setting_btn->setEnabled(true);
	ui.comboBox->setEnabled(true);
	ui.spinBox->setEnabled(true);
	ui.count_clear_btn->setEnabled(true);
	ui.start_btn->setEnabled(true);
}

void laserMarking::enlarge()
{
	if (m1Item) {
		m1Item->setmScale(m1Item->getScaleValue() * 1.1);
	}
}

void laserMarking::delarge()
{
	if (m1Item) {
		m1Item->setmScale(m1Item->getScaleValue() * 0.9);
	}
}

void laserMarking::resetImg()
{
	if (m1Item) {
		m1Item->ResetItemPos();
	}
}

/*
* 定时器触发函数，每隔一段时间去接受打标机的返回信息，用于接受<XE>
* XE表示上一次打标结束
*/
void laserMarking::updateShowText()
{
	QString readString;
	if (m_clientSocket->isOpen() && (m_clientSocket->bytesAvailable() > 0)) {
		readString = m_clientSocket->readAll();
	}
	if (readString == "<XE>") {
		qDebug() << "mark ok!";
		if (started) {
			ALL_numberMarked += numberMarked;
			ui.finish_count->setText(QString::number(ALL_numberMarked));
			if (ui.comboBox->currentIndex() != 6) {
				qDebug() << "sendSignalToPLC:port = com "<<ui.comboBox->currentIndex();
				sendSignalToPLC(ui.comboBox->currentIndex() + 1);
				qDebug() << "send successful!";
			}
		}
		markOK = true;//表示上一次打标结束了
	}
}

//开启整体流程，修改相机为触发模式
void laserMarking::start()
{


	ui.realtime_display->setDown(true);
	ui.realtime_display->setCheckState(Qt::CheckState::Checked);
	started = true;
	//其余按钮全部disable
	ui.take_pthoto_btn->setEnabled(false);
	ui.resume_btn->setEnabled(false);
	ui.zoom_in_btn->setEnabled(false);
	ui.zoom_out_btn->setEnabled(false);
	ui.marking_range->setEnabled(false);
	ui.center_line->setEnabled(false);
	ui.realtime_display->setEnabled(false);
	ui.mark_test_btn->setEnabled(false);
	ui.setting_btn->setEnabled(false);
	ui.comboBox->setEnabled(false);
	ui.spinBox->setEnabled(false);
	ui.count_clear_btn->setEnabled(false);
	ui.start_btn->setEnabled(false);
	m_thread->setTrigger(1);
}

void laserMarking::clear()
{
	ui.finish_count->setText(0);
}

void laserMarking::showWrongMessage(int _)
{
	QMessageBox::warning(this, tr("wrong"), tr("no camera find!"));
}

void laserMarking::showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item)
{
	int nwidth = qgraphicsView->width();
	int nheight = qgraphicsView->height();
	item->setQGraphicsViewWH(nwidth, nheight);
	qgraphicsScene->clear();
	qgraphicsScene->addItem(item);
	//qgraphicsView->setSceneRect(QRectF(-(nwidth / 2), -(nheight / 2), nwidth, nheight));
	qgraphicsView->setScene(qgraphicsScene);
}
