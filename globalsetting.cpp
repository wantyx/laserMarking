#include "globalsetting.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <qdebug.h>
#include "MatConvertQImage.h"

GlobalSetting *GlobalSetting::m_instance = NULL;

GlobalSetting *GlobalSetting::instance()
{
    if(!m_instance) {
        m_instance = new GlobalSetting(qApp);
    }

    return m_instance;
}

GlobalSetting::GlobalSetting(QObject *parent) : QObject(parent)
{
    readIniFile();
}

GlobalSetting::~GlobalSetting()
{
    saveIniFile();
}

void GlobalSetting::init()
{
    readIniFile();
}

void GlobalSetting::readIniFile()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    m_com = settings.value("DevOption/com","COM3").toString();
    m_baudrate = settings.value("DevOption/baudrate",115200).toInt();
    m_databit = settings.value("DevOption/bytesize",8).toInt();
    m_parity = settings.value("DevOption/parity",0).toInt();
    m_stopbit = settings.value("DevOption/stopbits",0).toInt();

    m_netAddr = settings.value("DevOption/NetAddr","192.168.1.106").toString();
    m_netPort = settings.value("DevOption/NetPort",2000).toInt();
    A_caliBoard = settings.value("DevOption/A_caliBoard").toFloat();
    B_caliBoard = settings.value("DevOption/B_caliBoard").toFloat();
    C_caliBoard = settings.value("DevOption/C_caliBoard").toFloat();
    D_caliBoard = settings.value("DevOption/D_caliBoard").toFloat();

    templateImagePath = settings.value("DevOption/templateImagePath").toString();
    templateName = settings.value("DevOption/templateName").toString();
    featureNumber = settings.value("DevOption/featureNumber").toInt();
    padding = settings.value("DevOption/padding").toInt();
    savePath = settings.value("DevOption/savePath").toString();

    int roix = settings.value("DevOption/ROIX").toInt();
    int roiy = settings.value("DevOption/ROIY").toInt();
    int roiw = settings.value("DevOption/ROIW").toInt();
    int roih = settings.value("DevOption/ROIH").toInt();
    ROI = QRect(roix, roiy, roiw, roih);
    calibrateImage = matConvertQImage.QImage2cvMat(settings.value("DevOption/calibrateImage").value<QImage>());
    templateImage = matConvertQImage.QImage2cvMat(settings.value("DevOption/templateImage").value<QImage>());

    chessH = settings.value("DevOption/chessH").toInt();
    chessW = settings.value("DevOption/chessW").toInt();
    chessV = settings.value("DevOption/chessV").toInt();

    cameraIntriData[0] = settings.value("DevOption/cameraIntriData0").toDouble();
    cameraIntriData[1] = settings.value("DevOption/cameraIntriData1").toDouble();
    cameraIntriData[2] = settings.value("DevOption/cameraIntriData2").toDouble();
    cameraIntriData[3] = settings.value("DevOption/cameraIntriData3").toDouble();
    cameraIntriData[4] = settings.value("DevOption/cameraIntriData4").toDouble();
    cameraIntriData[5] = settings.value("DevOption/cameraIntriData5").toDouble();
    cameraIntriData[6] = settings.value("DevOption/cameraIntriData6").toDouble();
    cameraIntriData[7] = settings.value("DevOption/cameraIntriData7").toDouble();
    cameraIntriData[8] = settings.value("DevOption/cameraIntriData8").toDouble();
    cameraIntri = cv::Mat(3,3,6,cameraIntriData);

    RTData[0] = settings.value("DevOption/RTData0").toDouble();
    RTData[1] = settings.value("DevOption/RTData1").toDouble();
    RTData[2] = settings.value("DevOption/RTData2").toDouble();
    RTData[3] = settings.value("DevOption/RTData3").toDouble();
    RTData[4] = settings.value("DevOption/RTData4").toDouble();
    RTData[5] = settings.value("DevOption/RTData5").toDouble();
    RTData[6] = settings.value("DevOption/RTData6").toDouble();
    RTData[7] = settings.value("DevOption/RTData7").toDouble();
    RTData[8] = settings.value("DevOption/RTData8").toDouble();
    RTData[9] = settings.value("DevOption/RTData9").toDouble();
    RTData[10] = settings.value("DevOption/RTData10").toDouble();
    RTData[11] = settings.value("DevOption/RTData11").toDouble();
    RTData[12] = settings.value("DevOption/RTData12").toDouble();
    RTData[13] = settings.value("DevOption/RTData13").toDouble();
    RTData[14] = settings.value("DevOption/RTData14").toDouble();
    RTData[15] = settings.value("DevOption/RTData15").toDouble();

    int vec_centerLineSize = settings.value("DevOption/centerLineSize").toInt();
    for (int i = 0; i < vec_centerLineSize; i++) {
        vec_centerLine.push_back(settings.value("DevOption/centerLineSize"+QString::number(i)).toPointF());
    }
    int vec_rangeAreaSize = settings.value("DevOption/rangeAreaSize").toInt();
    for (int i = 0; i < vec_rangeAreaSize; i++) {
        vec_range.push_back(settings.value("DevOption/rangeArea" + QString::number(i)).toPointF());
    }
    RT = cv::Mat(4, 4, 6, RTData);

    Tranform = settings.value("DevOption/transform").toInt();
}

