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

    void setUrl(const QString& nurl);
    void run();

public slots:
    void stopFFmpeg();

private slots:
    void handleOutput();
    void handleError();
private:
    QProcess* process;
    QString url;
};

#endif // FFMPEGTHREAD_H
