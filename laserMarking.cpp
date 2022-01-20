#include "laserMarking.h"
#include "qdebug.h"


using namespace std;

laserMarking::laserMarking(QWidget *parent)
    : QMainWindow(parent)
{

	registered = checkRegist();////////////检查注册
	setting = NULL;
    ui.setupUi(this);
	
	
	logSysInit("log.txt");//开启日志系统
	m_thread = new CaptureThread(this);
	scene = new QGraphicsScene(this);
	scene->setSceneRect(-ui.graphicsView->height()/2, -ui.graphicsView->height() / 2,ui.graphicsView->height(),ui.graphicsView->height());
	m1Item = NULL;
	//开启一个定时器
	m_showTimer = new QTimer(this);
	m_showTimer->setInterval(100);
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

	ui.dist->setValue(GlobalSetting::instance()->distance);
	ui.spinBox->setValue(GlobalSetting::instance()->sleepTime);
	ui.errorRange->setValue(GlobalSetting::instance()->errorRangeMax);
}

laserMarking::~laserMarking()
{
	delete GlobalSetting::m_instance;
	if (m_clientSocket->isOpen()) {
		m_clientSocket->close();
	}
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
		qDebug() << "m_clientSockert is open!";
	}
	else {
		m_clientSocket->connectToHost(GlobalSetting::instance()->getNetAddr(), GlobalSetting::instance()->getNetPort());
		//m_clientSocket->connectToHost("127.0.0.1", GlobalSetting::instance()->getNetPort());
		qDebug() << GlobalSetting::instance()->getNetAddr();
		qDebug() << GlobalSetting::instance()->getNetPort();
		bool ret = m_clientSocket->waitForConnected(2000);
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
		curTime2 = QDateTime::currentDateTime();
		int time = curTime1.msecsTo(curTime2);
		qDebug() << "between find the object and send to laserMarking marchine time:" << time;
	}
	else {
		markOK = true;
	}
	numberMarked = m_datalist.size();
	ui.counts->setText(QString::number(numberMarked));
	m_datalist.clear();
	
}


///*
//* 计算处理过程的时间
//*/
//void laserMarking::cal_time(){
//	
//	clock_t start, finish;
//	double duration;
//	
//	/*if (m_clientSocket->waitForConnected(10000)) {
//		start = clock();
//	}*/
//	if (ui.take_pthoto_btn) {
//		start = clock();
//	}
//	if (m_datalist.size() ){
//
//		finish = clock();
//	}
//
//	duration = (double)(finish - start)/ CLOCKS_PER_SEC;
//
//	//printf("%f seconds\n", duration);
//
//
//}
///*
//* 测试函数 
//*/
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

	//Mat undistCenterPoint = Mat::zeros(3, 1, CV_64F);
	//undistCenterPoint.at<double>(0, 0) = 1007;
	//undistCenterPoint.at<double>(1, 0) = 709;
	//undistCenterPoint.at<double>(2, 0) = 1;
	//Mat centerPointInCameraOXY = Mat::zeros(3, 1, CV_64F);
	//centerPointInCameraOXY = cameraIntri_inv * undistCenterPoint;
	//double a = centerPointInCameraOXY.at<double>(0, 0);
	//double b = centerPointInCameraOXY.at<double>(1, 0);
	//double X = D_caliBoard * a / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
	//double Y = D_caliBoard * b / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
	//double Z = D_caliBoard / (A_caliBoard * a + B_caliBoard * b + C_caliBoard);
	//Mat pointInCamera = Mat(4, 1, CV_64F);
	//pointInCamera.at<double>(0) = X;
	//pointInCamera.at<double>(1) = Y;
	//pointInCamera.at<double>(2) = Z;
	//pointInCamera.at<double>(3) = 1;
	//Mat RT_inv = GlobalSetting::instance()->getRT().inv();
	//Mat result = RT_inv * pointInCamera;

	//TargetData targetData;
	//targetData.setCenterX(result.at<double>(0));
	//targetData.setCenterY(result.at<double>(1));

	//m_datalist.push_back(targetData);
	//int verify_x = (GlobalSetting::instance()->chessW) / 2 * GlobalSetting::instance()->chessV;
	//int verify_y = (GlobalSetting::instance()->chessH) / 2 * GlobalSetting::instance()->chessV;
	//if (GlobalSetting::instance()->Tranform == 0) {
	//	targetData.setCenterX((targetData.centerX() - verify_x));
	//	targetData.setCenterY(-(targetData.centerY() - verify_y));
	//}
	//else {
	//	targetData.setCenterX(-(targetData.centerX() - verify_x));
	//	targetData.setCenterY((targetData.centerY() - verify_y));
	//}


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
		//将所有的情况全部转换到振镜2的坐标系下，采用振镜2如果行和列数输入正确，那么应该是Transform为2或者3的情况

		if (GlobalSetting::instance()->Tranform == 1) {
			targetData.setCenterX((targetData.centerX() - verify_x));
			targetData.setCenterY(-(targetData.centerY() - verify_y));
			targetData.setAngle(targetData.angle());
		}
		else if(GlobalSetting::instance()->Tranform == 0){
			targetData.setCenterX(-(targetData.centerX() - verify_x));
			targetData.setCenterY((targetData.centerY() - verify_y));
			targetData.setAngle(targetData.angle());
		}
		else if (GlobalSetting::instance()->Tranform == 2) {
			targetData.setCenterX(-(targetData.centerX() - verify_x));
			targetData.setCenterY(-(targetData.centerY() - verify_y));
			targetData.setAngle(360 - targetData.angle());
		}
		else if (GlobalSetting::instance()->Tranform == 3) {
			targetData.setCenterX((targetData.centerX() - verify_x));
			targetData.setCenterY((targetData.centerY() - verify_y));
			targetData.setAngle(360 - targetData.angle());
		}
		m_datalist.push_back(targetData);
	}
	removeIfMarked();
	return 0;
}

