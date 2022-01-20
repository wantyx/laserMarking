#include "laserMarking.h"
#include "qdebug.h"


using namespace std;

laserMarking::laserMarking(QWidget *parent)
    : QMainWindow(parent)
{

	registered = checkRegist();////////////���ע��
	setting = NULL;
    ui.setupUi(this);
	
	
	logSysInit("log.txt");//������־ϵͳ
	m_thread = new CaptureThread(this);
	scene = new QGraphicsScene(this);
	scene->setSceneRect(-ui.graphicsView->height()/2, -ui.graphicsView->height() / 2,ui.graphicsView->height(),ui.graphicsView->height());
	m1Item = NULL;
	//����һ����ʱ��
	m_showTimer = new QTimer(this);
	m_showTimer->setInterval(100);
	connect(m_showTimer, SIGNAL(timeout()), this, SLOT(updateShowText()));
	//��ʼ��������ʽ��ͨ��
	m_pSerialPort = new QSerialPort;
	m_clientSocket = new QTcpSocket;

	//connect(ui.vision_position_btn, SIGNAL(clicked()), this, SLOT(get_position()));
	//�󶨰�ť����Ӧ��ִ�к���
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
*������ 
* 1.ͨ��piex2location����ת��֮��õ���Ӧ�ı궨��ƽ����Ϣ
* 2.�����Ƿ������϶�Ӧ����λ��(����)�����û�������Ӳ�������ʱ��
* 3.���ʹ����Ϣ����յ�ǰ֡����Ϣ���ȴ���ʱ�����յ�markOK֮�����ִ����һ�ε�ģ��ƥ�乤��
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
//* ���㴦����̵�ʱ��
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
//* ���Ժ��� 
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
* �����ý��棬��ʱ����ر����������������̣߳����������������ռͬһ����������ֳ������
*/
void laserMarking::setting_net()
{
	if (m_thread->isRunning()) {
		m_thread->stop();
		ui.take_pthoto_btn->setText(codec->toUnicode("Ԥ��"));
	}
	setting = new Setting();
	setting->show();
}

/*
* ����ת��������������ϵ���궨��ƽ������ϵ��ת������
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
		//�����е����ȫ��ת������2������ϵ�£�������2����к�����������ȷ����ôӦ����TransformΪ2����3�����

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
* �ж��Ƿ����Ѿ������Ĺ�����������ڣ��򽫶�Ӧ�Ĺ����Ƴ�
*/
void laserMarking::removeIfMarked()
{
	//�����ｫ��һ�ε�����һ�εĽ��жԱ�
	//��ȡÿһ�ι��������еĳ���
	double distance = ui.dist->value();
	double errorRangeMax = ui.errorRange->value();
	GlobalSetting::instance()->distance = distance;
	GlobalSetting::instance()->errorRangeMax = errorRangeMax;
	QList<TargetData>::iterator iter;
	for (iter = m_datalist.begin();iter != m_datalist.end() ; iter++) {
		for (int j = 0; j < lastData.size(); j++) {
			if (abs((*iter).centerY() - lastData.at(j).centerY()) < 5) {
				double errorRange = abs((*iter).centerX() - lastData.at(j).centerX() + distance);//֮������
				if (errorRange < errorRangeMax) {//���֮������С��2����˵���Ѿ������
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
* ���Ԥ�������չ���
*/
void laserMarking::preview()
{
	if (m_thread->isRunning()) {
		m_thread->stop();
		ui.take_pthoto_btn->setText(codec->toUnicode("Ԥ��"));
		ui.realtime_display->setDisabled(true);
		ui.center_line->setDisabled(true);
		ui.marking_range->setDisabled(true);
		
	}
	else {
		m_thread->start();
		ui.take_pthoto_btn->setText(codec->toUnicode("����"));
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
* ��������̵߳Ĳۺ�����ÿһ�β���ͼƬ����ִ��һ�θú���
* ��Ҫʵ�ֵ�ǰ֡��ģ��ƥ�乤����ͬʱ�ж������ߺʹ�귶Χ��ѡ����Ϣ��ming��Ҫʵ�ֵ�ǰ�ۺ����������֮������ٴδ�������ֹͬһʱ���δ���
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
				//�ж�ʶ��������Ƿ񶼴����ڴ�귶Χ�У����û���ڴ�귶Χ�о��޳�
				QPointF leftTop = GlobalSetting::instance()->vec_range.at(0) ;//��Χ�����Ͻ�
				QPointF rightBottom = GlobalSetting::instance()->vec_range.at(4);//��Χ�����Ͻ�
				//���û���ڷ�Χ�ڣ�ֱ��break������ڷ�Χ���棬 ���沢������
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

				//�����еĵ�ŵ�vector�з�����滭ͼ
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
			//�����ʼ֮�󣬽��д��
			if (started) {
				qDebug() << "markOK: " << markOK;
				if (markOK) { //��һ�εĴ��Ҳ������
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

//ֹͣ��ť���޸�����Ĵ���ģʽΪ����
void laserMarking::stop()
{
	qDebug() << "change camera mode ==> gogo";
	m_thread->stop();
	while (true) {
		if (!m_thread->isRunning()) {
			//��ֹ����̣߳����´����µ��߳�ģ�ͣ����ǵ�ǰ�̻߳������¼�����,��state��Ϊfalse
			delete m_thread;
			m_thread = new CaptureThread(this);
			connect(m_thread, SIGNAL(captured(QImage)), this, SLOT(image_process(QImage)), Qt::QueuedConnection);
			connect(m_thread, SIGNAL(showWrongMessageInCameraThread(int)), this, SLOT(showWrongMessage(int)));
			GlobalSetting::instance()->state = false;
			break;
		}
	}
	started = false;
	//���ఴťȫ��enable
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
* ��ʱ������������ÿ��һ��ʱ��ȥ���ܴ����ķ�����Ϣ�����ڽ���<XE>
* XE��ʾ��һ�δ�����
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
		markOK = true;//��ʾ��һ�δ�������
	}
}

//�����������̣��޸����Ϊ����ģʽ
void laserMarking::start()
{
	qDebug() << "change camera mode ==> trigger";
	m_thread->stop();
	while (true) {
		if (!m_thread->isRunning()) {
			//��ֹ����̣߳����´����µ��߳�ģ�ͣ����ǵ�ǰ�̻߳������¼�����
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
	//���ఴťȫ��disable
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

////////////////////������ע�����ش���/////////////////////
//��ʱ������ע��
bool laserMarking::checkRegist()
{
	// �ȵ���ע���������Ƿ���ע��
	bool registState = registVerify();

	//δע�����ر�
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
	const char* exeName = "Client.exe";// ע���Ӧ�ó�������·��

	/////////////////////////////////////����Client.exe///////////////////////////////////
	HINSTANCE hNewExe = ShellExecuteA(NULL, "open", exeName, NULL, NULL, SW_SHOW);
	if ((DWORD)hNewExe <= 32)
	{
		printf("return value:%d\n", (DWORD)hNewExe);
		return false;
	}

	///////////////////////////////////��������ͨ�ţ�����Client���ź�///////////////////////////////////
	//����2�����������ܵ�
	//HANDLE h_Mypipe = NULL;
	h_Mypipe = CreateNamedPipe(
		MY_NAMED_PIPE, //Ϊ�����ܵ���������
		PIPE_ACCESS_DUPLEX, //�ܵ����ʷ�ʽ��PIPE_ACCESS_DUPLEXָ˫��ģʽ
		PIPE_TYPE_MESSAGE | //�����ܵ������д�뷽ʽ�������ݿ�ķ�ʽд��ܵ�
		PIPE_READMODE_MESSAGE | //�����ܵ�����Ķ�ȡ��ʽ�������ݿ�ķ�ʽ�ӹܵ���ȡ
		PIPE_WAIT, //�����ܵ�����ĵȴ���ʽ��������ʽPIPE_WAIT   PIPE_NOWAIT
		PIPE_UNLIMITED_INSTANCES, //�ܵ����ܴ��������ʵ��������1~255��
		0, //�ܵ������������������0��ʾĬ�ϴ�С
		0, //�ܵ������뻺����������0��ʾĬ�ϴ�С 1000, //�ܵ���Ĭ�ϵȴ���ʱ����λ����
		NMPWAIT_WAIT_FOREVER,
		0); //�ܵ��İ�ȫ�ԣ�NULL��ʾwindows�ṩ��Ĭ�ϰ�ȫ

	//INVALID_HANDLE_VALUE��CreateNamedPipe����ֵ����ʾ����ʧ��
	if (h_Mypipe == INVALID_HANDLE_VALUE)
	{
		qDebug() << "Create Named_Pipe Failed...";
		return false;
	}

	//����3���ȴ��ͻ��˵�����
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

	// ���տͻ��ˣ�ע������ź�
	while(!state)
	{
		if (!ReadFile(h_Mypipe, szBuffer, BUF_SIZE, &rLen, NULL)) {
			qDebug() << "Read Failed...";
		}
		else {
			qDebug() << szBuffer << "!!!";
			string szBufferString = string(szBuffer);
			string compareBufferString = string(szBuffer, 0, 8);
			// ���յ��ͻ��ˣ�ע�����ע��ɹ����źţ�����
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

/*ע���*/
//ʵʱ�����ź�����
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
			QMessageBox::information(this, tr("��ʾ"), QString(tr("������ע��")));
			QApplication* app;
			app->exit(0);
		}
		else if ((!((msg.mid(0, 8)).compare("deadline"))) && ((msg.mid(9).split(":")[2].toInt() + msg.mid(9).split(":")[1].toInt() * 60 + msg.mid(9).split(":")[0].toInt() * 3600) >= sumTime))
		{
			QMessageBox::information(NULL, tr("��ʾ"), QString(tr("����ֹͣ��ؽ���")));
			QApplication* app;
			app->exit(0);
		}


}

/*ע���*/
//��ȡע����򷢳����ź�
void laserMarking::getMsg()
{
	MyTime mTime;
	const char* exeName = "Client.exe";

	/////////////////////////////////////����Client.exe///////////////////////////////////
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
		//printf("��������ɹ�!\n");
		//system("TASKKILL /F /IM Client.exe");
	}

	//�����ʼ��ʱ��������������߳̿ɶ�ʱ�鿴��ʱ�������ֵ�Ƿ����ı�
	int hours = 0;
	int minute = 0;
	int second = 0;

	///////////////////////////////////��������ͨ�ţ�����Client���ź�///////////////////////////////////
	//����2�����������ܵ�
	h_Mypipe = CreateNamedPipe(
		MY_NAMED_PIPE, //Ϊ�����ܵ���������
		PIPE_ACCESS_DUPLEX, //�ܵ����ʷ�ʽ��PIPE_ACCESS_DUPLEXָ˫��ģʽ
		PIPE_TYPE_MESSAGE | //�����ܵ������д�뷽ʽ�������ݿ�ķ�ʽд��ܵ�
		PIPE_READMODE_MESSAGE | //�����ܵ�����Ķ�ȡ��ʽ�������ݿ�ķ�ʽ�ӹܵ���ȡ
		PIPE_WAIT, //�����ܵ�����ĵȴ���ʽ��������ʽPIPE_WAIT   PIPE_NOWAIT
		PIPE_UNLIMITED_INSTANCES, //�ܵ����ܴ��������ʵ��������1~255��
		0, //�ܵ������������������0��ʾĬ�ϴ�С
		0, //�ܵ������뻺����������0��ʾĬ�ϴ�С 1000, //�ܵ���Ĭ�ϵȴ���ʱ����λ����
		NMPWAIT_WAIT_FOREVER,
		0); //�ܵ��İ�ȫ�ԣ�NULL��ʾwindows�ṩ��Ĭ�ϰ�ȫ

	//INVALID_HANDLE_VALUE��CreateNamedPipe����ֵ����ʾ����ʧ��
	if (h_Mypipe == INVALID_HANDLE_VALUE)
	{
		qDebug() << "Create Named_Pipe Failed..." << endl;
		return;
	}

	//����3���ȴ��ͻ��˵�����
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