#include "ffmpegthread.h"



ffmpegThread::ffmpegThread():process(nullptr) {}

ffmpegThread::~ffmpegThread()
{
    if (process != nullptr) {
        stopFFmpge();
    }
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
              << "-video_size" << "2560x1600"
              << "-f" << "rtsp"
              << "-rtsp_transport" << "tcp"
              << "rtsp://127.0.0.1/live/test";

    process->start(program, arguments);

    if (!process->waitForStarted()) {
        qDebug() << "Failed to start FFmpeg process.";
        return;
    }

    /*process->waitForFinished(-1);*/ // Wait indefinitely until the process finishes
}

void ffmpegThread::stopFFmpge()
{
    if (process) {
        process->kill();
        // process->terminate();
        process->waitForFinished();
        delete process;
        process = nullptr;
    }
}

void ffmpegThread::handleOutput()
{
    qDebug()<< " output :" << process->readAllStandardOutput();
}

void ffmpegThread::handleError()
{
    qDebug()<<"Error: " << process->readAllStandardError();
}

