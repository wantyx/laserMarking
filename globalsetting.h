#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <QObject>
#include <QFile>
#include <opencv2/opencv.hpp>
#include "targetdata.h"
#include <qrect.h>
#include "MatConvertQImage.h"

class GlobalSetting : public QObject
{
    Q_OBJECT
public:

    GlobalSetting(QObject* parent = NULL);
    ~GlobalSetting();

    static GlobalSetting* m_instance;
    bool ming = true;
    int Tranform = 0;
    static GlobalSetting *instance();

    void init();
    void saveIniFile();

    QString getCom()const{return m_com;}
    void setCom(QString com){m_com = com;}

    int getBaudrate()const{return m_baudrate;}
    void setBaudrate(int baudrate){m_baudrate = baudrate;}

    int getDatabit()const{return m_databit;}
    void setDatabit(int databit){m_databit = databit;}

    int getStopbit()const{return m_stopbit;}
    void setStopbit(int stopbit){m_stopbit = stopbit;}

    int getParity()const{return m_parity;}
    void setParity(int parity){m_parity = parity;}

    QString getNetAddr()const{return m_netAddr;}
    void setNetAddr(QString addr){m_netAddr = addr;}

    int getNetPort()const{return m_netPort;}
    void setNetPort(int port){m_netPort = port;}

    QString getImgPath()const { return imgPath; }
    void setImgPath(QString path) { imgPath = path; }

    cv::Mat getCameraIntri() { return cameraIntri; }
    void setCameraIntri(cv::Mat _) { cameraIntri = _; }

    cv::Mat getDistCoffe() { return distCoffe; }
    void setDistCoffe(cv::Mat _) { distCoffe = _; }

    cv::Mat getRT() { return RT; };
    void setRT(cv::Mat _) { RT = _; }

    cv::Mat getTemplateImage() { return templateImage; };
    void setTemplateImage(cv::Mat _) { templateImage = _; }

    double A_caliBoard;
    double B_caliBoard;
    double C_caliBoard;
    double D_caliBoard;
    cv::Mat calibrateImage;
    cv::Mat templateImage;
    int chessW;
    int chessH;
    int chessV;
    bool trigger = false;
    bool state = true;

    double distance = 0;
    double errorRangeMax = 0;
    double sleepTime = 0;

    QString getTemplateImagePath()const { return templateImagePath; }
    void setTemplateImagePath(QString _) { templateImagePath = _; }
    
    QString getTemplateName()const { return templateName; }
    void setTemplateName(QString _) { templateName = _; }

    int getFeatureNumber() { return featureNumber; }
    void setFeatureNumber(int _) { featureNumber = _; }

    int getScoreNumber() { return scoreNumber; }
    void setScoreNumber(int _) { scoreNumber = _; }

    int getPadding()const { return padding; }
    void setPadding(int _) { padding = _; }

    QString getSavePath()const { return savePath; }
    void setSavePath(QString _) { savePath = _; }

    QRect getROI()const { return ROI; }
    void setROI(QRect _) { ROI = _; }

    //中心线和打标范围
    QVector<QPointF> vec_centerLine;
    QVector<QPointF> vec_range;

private:

    

    

    void readIniFile();

    QString m_com;
    int m_baudrate;
    int m_databit;
    int m_stopbit;
    int m_parity;

    QString m_netAddr;
    int m_netPort;

    //以下是标定的一些参数配置
    QString imgPath;//标定图
    cv::Mat cameraIntri, distCoffe;
    double cameraIntriData[9];
    cv::Mat RT;
    double RTData[16];

    //以下是模板参数
    QString templateImagePath;
    QString templateName;
    int featureNumber;
    int padding;
    int scoreNumber;
    QString savePath;
    QRect ROI;

    //图片格式转换
    MatConvertQImage matConvertQImage;

    
    
     
};

#endif // GLOBALSETTING_H
