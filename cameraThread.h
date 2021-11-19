#pragma once
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class CaptureThread : public QThread
{
	Q_OBJECT
public:
	explicit CaptureThread(QObject *parent = 0);
	~CaptureThread();
public:
	void run();
	void stop();
	//bool enum_devices();
	bool setExposureTime(float time);
	bool setTrigger(int mode);
	bool quit;


signals:
	void captured(QImage img);
	void showWrongMessageInCameraThread(int _);
private:
	bool pause_status;
	float time_later=0;
	QObject* parent1;


	QVector<QRgb> grayColourTable;
	QVector<QRgb> ColourTable;

public slots:

};
