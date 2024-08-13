#include "ffmpegthread.h"
#include <QDebug>

ffmpegThread::ffmpegThread() : process(nullptr) {}

ffmpegThread::~ffmpegThread()
{
    if (process != nullptr) {
        stopFFmpeg();
    }
}

void ffmpegThread::setUrl(const QString &nurl)
{
    url=nurl;
}

void ffmpegThread::run()
{
    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, this, &ffmpegThread::handleOutput);
    connect(process, &QProcess::readyReadStandardError, this, &ffmpegThread::handleError);

    QString program = "ffmpeg";
    QStringList arguments;
    arguments << "-f" << "gdigrab"
              << "-framerate" << "30"
              << "-i" << "desktop"
              << "-video_size" << "2560x1440"
              << "-probesize" << "10M"
              << "-analyzeduration" << "10M"
              << "-c:v" << "libx264"
              << "-preset" << "ultrafast"
              << "-pix_fmt" << "yuv420p"
              << "-f" << "rtsp"
              << "-b:v" << "2M"
              << "-rtsp_transport" << "tcp"
              << url;


    process->start(program, arguments);

    if (!process->waitForStarted()) {
        qDebug() << "Failed to start FFmpeg process.";
        return;
    }
}

void ffmpegThread::stopFFmpeg()
{
    if (process) {
        process->kill();
        process->waitForFinished();
        delete process;
        process = nullptr;
    }
}

void ffmpegThread::handleOutput()
{
    qDebug() << "output :" << process->readAllStandardOutput();
}

void ffmpegThread::handleError()
{
    qDebug() << "error :" << process->readAllStandardError();
}
