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
    void drawRect();//选择为画框方式
    void drawLine();//选择为画线方式
    void startDrawRect();//开始画打标范围
    void startDrawCenterLine();//开始画中心线
    void endDrawRect();//保存打标范围到GlobalSetting中
    void endDrawCenterLine();//保存中心线到GlobalSetting中
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
