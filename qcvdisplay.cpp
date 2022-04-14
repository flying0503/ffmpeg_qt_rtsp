#include "qcvdisplay.h"

#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

#include <opencv2/imgproc/imgproc.hpp>

QCVDisplay::QCVDisplay(QWidget *parent) :
    QWidget(parent)
{
}

cv::Mat QCVDisplay::getBuffer()
{
    return buffer;
}

QImage QCVDisplay::cvMat2QImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
        if(mat.type() == CV_8UC1)
        {
            QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
            // Set the color table (used to translate colour indexes to qRgb values)
            image.setColorCount(256);
            for(int i = 0; i < 256; i++)
            {
                image.setColor(i, qRgb(i, i, i));
            }
            // Copy input Mat
            uchar *pSrc = mat.data;
            for(int row = 0; row < mat.rows; row ++)
            {
                uchar *pDest = image.scanLine(row);
                memcpy(pDest, pSrc, mat.cols);
                pSrc += mat.step;
            }
            return image;
        }
        // 8-bits unsigned, NO. OF CHANNELS = 3
        else if(mat.type() == CV_8UC3)
        {
            // Copy input Mat
            const uchar *pSrc = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }
        else if(mat.type() == CV_8UC4)
        {
            // Copy input Mat
            const uchar *pSrc = (const uchar*)mat.data;
            // Create QImage with same dimensions as input Mat
            QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
            return image.copy();
        }
        else
        {
            return QImage();
        }
}

// 将 Mat 转换为 QImage，由于 QImage 的每一行有多余对齐字节
// 故采用 RBG32 来消除多余字节
void QCVDisplay::matToQImage(const cv::Mat3b &src, QImage& dest)
{
    for (int y = 0; y < src.rows; ++y) {
        const cv::Vec3b *srcrow = src[y];
        QRgb *destrow = (QRgb*)dest.scanLine(y);
        for (int x = 0; x < src.cols; ++x) {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }
}

// 在控件上显示图片，使用 opencv 自带的 resize 使其缩放到和控件大小一致
void QCVDisplay::displayImage(const cv::Mat &img)
{
    QSize sz = data.size();
    if (img.channels() == 3) {
        buffer = img.clone();
    } else if (img.channels() == 1) {
        cv::cvtColor(img, buffer,cv::COLOR_BGR2GRAY);
    } else {
        throw UnsupportedFormatError();
    }
    cv::Mat resized;
    if (!sz.isEmpty()) {
        cv::resize(buffer, resized, cv::Size(sz.width(), sz.height()));
        //matToQImage(resized, data);
        data = cvMat2QImage(resized);
        update();
    }
}

// 绘图事件处理函数
void QCVDisplay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), data, event->rect());
}

// 缩放事件也采用 opencv 自带的 resize
void QCVDisplay::resizeEvent(QResizeEvent *event)
{
    if (data.size() != event->size()) {
        cv::Mat resized;
        data = QImage(event->size(), QImage::Format_RGB32);
        if (!buffer.empty() && !event->size().isEmpty()) {
            cv::resize(buffer, resized, cv::Size(event->size().width(),
                                                  event->size().height()));
            //matToQImage(resized, data);
            data = cvMat2QImage(resized);
        }
    }
    QWidget::resizeEvent(event);
}
