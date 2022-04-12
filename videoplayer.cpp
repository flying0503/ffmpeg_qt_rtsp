#include "videoplayer.h"
#include <QDebug>

VideoPlayer::VideoPlayer()
{
    pauseFlag = false;
    stopFlag = false;

    pFormatCtx = avformat_alloc_context();
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
}

VideoPlayer::~VideoPlayer()
{
    av_free(pFrameRGB);
    av_free(pFrame);
    avformat_close_input(&pFormatCtx);
}

void VideoPlayer::startPlay()
{
    ///调用 QThread 的start函数 将会自动执行下面的run函数 run函数是一个新的线程
    this->start();

}

void VideoPlayer::stopPlay()
{
    if (QThread::isRunning())
    {
        stopFlag = true;
        condition.wakeAll();
        QThread::quit();
        QThread::wait();
    }
}

void VideoPlayer::pause()
{
    if (QThread::isRunning())
    {
        pauseFlag = true;
    }
}

void VideoPlayer::resume()
{
    if (QThread::isRunning())
    {
        pauseFlag = false;
        condition.wakeAll();
    }
}

void VideoPlayer::setFileName(QString name)
{
    this->mFileName = name;
}

QString VideoPlayer::getFileName()
{
    return mFileName;
}

State VideoPlayer::state()
{
    State s = Stoped;
    if (!QThread::isRunning())
    {
        s = Stoped;
    }
    else if (QThread::isRunning() && pauseFlag)
    {
        s = Paused;
    }
    else if (QThread::isRunning() && (!pauseFlag))
    {
        s = Running;
    }
    return s;

}

void VideoPlayer::run()
{
    qDebug() << "enter thread : " << QThread::currentThreadId();

    while (!stopFlag)
    {
        if (pauseFlag)
        {
            mutex.lock();
            condition.wait(&mutex);
            mutex.unlock();
        }
        process();
    }
    pauseFlag = false;
    stopFlag = false;

    qDebug() << "exit thread : " << QThread::currentThreadId();
}

void VideoPlayer::process()
{
    qDebug()<<"hello";
    sleep(1);
}
