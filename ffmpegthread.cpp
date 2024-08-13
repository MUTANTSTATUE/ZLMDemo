#include "ffmpegThread.h"
#include <QDebug>

ffmpegThread::ffmpegThread(QObject *parent)
    : QThread(parent), videoFormatContext(nullptr),audioFormatContext(nullptr), videoCodecContext(nullptr),
    audioCodecContext(nullptr), frame(nullptr), packet(nullptr),
    swsCtx(nullptr), videoStreamIndex(-1), audioStreamIndex(-1), running(false)
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



    // 设置视频输入格式
    const AVInputFormat *videoInputFormat = av_find_input_format("gdigrab");
    if (!videoInputFormat) {
        qWarning() << "Could not find video input format";
        return false;
    }

    // 设置视频输入设备（捕获整个桌面）
    QString videoUrl = QString("desktop");

    // 打开视频输入设备
    if (avformat_open_input(&videoFormatContext, videoUrl.toStdString().c_str(), videoInputFormat, nullptr) < 0) {
        qWarning() << "Could not open video input file";
        return false;
    }

    // 查找视频流信息
    if (avformat_find_stream_info(videoFormatContext, nullptr) < 0) {
        qWarning() << "Could not find video stream information";
        return false;
    }

    // 设置音频输入格式
    const AVInputFormat *audioInputFormat = av_find_input_format("dshow");
    if (!audioInputFormat) {
        qWarning() << "Could not find audio input format";
        return false;
    }

    QStringList deviceNames = listAudioInputDevices();


    // 假设用户选择了第一个设备（你可以根据实际需求进行选择）
    QString audioUrl;
    if (!deviceNames.isEmpty()) {
        QString selectedDevice = deviceNames[1]; // 选择第一个设备作为示例
        audioUrl= QString("audio=%1").arg(selectedDevice);
    } else {
        qWarning() << "No audio input devices found";
        return false;
    }


    // 打开音频输入设备
    if (avformat_open_input(&audioFormatContext, audioUrl.toStdString().c_str(), audioInputFormat, nullptr) < 0) {
        qWarning() << "Could not open audio input file with URL:" << audioUrl;
        return false;
    } else {
        qDebug() << "Successfully opened audio input device with URL:" << audioUrl;
    }

    // 查找音频流信息
    if (avformat_find_stream_info(audioFormatContext, nullptr) < 0) {
        qWarning() << "Could not find audio stream information";
        return false;
    }

    // 查找视频流和音频流
    videoStreamIndex = -1;
    audioStreamIndex = -1;

    for (int i = 0; i < videoFormatContext->nb_streams; ++i) {
        if (videoFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    for (int i = 0; i < audioFormatContext->nb_streams; ++i) {
        if (audioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qWarning() << "Could not find video stream";
        return false;
    }

    if (audioStreamIndex == -1) {
        qWarning() << "Could not find audio stream";
        return false;
    }

    // 获取视频流的编码器（选择 H.264 编码器）
    const AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!videoCodec) {
        qWarning() << "Video encoder not found";
        return false;
    }

    videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!videoCodecContext) {
        qWarning() << "Could not allocate video codec context";
        return false;
    }

    // 设置编码器参数
    videoCodecContext->bit_rate = 400000;
    videoCodecContext->width = 640; // 根据需要设置
    videoCodecContext->height = 480; // 根据需要设置
    videoCodecContext->time_base ={1, 30}; // 根据需要设置
    videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P; // FLV 支持的像素格式

    if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0) {
        qWarning() << "Could not open video codec";
        return false;
    }


    // 获取音频流的解码器
    const AVCodec *audioCodec = avcodec_find_decoder(audioFormatContext->streams[audioStreamIndex]->codecpar->codec_id);
    if (!audioCodec) {
        qWarning() << "Audio codec not found";
        return false;
    }

    audioCodecContext = avcodec_alloc_context3(audioCodec);
    if (!audioCodecContext) {
        qWarning() << "Could not allocate audio codec context";
        return false;
    }

    if (avcodec_parameters_to_context(audioCodecContext, audioFormatContext->streams[audioStreamIndex]->codecpar) < 0) {
        qWarning() << "Could not copy audio codec parameters to context";
        return false;
    }

    if (avcodec_open2(audioCodecContext, audioCodec, nullptr) < 0) {
        qWarning() << "Could not open audio codec";
        return false;
    }

    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        qWarning() << "Could not allocate frame or packet";
        return false;
    }

    swsCtx = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
                            videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_RGB24,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx) {
        qWarning() << "Could not initialize the conversion context";
        return false;
    }

    running = true;
    return true;
}

