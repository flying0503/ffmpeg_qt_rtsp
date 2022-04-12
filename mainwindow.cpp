#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mPlayer = new VideoPlayer;
    ui->pauseButton->setDisabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_startButton_clicked()
{
    static bool flag = true;

    if(flag)
    {
        //将播放路径传入videoplayer
        mPlayer->setFileName(ui->urlList->currentText());

        //开启播放线程
        mPlayer->startPlay();
        //改变ui
        ui->startButton->setText("Close");
        ui->pauseButton->setEnabled(true);
        flag = false;
    }else {
        //停止播放线程
        if(mPlayer->state()==Paused)
            on_pauseButton_clicked();

        mPlayer->stopPlay();

        //改变ui
        ui->startButton->setText("Open");
        ui->pauseButton->setDisabled(true);
        flag = true;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto s = mPlayer->state();
    if(Stoped == s)
    {
    }else if (Running ==s) {
        mPlayer->stopPlay();
    }else {
        mPlayer->stopPlay();
    }
}


void MainWindow::on_pauseButton_clicked()
{
    static bool flag = true;

    if(flag)
    {

        //暂停播放线程开启播放线程
        mPlayer->pause();
        //改变ui
        ui->pauseButton->setText("play");
        flag = false;
    }else {
        //开启播放线程
        mPlayer->resume();
        //改变ui
        ui->pauseButton->setText("pause");
        flag = true;
    }
}

