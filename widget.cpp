#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    Thread = new ffmpegThread(this);

}

Widget::~Widget()
{
    Thread->stopFFmpge();
    delete ui;
}

void Widget::on_pushButton_clicked(bool checked)
{
    if(checked)
    {
        Thread->run();
    }
    else{
        Thread->stopFFmpge();
    }
}

