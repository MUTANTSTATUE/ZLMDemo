#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <ffmpegthread.h>
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
    void on_pushButton_clicked(bool checked);

private:
    Ui::Widget *ui;
    ffmpegThread * Thread;
};
#endif // WIDGET_H
