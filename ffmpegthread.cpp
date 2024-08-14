#include "ffmpegThread.h"
#include <QDebug>

ffmpegThread::ffmpegThread(QObject *parent)
    : QThread(parent), running(false)
{
    // 初始化 FFmpeg 设备
    avdevice_register_all();

    avformat_network_init();

    AVFormatContext *inputImageFmt_Ctx = nullptr;
    AVFormatContext *inputSoundFmt_Ctx = nullptr;
    AVFormatContext *outPutVideoFmt_Ctx = nullptr;
    AVCodecContext *imageCodec_Ctx = nullptr;
    AVCodecContext *soundCodec_Ctx = nullptr;
    AVCodecContext *videoCodec_Ctx = nullptr;
    imageStreamIndex = -1;
    audioStreamIndex = -1;
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
    return initInputFormatContext()&&initOutputFormatContext();
}

void ffmpegThread::cleanupFFmpeg() {
    avformat_close_input(&inputImageFmt_Ctx);
    avformat_close_input(&inputSoundFmt_Ctx);
    avformat_free_context(outPutVideoFmt_Ctx);
    avcodec_free_context(&imageCodec_Ctx);
    avcodec_free_context(&soundCodec_Ctx);
    avcodec_free_context(&videoCodec_Ctx);

}

//捕获并输出
void ffmpegThread::captureAndStream() {
    imageStreamIndex = -1;
    audioStreamIndex = -1;

    // 找到视频和音频流的索引
    for (int i = 0; i < inputImageFmt_Ctx->nb_streams; ++i) {
        if (inputImageFmt_Ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            imageStreamIndex = i;
            const AVCodec *videoCodec = avcodec_find_decoder(inputImageFmt_Ctx->streams[i]->codecpar->codec_id);
            if (!videoCodec) {
                qDebug() << "未找到视频解";
                return;
            }
            imageCodec_Ctx = avcodec_alloc_context3(videoCodec);
            avcodec_parameters_to_context(imageCodec_Ctx, inputImageFmt_Ctx->streams[i]->codecpar);
            if (avcodec_open2(imageCodec_Ctx, videoCodec, nullptr) < 0) {
                qDebug() << "无法打开视频解 " ;
                return;
            }
        }
    }

    for (int i = 0; i < inputSoundFmt_Ctx->nb_streams; ++i) {
        if (inputSoundFmt_Ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            const AVCodec *audioCodec = avcodec_find_decoder(inputSoundFmt_Ctx->streams[i]->codecpar->codec_id);
            if (!audioCodec) {
                qDebug() << "未找到音频解码器" ;
                return;
            }
            soundCodec_Ctx = avcodec_alloc_context3(audioCodec);
            avcodec_parameters_to_context(soundCodec_Ctx, inputSoundFmt_Ctx->streams[i]->codecpar);
            if (avcodec_open2(soundCodec_Ctx, audioCodec, nullptr) < 0) {
                qDebug() << "  无法打开音频解码器 " ;
                return;
            }
        }
    }

    if (imageStreamIndex == -1 || audioStreamIndex == -1) {
        qDebug() << "没有找到视频或音频流";
        return;
    }

    AVPacket packet;
    av_init_packet(&packet);

    while (running) {
        // 从视频流中读取数据包
        if (av_read_frame(inputImageFmt_Ctx, &packet) >= 0 && packet.stream_index == imageStreamIndex) {
            // 解码视频数据包
            if (avcodec_send_packet(imageCodec_Ctx, &packet) == 0) {
                AVFrame *frame = av_frame_alloc();
                while (avcodec_receive_frame(imageCodec_Ctx, frame) == 0) {
                    // 编码视频帧并写入输出文件
                    AVPacket outPacket;
                    av_init_packet(&outPacket);
                    outPacket.data = nullptr;
                    outPacket.size = 0;

                    if (avcodec_send_frame(videoCodec_Ctx, frame) == 0) {
                        while (avcodec_receive_packet(videoCodec_Ctx, &outPacket) == 0) {
                            av_write_frame(outPutVideoFmt_Ctx, &outPacket);
                            av_packet_unref(&outPacket);
                        }
                    }
                    av_frame_free(&frame);
                }
            }
            av_packet_unref(&packet);
        }

        // 从音频流中读取数据包
        if (av_read_frame(inputSoundFmt_Ctx, &packet) >= 0 && packet.stream_index == audioStreamIndex) {
            // 解码音频数据包
            if (avcodec_send_packet(soundCodec_Ctx, &packet) == 0) {
                AVFrame *frame = av_frame_alloc();
                while (avcodec_receive_frame(soundCodec_Ctx, frame) == 0) {
                    // 编码音频帧并写入输出文件
                    AVPacket outPacket;
                    av_init_packet(&outPacket);
                    outPacket.data = nullptr;
                    outPacket.size = 0;

                    if (avcodec_send_frame(soundCodec_Ctx, frame) == 0) {
                        while (avcodec_receive_packet(soundCodec_Ctx, &outPacket) == 0) {
                            av_write_frame(outPutVideoFmt_Ctx, &outPacket);
                            av_packet_unref(&outPacket);
                        }
                    }
                    av_frame_free(&frame);
                }
            }
            av_packet_unref(&packet);
        }
    }

    // 写入文件尾
    av_write_trailer(outPutVideoFmt_Ctx);

}

