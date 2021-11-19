#pragma once
#include <QtWidgets/QMainWindow>
#include <opencv2/opencv.hpp>
#include "globalsetting.h"
#include "qmessagebox.h"
#include "ui_setting.h"
#include "SetParam.h"
#include "cameraThread.h"
#include "mygraphicsitem.h"
#include "qevent.h"
#include "MatConvertQImage.h"
#include "log.h"
#include <qtextcodec.h>

class Setting : public QWidget
{
    Q_OBJECT

public:
    Setting(QWidget* parent = Q_NULLPTR);
    ~Setting();
    CaptureThread* m_thread;
    int nwidth;
    int nheight;
    void transform();

public slots:
    void showSetParam();
    void calibrate();
    void checkChessPoints();
    void piex2location(double x,double y);
    void start();
    void image_process(QImage img);
    void apply();
    void setMyGraphicsitem();
    void saveTemplate();
    void resetImage();
    void preview();
    void calibrate_photo();
    void drawRect();//ѡ��Ϊ����ʽ
    void drawLine();//ѡ��Ϊ���߷�ʽ
    void startDrawRect();//��ʼ����귶Χ
    void startDrawCenterLine();//��ʼ��������
    void endDrawRect();//�����귶Χ��GlobalSetting��
    void endDrawCenterLine();//���������ߵ�GlobalSetting��
    void clear1();
    void clear2();

private:
    SetParam* setparam;
    Ui::settingClass ui;
    QImage imgshow;
    QPixmap pixmap;
    MyGraphicsItem* myGraphicsItem;
    MyGraphicsItem* m1Item;
    QGraphicsScene* m1Scene;
    MatConvertQImage matConvertQImage;
    QTextCodec* codec = QTextCodec::codecForName("GBK");

    void showImage(QGraphicsView* const qgraphicsView, QGraphicsScene* const qgraphicsScene, MyGraphicsItem* const item);
};