void ffmpegThread::cleanupFFmpeg() {
    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (packet) {
        av_packet_free(&packet);
    }
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
    }

}

void ffmpegThread::captureAndStream() {
    // 初始化 RTSP 推流
    AVFormatContext *outputFormatContext = nullptr;
    AVStream *videoStream = nullptr;
    AVStream *audioStream = nullptr;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    if (!packet || !frame) {
        qWarning() << "Could not allocate packet or frame";
        return;
    }

    // 设置 RTSP 输出格式
    const AVOutputFormat *outputFormat = av_guess_format("flv", nullptr, nullptr);
    if (!outputFormat) {
        qWarning() << "Could not find output format";
        return;
    }

    // 创建输出上下文
    if (avformat_alloc_output_context2(&outputFormatContext, outputFormat, nullptr, url.toStdString().c_str()) < 0) {
        qWarning() << "Could not create output context";
        return;
    }

    // 添加视频流到输出上下文
    const AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    videoStream = avformat_new_stream(outputFormatContext, videoCodec);
    if (!videoStream) {
        qWarning() << "Could not create video stream";
        return;
    }

    // 设置视频流参数
    videoStream->codecpar->codec_id = videoCodecContext->codec_id;
    videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    videoStream->codecpar->width = videoCodecContext->width;
    videoStream->codecpar->height = videoCodecContext->height;
    videoStream->codecpar->format = videoCodecContext->pix_fmt;
    videoStream->time_base = videoCodecContext->time_base;

    // 添加音频流到输出上下文
    audioStream = avformat_new_stream(outputFormatContext, nullptr);
    if (!audioStream) {
        qWarning() << "Could not create audio stream";
        return;
    }
    avcodec_parameters_copy(audioStream->codecpar, audioFormatContext->streams[audioStreamIndex]->codecpar);
    audioStream->time_base = audioFormatContext->streams[audioStreamIndex]->time_base;

    // 打开 RTSP 输出
    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&outputFormatContext->pb, url.toStdString().c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            char errorBuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errorBuf, sizeof(errorBuf));
            qWarning() << "Could not open output URL:" << url;
            qWarning() << "Error details:" << errorBuf << ret;
            return;
        }
    }

    // 写文件头
    int ret = avformat_write_header(outputFormatContext, nullptr);
    if (ret < 0) {
        char errorBuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errorBuf, sizeof(errorBuf));
        qWarning() << "Error occurred when writing header:" << errorBuf << ret;
        return;
    }

    while (running) {
        // 处理视频数据包
        if (av_read_frame(videoFormatContext, packet) >= 0) {
            if (packet->stream_index == videoStreamIndex) {
                if (avcodec_send_packet(videoCodecContext, packet) >= 0) {
                    while (avcodec_receive_frame(videoCodecContext, frame) >= 0) {
                        AVPacket outPacket;
                        av_init_packet(&outPacket);
                        outPacket.data = nullptr;
                        outPacket.size = 0;

                        if (avcodec_send_frame(videoCodecContext, frame) >= 0) {
                            while (avcodec_receive_packet(videoCodecContext, &outPacket) >= 0) {
                                outPacket.stream_index = videoStream->index;
                                av_write_frame(outputFormatContext, &outPacket);
                                av_packet_unref(&outPacket);
                            }
                        }
                    }
                }
            }
        }

        // 处理音频数据包
        if (av_read_frame(audioFormatContext, packet) >= 0) {
            if (packet->stream_index == audioStreamIndex) {
                if (avcodec_send_packet(audioCodecContext, packet) >= 0) {
                    while (avcodec_receive_frame(audioCodecContext, frame) >= 0) {
                        AVPacket outPacket;
                        av_init_packet(&outPacket);
                        outPacket.data = nullptr;
                        outPacket.size = 0;

                        if (avcodec_send_frame(audioCodecContext, frame) >= 0) {
                            while (avcodec_receive_packet(audioCodecContext, &outPacket) >= 0) {
                                outPacket.stream_index = audioStream->index;
                                av_write_frame(outputFormatContext, &outPacket);
                                av_packet_unref(&outPacket);
                            }
                        }
                    }
                }
            }
        }

        // 释放数据包
        av_packet_unref(packet);
    }

    // 写文件尾
    av_write_trailer(outputFormatContext);

    // 确保所有数据包都被写入
    if (videoCodecContext->frame_num > 0 || audioCodecContext->frame_num > 0) {
        av_write_trailer(outputFormatContext);
    }

    // 关闭输出
    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        avio_closep(&outputFormatContext->pb);
    }

    // 释放资源
    av_packet_free(&packet);
    av_frame_free(&frame);
    avformat_free_context(outputFormatContext);
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
