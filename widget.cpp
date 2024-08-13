#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    Thread = new ffmpegThread(this); // Set parent for proper memory management

    playerwidget = new VideoPlayer();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(playerwidget);
    ui->widget->setLayout(layout);

    // Connect signal from ffmpegThread to slot in VideoPlayer
    connect(Thread, &ffmpegThread::frameReady, playerwidget, &VideoPlayer::onFrameReady);
}

Widget::~Widget()
{
    Thread->stop();
    Thread->wait(); // Ensure thread has stopped before deleting
    delete ui;
}

void Widget::on_pushButtonPush_clicked(bool checked)
{
    if (checked)
    {
        Thread->setUrl(ui->lineEditPushUrl->text());
        Thread->start(); // Start the FFmpeg thread
    }
    else
    {
        Thread->stop(); // Stop the FFmpeg thread
    }
}

void Widget::on_pushButtonPlay_clicked()
{
    playerwidget->playStream(ui->lineEditPlayUrl->text());
}
