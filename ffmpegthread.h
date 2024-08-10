#ifndef FFMPEGTHREAD_H
#define FFMPEGTHREAD_H

#include "qprocess.h"
#include <QThread>
#include <qwidget.h>
#include "qdebug.h"
#include <signal.h>

class ffmpegThread :public QWidget
{
    Q_OBJECT
public:
    ffmpegThread();

    ~ffmpegThread();


    void run();

public slots:
    void stopFFmpge();

private slots:
    void handleOutput();
    void handleError();
private:
    QProcess* process;
};

#endif // FFMPEGTHREAD_H
