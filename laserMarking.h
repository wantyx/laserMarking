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
#include <time.h>
//注册机新增头文件
#include <Windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <qprocess.h>
using namespace std;

/*注册机*/
#define MY_NAMED_PIPE   L"\\\\.\\pipe\\lzb_Pipe"
#define BUF_SIZE 4096
//extern HANDLE h_Mypipe = NULL;
//extern HANDLE h_Mypipe;

struct MyTime
{
    int hours = 0;
    int minute = 0;
    int second = 0;

    bool compare(MyTime others) {
        if (hours == others.hours && minute == others.minute && second == others.second)
        {
            return true;
        }
        return false;
    }
};

class laserMarking : public QMainWindow
{
    Q_OBJECT

public:
    laserMarking(QWidget *parent = Q_NULLPTR);
    ~laserMarking();
    HANDLE h_Mypipe;

    CaptureThread* m_thread;//相机的线程

    QProcess* pro_align;
    QProcess* pro_show;
    bool registered = false;

public slots:
    void mark_test();//打标测试
    void get_position();//视觉定位
    void setting_net();//网口配置
    int piex2location();//像素转世界坐标

    //void cal_time();//计算从开始到发送给打标机信息这个过程的时间
    void removeIfMarked();
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

private slots:
    //注册机
    void on_showMsg(QString);
    void on_deleteProcess();
    void on_exit();

private:
    //注册机
    bool checkRegist();
    bool registVerify();
    void getMsg();
    //bool IsExistProcess(const char* szProcessName);
    //int killTaskl(const QString& exe);
    MyTime mTimer;
    MyTime curTime;
    MyTime oldTime;
    bool bContinue;
    bool success = true;
    int second = 60;
    int minute = 60;
    int hours = 60;
    int sumTime = 1000000000;

    Ui::laserMarkingClass ui;
    Interaction* interaction;
    Setting* setting;
    QSerialPort* m_pSerialPort;
    QTcpSocket* m_clientSocket;
    QTimer* m_showTimer;
    QList<TargetData> m_datalist;
    QList<TargetData> lastData;
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
    //四个时间分别用来记录对应的时间段
    QDateTime curTime0;
    QDateTime curTime1;
    QDateTime curTime2;
    QDateTime curTime3;
    bool state = true;
    void showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item);
signals:
    //注册机
    void receiveMsg(QString);
    void exitAll();
};