bool ffmpegThread::initInputFormatContext()
{
    //申请资源
    inputImageFmt_Ctx=avformat_alloc_context();
    inputSoundFmt_Ctx=avformat_alloc_context();
    imageCodec_Ctx=avcodec_alloc_context3(nullptr);
    soundCodec_Ctx=avcodec_alloc_context3(nullptr);

    // 配置桌面捕获输入
    const AVInputFormat *inputVideoFormat = av_find_input_format("gdigrab");
    if (avformat_open_input(&inputImageFmt_Ctx, "desktop", inputVideoFormat, nullptr) < 0) {
        qDebug() << "无法打开桌面捕获输入";
        return false;
    }

    // 配置音频输入
    const AVInputFormat *inputAudioFormat = av_find_input_format("dshow");

    QStringList deviceNames = listAudioInputDevices();
    QString audioUrl;
    if (!deviceNames.isEmpty()) {
        QString selectedDevice = deviceNames[1];//选择某个设备
        audioUrl= QString("audio=%1").arg(selectedDevice);
    } else {
        qWarning() << "No audio input devices found";
        return false;
    }
    if (avformat_open_input(&inputSoundFmt_Ctx, audioUrl.toStdString().c_str(), inputAudioFormat, nullptr) < 0) {
        qDebug() << "无法打开音频输入";
        return false;
    }
    return true;
}

bool ffmpegThread::initOutputFormatContext()
{
    videoCodec_Ctx=avcodec_alloc_context3(nullptr);
    avformat_alloc_output_context2(&outPutVideoFmt_Ctx, nullptr, "rtsp", url.toStdString().c_str());
    if (!outPutVideoFmt_Ctx) {
        qDebug() << "无法分配输出";
        return false;
    }

    // 创建视频编码器
    AVStream *outVideoStream = avformat_new_stream(outPutVideoFmt_Ctx, nullptr);
    videoCodec_Ctx = avcodec_alloc_context3(avcodec_find_encoder(AV_CODEC_ID_H264));
    outVideoStream->codecpar->codec_id = AV_CODEC_ID_H264;

    // 设置视频编码参数
    videoCodec_Ctx->width = 1920; // 假设桌面分辨率为1920x1080
    videoCodec_Ctx->height = 1080;
    videoCodec_Ctx->time_base = {1, 30};  // 假设30fps
    videoCodec_Ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    avcodec_open2(videoCodec_Ctx, avcodec_find_encoder(AV_CODEC_ID_H264), nullptr);
    avcodec_parameters_from_context(outVideoStream->codecpar, videoCodec_Ctx);

    // 创建音频编码器
    AVStream *outAudioStream = avformat_new_stream(outPutVideoFmt_Ctx, nullptr);
    soundCodec_Ctx = avcodec_alloc_context3(avcodec_find_encoder(AV_CODEC_ID_AAC));
    outAudioStream->codecpar->codec_id = AV_CODEC_ID_AAC;

    // 设置音频编码参数
    soundCodec_Ctx->sample_rate = 44100;
    AVChannelLayout ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    av_channel_layout_copy(&soundCodec_Ctx->ch_layout, &ch_layout);
    soundCodec_Ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    soundCodec_Ctx->bit_rate = 128000;
    avcodec_open2(soundCodec_Ctx, avcodec_find_encoder(AV_CODEC_ID_AAC), nullptr);
    avcodec_parameters_from_context(outAudioStream->codecpar, soundCodec_Ctx);

    // 打开输出流
    if (!(outPutVideoFmt_Ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outPutVideoFmt_Ctx->pb, url.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            qDebug() << "无法打开输出文件";
            return false;
        }
    }

    // 设置RTSP传输协议选项
    AVDictionary *options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0); // 或 "udp"

    // 写入头部，并应用RTSP选项
    if (avformat_write_header(outPutVideoFmt_Ctx, &options) < 0) {
        qDebug() << "无法写入头部";
        return false;
    }
    av_dict_free(&options);

    return true;
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
