#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_laserMarking.h"
#include "pch.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include "globalsetting.h"
#include "interaction.h"
#include <qserialport.h>
#include <qtcpsocket.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include "Setting.h"
#include "cameraThread.h"
#include <qtextcodec.h>
#include <QMessageLogger>
#include "cktx.h"
#include "log.h"
using namespace std;

class laserMarking : public QMainWindow
{
    Q_OBJECT

public:
    laserMarking(QWidget *parent = Q_NULLPTR);
    ~laserMarking();

    CaptureThread* m_thread;//相机的线程

public slots:
    void mark_test();//打标测试
    void get_position();//视觉定位
    void setting_net();//网口配置
    int piex2location();//像素转世界坐标

    void preview();//开启相机
    void image_process(QImage img);
    void stop();//停止相机
    void enlarge();
    void delarge();
    void resetImg();
    void updateShowText();
    void start();
    void clear();
    void showWrongMessage(int _);

private:
    Ui::laserMarkingClass ui;
    Interaction* interaction;
    Setting* setting;
    QSerialPort* m_pSerialPort;
    QTcpSocket* m_clientSocket;
    QTimer* m_showTimer;
    QList<TargetData> m_datalist;
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QGraphicsScene* scene;
    QImage imgshow;
    cv::Mat cvImg;
    MatConvertQImage matConvertQImage;
    MyGraphicsItem* m1Item;
    qreal scale;
    bool markOK;
    bool started = false;
    long ALL_numberMarked = 0;
    long numberMarked = 0;

    void showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item);
};
