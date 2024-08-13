#include "widget.h"
#include "ui_widget.h"
#include<QVBoxLayout>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    Thread = new ffmpegThread();

    playerwidget=new VideoPlayer();

    QVBoxLayout *layout=new QVBoxLayout();
    layout->addWidget(playerwidget);
    ui->widget->setLayout(layout);

}

Widget::~Widget()
{
    Thread->stopFFmpge();
    delete ui;
}

void Widget::on_pushButtonPush_clicked(bool checked)
{
    if(checked)
    {
        Thread->run();
    }
    else{
        Thread->stopFFmpge();
    }
}


void Widget::on_pushButtonPlay_clicked()
{

    playerwidget->playStream(ui->lineEditPlayUrl->text());
}

