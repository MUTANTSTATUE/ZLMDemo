#include "ffmpegThread.h"
#include <QDebug>

ffmpegThread::ffmpegThread(QObject *parent)
    : QThread(parent), running(false)
{
    // 初始化 FFmpeg 设备
    avdevice_register_all();

    avformat_network_init();
}

ffmpegThread::~ffmpegThread() {
    stop();
    wait();  // 等待线程安全退出
}

void ffmpegThread::setUrl(const QString &nurl) {
    url = nurl;
}

void ffmpegThread::stop() {
    running = false;
}

bool ffmpegThread::initializeFFmpeg() {

    return false;
}

void ffmpegThread::cleanupFFmpeg() {


}

void ffmpegThread::captureAndStream() {

}




QStringList ffmpegThread::listAudioInputDevices() {
    QStringList deviceNames;
    AVDeviceInfoList *deviceList = nullptr;
    const AVInputFormat *inputFormat = av_find_input_format("dshow");

    if (!inputFormat) {
        qWarning() << "Could not find audio input format";
        return deviceNames;
    }

    // 列出所有输入设备
    if (avdevice_list_input_sources(inputFormat, nullptr, nullptr, &deviceList) < 0) {
        qWarning() << "Could not list input devices";
        return deviceNames;
    }

    // 打印设备名称和详细信息，并存储设备名称
    for (int i = 0; i < deviceList->nb_devices; ++i) {
        AVDeviceInfo *device = deviceList->devices[i];
        QString deviceName = QString::fromUtf8(device->device_name);
        QString deviceDescription = device->device_description ? QString::fromUtf8(device->device_description) : "Unknown";
        qDebug() << "Device name:" << deviceName;
        qDebug() << "Device description:" << deviceDescription;

        deviceNames << deviceDescription;
    }

    // 释放设备列表
    avdevice_free_list_devices(&deviceList);
    return deviceNames;
}



void ffmpegThread::run() {
    if (!initializeFFmpeg()) {
        return;
    }

    captureAndStream();

    cleanupFFmpeg();
}