/*
* 判断是否有已经打过标的工件，如果存在，则将对应的工件移除
*/
void laserMarking::removeIfMarked()
{
	//在这里将这一次的与上一次的进行对比
	//获取每一次工件所运行的长度
	double distance = ui.dist->value();
	double errorRangeMax = ui.errorRange->value();
	GlobalSetting::instance()->distance = distance;
	GlobalSetting::instance()->errorRangeMax = errorRangeMax;
	QList<TargetData>::iterator iter;
	for (iter = m_datalist.begin();iter != m_datalist.end() ; iter++) {
		for (int j = 0; j < lastData.size(); j++) {
			if (abs((*iter).centerY() - lastData.at(j).centerY()) < 5) {
				double errorRange = abs((*iter).centerX() - lastData.at(j).centerX() + distance);//之间的误差
				if (errorRange < errorRangeMax) {//如果之间的误差小于2，则说明已经打过了
					qDebug() << "object is marked last time";
					m_datalist.erase(iter);
					break;
				}
			}
			
		}
	}
	lastData = m_datalist;
	return;
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
	if (GlobalSetting::instance()->state) {
		curTime0 = QDateTime::currentDateTime();
		if (started) {
			qDebug() << "emit image_process method!";
		}
		GlobalSetting::instance()->ming = false;
		imgshow = img;
		int imgW = img.width();
		int imgH = img.height();
		if (ui.realtime_display->checkState() == Qt::CheckState::Checked) {
			cvImg = matConvertQImage.QImage2cvMat(imgshow);
			imwrite("./test1111.bmp", cvImg);
			int featureNumber = GlobalSetting::instance()->getFeatureNumber();
			int padding = GlobalSetting::instance()->getPadding();
			int scoreNumber = GlobalSetting::instance()->getScoreNumber();

			int x = GlobalSetting::instance()->getROI().left();
			int y = GlobalSetting::instance()->getROI().top();
			int width = GlobalSetting::instance()->getROI().right() - x;
			int height = GlobalSetting::instance()->getROI().bottom() - y;
			Rect roi(x + imgshow.width() / 2, y + imgshow.height() / 2, width, height);
			std::string classid = GlobalSetting::instance()->getTemplateName().toStdString();

			vector<recognizedObjectLocation> ObjLocations;
			ObjLocations = test("./test1111.bmp", "./", featureNumber, roi, padding, classid, scoreNumber);
			QPixmap ConvertPixmap = QPixmap::fromImage(imgshow);
			if (m1Item)
			{
				delete m1Item;
				m1Item = NULL;
			}
			m1Item = new MyGraphicsItem(&ConvertPixmap);
			for (auto& i : ObjLocations) {
				if (GlobalSetting::instance()->vec_range.size() == 0 || GlobalSetting::instance()->vec_range.isEmpty()) {
					QMessageBox::warning(this, tr("failed"), tr("No marking range is set! "));
					return;
				}
				//判断识别的物体是否都存在于打标范围中，如果没有在打标范围中就剔除
				QPointF leftTop = GlobalSetting::instance()->vec_range.at(0) ;//范围的左上角
				QPointF rightBottom = GlobalSetting::instance()->vec_range.at(4);//范围的左上角
				//如果没有在范围内，直接break；如果在范围里面， 保存并画下来
				if (!(i.center.x > leftTop.x() + imgW/2 && i.center.y > leftTop.y() + imgH / 2 &&
					i.center.x < rightBottom.x() + imgW / 2 && i.center.y < rightBottom.y() + imgH / 2)) {
					break;
				}
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
			curTime1 = QDateTime::currentDateTime();
			int mesc = curTime0.msecsTo(curTime1);
			qDebug() << "between getImage and find the object time:" << mesc;
			//点击开始之后，进行打标
			if (started) {
				qDebug() << "markOK: " << markOK;
				if (markOK) { //上一次的打标也结束了
					markOK = false;
					double sleepTime = ui.spinBox->value();
					GlobalSetting::instance()->sleepTime = sleepTime;
					QThread::sleep(sleepTime);
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
	else {
		qDebug() << "event queue has something but ignore!";
	}
}

//停止按钮，修改相机的触发模式为连续
void laserMarking::stop()
{
	qDebug() << "change camera mode ==> gogo";
	m_thread->stop();
	while (true) {
		if (!m_thread->isRunning()) {
			//终止相机线程，重新创建新的线程模型，但是当前线程还存在事件队列,将state置为false
			delete m_thread;
			m_thread = new CaptureThread(this);
			connect(m_thread, SIGNAL(captured(QImage)), this, SLOT(image_process(QImage)), Qt::QueuedConnection);
			connect(m_thread, SIGNAL(showWrongMessageInCameraThread(int)), this, SLOT(showWrongMessage(int)));
			GlobalSetting::instance()->state = false;
			break;
		}
	}
	started = false;
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
	GlobalSetting::instance()->trigger = false;
	m_thread->start();
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
	if (readString.toStdString().find("<XE>") != string::npos) {
		qDebug() << readString;
		qDebug() << "mark ok!";
		if (started) {
			ALL_numberMarked += numberMarked;
			ui.finish_count->setText(QString::number(ALL_numberMarked));
			if (ui.comboBox->currentIndex() != 6) {
				qDebug() << "sendSignalToPLC:port = com "<<ui.comboBox->currentIndex();
				sendSignalToPLC(ui.comboBox->currentIndex() + 1);
				qDebug() << "send successful!";
			}
			curTime3 = QDateTime::currentDateTime();
			int time0 = curTime2.msecsTo(curTime3);
			qDebug() << "between send to laserMarking marchine and get result time:" << time0;
		}
		markOK = true;//表示上一次打标结束了
	}
}

//开启整体流程，修改相机为触发模式
void laserMarking::start()
{
	qDebug() << "change camera mode ==> trigger";
	m_thread->stop();
	while (true) {
		if (!m_thread->isRunning()) {
			//终止相机线程，重新创建新的线程模型，但是当前线程还存在事件队列
			delete m_thread;
			m_thread = new CaptureThread(this);
			connect(m_thread, SIGNAL(captured(QImage)), this, SLOT(image_process(QImage)), Qt::QueuedConnection);
			connect(m_thread, SIGNAL(showWrongMessageInCameraThread(int)), this, SLOT(showWrongMessage(int)));
			GlobalSetting::instance()->state = false;
			break;
		}
	}
	markOK = true;
	ui.realtime_display->setDown(true);
	ui.realtime_display->setCheckState(Qt::CheckState::Checked);
	started = true;
	GlobalSetting::instance()->trigger = true;
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
	m_thread->start();
}

void laserMarking::clear()
{
	ALL_numberMarked = 0;
	ui.finish_count->setText(QString::number(ALL_numberMarked));
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

////////////////////以下是注册机相关代码/////////////////////
//无时间限制注册
bool laserMarking::checkRegist()
{
	// 先调用注册机，检查是否已注册
	bool registState = registVerify();

	//未注册程序关闭
	if (registState)
	{
		this->show();
	}
	else
	{
		QTimer::singleShot(0, qApp, SLOT(quit()));
		return false;
	}
}

bool laserMarking::registVerify()
{
	const char* exeName = "Client.exe";// 注册机应用程序所在路径

	/////////////////////////////////////启动Client.exe///////////////////////////////////
	HINSTANCE hNewExe = ShellExecuteA(NULL, "open", exeName, NULL, NULL, SW_SHOW);
	if ((DWORD)hNewExe <= 32)
	{
		printf("return value:%d\n", (DWORD)hNewExe);
		return false;
	}

	///////////////////////////////////开启进程通信，接收Client的信号///////////////////////////////////
	//步骤2：创建命名管道
	//HANDLE h_Mypipe = NULL;
	h_Mypipe = CreateNamedPipe(
		MY_NAMED_PIPE, //为命名管道创建名称
		PIPE_ACCESS_DUPLEX, //管道访问方式：PIPE_ACCESS_DUPLEX指双向模式
		PIPE_TYPE_MESSAGE | //命名管道句柄的写入方式：以数据块的方式写入管道
		PIPE_READMODE_MESSAGE | //命名管道句柄的读取方式：以数据块的方式从管道读取
		PIPE_WAIT, //命名管道句柄的等待方式：阻塞方式PIPE_WAIT   PIPE_NOWAIT
		PIPE_UNLIMITED_INSTANCES, //管道所能创建的最大实例个数：1~255，
		0, //管道的输出缓冲区容量，0表示默认大小
		0, //管道的输入缓冲区容量，0表示默认大小 1000, //管道的默认等待超时，单位毫秒
		NMPWAIT_WAIT_FOREVER,
		0); //管道的安全性，NULL表示windows提供的默认安全

	//INVALID_HANDLE_VALUE是CreateNamedPipe返回值，表示创建失败
	if (h_Mypipe == INVALID_HANDLE_VALUE)
	{
		qDebug() << "Create Named_Pipe Failed...";
		return false;
	}

	//步骤3：等待客户端的连接
	qDebug() << "Wait for Connect...";
	if (!ConnectNamedPipe(h_Mypipe, NULL))
	{
		qDebug() << "Connect Failed...";
		CloseHandle(h_Mypipe);
		return false;
	}
	else
		qDebug() << "Connect Successed...";

	DWORD wLen = 0;
	DWORD rLen = 0;
	char szBuffer[BUF_SIZE] = { 0 };

	int state = 0;

	// 接收客户端（注册机）信号
	while(!state)
	{
		if (!ReadFile(h_Mypipe, szBuffer, BUF_SIZE, &rLen, NULL)) {
			qDebug() << "Read Failed...";
		}
		else {
			qDebug() << szBuffer << "!!!";
			string szBufferString = string(szBuffer);
			string compareBufferString = string(szBuffer, 0, 8);
			// 接收到客户端（注册机）注册成功的信号，返回
			if (strcmp(compareBufferString.c_str(), "lefttime") == 0)
			{
				state = 1;
				CloseHandle(h_Mypipe);
				return true;
			}
			else if (strcmp(szBuffer, "ExitRegistered") == 0) {
				break;
			}
		}

	}
	CloseHandle(h_Mypipe);
	return false;
}

/*注册机*/
//实时更新信号内容
void laserMarking::on_showMsg(QString msg)
{
	qDebug() << "0    msg" << msg;
	if (!((msg.mid(0, 8)).compare("deadline")) && success)
	{
		//ui.setupUi(this);
		success = false;
		this->show();

	}

	if (!((msg.mid(0, 8)).compare("deadline")) && ((msg.mid(9).split(":")[2].toInt() + msg.mid(9).split(":")[1].toInt() * 60 + msg.mid(9).split(":")[0].toInt() * 3600) < sumTime))
	{
		second = msg.mid(9).split(":")[2].toInt();
		minute = msg.mid(9).split(":")[1].toInt();
		hours = msg.mid(9).split(":")[0].toInt();
		sumTime = second + minute * 60 + hours * 3600;
		//deadTime->setText(msg);
	}
	else
		if (((msg.mid(0, 8)).compare("deadline")) && success == false)
		{
			QMessageBox::information(this, tr("提示"), QString(tr("请重新注册")));
			QApplication* app;
			app->exit(0);
		}
		else if ((!((msg.mid(0, 8)).compare("deadline"))) && ((msg.mid(9).split(":")[2].toInt() + msg.mid(9).split(":")[1].toInt() * 60 + msg.mid(9).split(":")[0].toInt() * 3600) >= sumTime))
		{
			QMessageBox::information(NULL, tr("提示"), QString(tr("请勿停止相关进程")));
			QApplication* app;
			app->exit(0);
		}


}

/*注册机*/
//获取注册程序发出的信号
void laserMarking::getMsg()
{
	MyTime mTime;
	const char* exeName = "Client.exe";

	/////////////////////////////////////启动Client.exe///////////////////////////////////
	HINSTANCE hNewExe = ShellExecuteA(NULL, "open", exeName, NULL, NULL, SW_SHOW);
	if ((DWORD)hNewExe <= 32)
	{
		//printf("return value:%d\n", (DWORD)hNewExe);
		qDebug() << "hhhhh" << endl;
		exitAll();
		return;
	}
	else
	{
		//printf("开启程序成功!\n");
		//system("TASKKILL /F /IM Client.exe");
	}

	//定义初始的时间变量，后期主线程可定时查看该时间变量的值是否发生改变
	int hours = 0;
	int minute = 0;
	int second = 0;

	///////////////////////////////////开启进程通信，接收Client的信号///////////////////////////////////
	//步骤2：创建命名管道
	h_Mypipe = CreateNamedPipe(
		MY_NAMED_PIPE, //为命名管道创建名称
		PIPE_ACCESS_DUPLEX, //管道访问方式：PIPE_ACCESS_DUPLEX指双向模式
		PIPE_TYPE_MESSAGE | //命名管道句柄的写入方式：以数据块的方式写入管道
		PIPE_READMODE_MESSAGE | //命名管道句柄的读取方式：以数据块的方式从管道读取
		PIPE_WAIT, //命名管道句柄的等待方式：阻塞方式PIPE_WAIT   PIPE_NOWAIT
		PIPE_UNLIMITED_INSTANCES, //管道所能创建的最大实例个数：1~255，
		0, //管道的输出缓冲区容量，0表示默认大小
		0, //管道的输入缓冲区容量，0表示默认大小 1000, //管道的默认等待超时，单位毫秒
		NMPWAIT_WAIT_FOREVER,
		0); //管道的安全性，NULL表示windows提供的默认安全

	//INVALID_HANDLE_VALUE是CreateNamedPipe返回值，表示创建失败
	if (h_Mypipe == INVALID_HANDLE_VALUE)
	{
		qDebug() << "Create Named_Pipe Failed..." << endl;
		return;
	}

	//步骤3：等待客户端的连接
	qDebug() << "Wait for Connect..." << endl;
	if (!ConnectNamedPipe(h_Mypipe, NULL))
	{
		qDebug() << "Connect Failed..." << endl;
		return;
	}
	else
		qDebug() << "Connect Successed..." << endl;

	DWORD wLen = 0;
	DWORD rLen = 0;
	char szBuffer[BUF_SIZE] = { 0 };

	int state = 0;

	while (!state && bContinue)
	{
		if (!ReadFile(h_Mypipe, szBuffer, BUF_SIZE, &rLen, NULL)) {
			qDebug() << "Read Failed..." << endl;
			qDebug() << szBuffer << "...." << endl;
			//on_deleteProcess();
			qDebug() << "111..." << endl;
			QApplication* app;
			app->quit();
			break;
		}
		else {
			qDebug() << szBuffer << "6" << endl;
			receiveMsg(szBuffer);
		}

	}
}

void laserMarking::on_exit()
{
	QApplication::exit();
}

void laserMarking::on_deleteProcess()
{

	qDebug() << "111111";
	if (pro_align)
	{
		qDebug() << "222222";
		pro_align->close();
		delete pro_align;
		pro_align = NULL;
	}
	if (pro_show)
	{
		qDebug() << "222222";
		pro_show->close();
		pro_show->~QProcess();
		/*pro_show->close();
		delete pro_show;
		pro_show = NULL;*/
		qDebug() << "222222";
	}

}