#pragma once
#ifndef MATQIMAGE_HPP
#define MATQIMAGE_HPP
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


class MatConvertQImage
{
public:
    MatConvertQImage();
    ~MatConvertQImage();

    cv::Mat QImage2cvMat(QImage image);
    QImage Mat2QImage(const cv::Mat& InputMat);

};

#endif // MATQIMAGE_HPP