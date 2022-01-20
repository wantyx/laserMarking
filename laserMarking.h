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
//ע�������ͷ�ļ�
#include <Windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <qprocess.h>
using namespace std;

/*ע���*/
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

    CaptureThread* m_thread;//������߳�

    QProcess* pro_align;
    QProcess* pro_show;
    bool registered = false;

public slots:
    void mark_test();//������
    void get_position();//�Ӿ���λ
    void setting_net();//��������
    int piex2location();//����ת��������

    //void cal_time();//����ӿ�ʼ�����͸�������Ϣ������̵�ʱ��
    void removeIfMarked();
    void preview();//�������
    void image_process(QImage img);
    void stop();//ֹͣ���
    void enlarge();
    void delarge();
    void resetImg();
    void updateShowText();
    void start();
    void clear();
    void showWrongMessage(int _);

private slots:
    //ע���
    void on_showMsg(QString);
    void on_deleteProcess();
    void on_exit();

private:
    //ע���
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
    //�ĸ�ʱ��ֱ�������¼��Ӧ��ʱ���
    QDateTime curTime0;
    QDateTime curTime1;
    QDateTime curTime2;
    QDateTime curTime3;
    bool state = true;
    void showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item);
signals:
    //ע���
    void receiveMsg(QString);
    void exitAll();
};
