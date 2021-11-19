#include "MatConvertQImage.h"
#include <qDebug>
#include <opencv2/opencv.hpp>

MatConvertQImage::MatConvertQImage()
{

}
MatConvertQImage::~MatConvertQImage()
{

}

cv::Mat MatConvertQImage::QImage2cvMat(QImage image)
{
    cv::Mat mat;
    //qDebug() << image.format();
    switch (image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

QImage MatConvertQImage::Mat2QImage(const cv::Mat& InputMat)

{

    cv::Mat TmpMat;

    // convert the color space to RGB
    if (InputMat.channels() == 1)

    {
        cv::cvtColor(InputMat, TmpMat, CV_GRAY2RGB);
    }

    else

    {
        cv::cvtColor(InputMat, TmpMat, CV_BGR2RGB);
    }


    // construct the QImage using the data of the mat, while do not copy the data

    QImage Result = QImage((const uchar*)(TmpMat.data), TmpMat.cols, TmpMat.rows,

        QImage::Format_RGB888);

    // deep copy the data from mat to QImage

    Result.bits();

    return Result;


}