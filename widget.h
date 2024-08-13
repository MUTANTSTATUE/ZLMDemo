#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <ffmpegthread.h>
#include<videoplayer.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButtonPush_clicked(bool checked);

    void on_pushButtonPlay_clicked();

private:
    Ui::Widget *ui;
    ffmpegThread * Thread;
    VideoPlayer *playerwidget;
};
#endif // WIDGET_H