void GlobalSetting::saveIniFile()
{
    qDebug() << "baocunle";
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup("DevOption");
    settings.setValue("com", m_com);
    settings.setValue("baudrate", m_baudrate);
    settings.setValue("bytesize", m_databit);
    settings.setValue("parity", m_parity);
    settings.setValue("stopbits", m_stopbit);

    settings.setValue("NetAddr",m_netAddr);
    settings.setValue("NetPort",m_netPort);
    settings.setValue("A_caliBoard", A_caliBoard);
    settings.setValue("B_caliBoard", B_caliBoard);
    settings.setValue("C_caliBoard", C_caliBoard);
    settings.setValue("D_caliBoard", D_caliBoard);
    settings.setValue("templateImagePath", templateImagePath);
    settings.setValue("templateName", templateName);
    settings.setValue("savePath", savePath);
    settings.setValue("featureNumber", featureNumber);
    settings.setValue("padding", padding);

    settings.setValue("ROIX", ROI.left());
    settings.setValue("ROIY", ROI.top());
    settings.setValue("ROIW", ROI.width());
    settings.setValue("ROIH", ROI.height());
    
    settings.setValue("chessW", chessW);
    settings.setValue("chessH", chessH);
    settings.setValue("chessV", chessV);

    if (calibrateImage.data) {
        settings.setValue("calibrateImage", matConvertQImage.Mat2QImage(calibrateImage));
        
    }
    //if (templateImage.data) {
    //    settings.setValue("templateImage", matConvertQImage.Mat2QImage(templateImage));
    //}
    //这里需要把内参的值取出来
    for (int i = 0; i < cameraIntri.rows; i++) {
        for (int j = 0; j < cameraIntri.cols; j++) {
            cameraIntriData[i*3+j] = cameraIntri.at<double>(i, j);
        }
    }
    settings.setValue("cameraIntriData0",cameraIntriData[0]);
    settings.setValue("cameraIntriData1",cameraIntriData[1]);
    settings.setValue("cameraIntriData2",cameraIntriData[2]);
    settings.setValue("cameraIntriData3",cameraIntriData[3]);
    settings.setValue("cameraIntriData4",cameraIntriData[4]);
    settings.setValue("cameraIntriData5",cameraIntriData[5]);
    settings.setValue("cameraIntriData6",cameraIntriData[6]);
    settings.setValue("cameraIntriData7",cameraIntriData[7]);
    settings.setValue("cameraIntriData8",cameraIntriData[8]);

    //这里需要将RT的值取出来
    for (int i = 0; i < RT.rows; i++) {
        for (int j = 0; j < RT.cols; j++) {
            RTData[i * 4 + j] = RT.at<double>(i, j);
        }
    }
    settings.setValue("RTData0", RTData[0]);
    settings.setValue("RTData1", RTData[1]);
    settings.setValue("RTData2", RTData[2]);
    settings.setValue("RTData3", RTData[3]);
    settings.setValue("RTData4", RTData[4]);
    settings.setValue("RTData5", RTData[5]);
    settings.setValue("RTData6", RTData[6]);
    settings.setValue("RTData7", RTData[7]);
    settings.setValue("RTData8", RTData[8]);
    settings.setValue("RTData9", RTData[9]);
    settings.setValue("RTData10", RTData[10]);
    settings.setValue("RTData11", RTData[11]);
    settings.setValue("RTData12", RTData[12]);
    settings.setValue("RTData13", RTData[13]);
    settings.setValue("RTData14", RTData[14]);
    settings.setValue("RTData15", RTData[15]);

    QVariant centerLineSize = vec_centerLine.size();
    settings.setValue("centerLineSize", centerLineSize);
    for (int i = 0; i < vec_centerLine.size(); i++) {
        settings.setValue("centerLineSize"+QString::number(i), vec_centerLine.at(i));
    }
    QVariant rangeAreaSize = vec_range.size();
    settings.setValue("rangeAreaSize", rangeAreaSize);
    for (int i = 0; i < vec_range.size(); i++) {
        settings.setValue("rangeArea" + QString::number(i), vec_range.at(i));
    }
    settings.setValue("transform", Tranform);

    settings.endGroup();
}
