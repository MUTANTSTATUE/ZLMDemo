#include "ffmpegThread.h"
#include <QDebug>

ffmpegThread::ffmpegThread(QObject *parent)
    : QThread(parent), running(true)
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
    qDebug()<<"here";
    int ret;
    while (running) {
        AVPacket packet;
        av_init_packet(&packet);

        // 从视频流中读取数据包
        if (av_read_frame(inputImageFmt_Ctx, &packet) >= 0) {
            if (packet.stream_index == imageStreamIndex) {
                qDebug() << "成功读取视频数据包，大小:" << packet.size;

                // 解码视频数据包
                int ret = avcodec_send_packet(imageCodec_Ctx, &packet);
                if (ret < 0) {
                    char errbuf[AV_ERROR_MAX_STRING_SIZE];
                    av_strerror(ret, errbuf, sizeof(errbuf));
                    qDebug() << "Error sending a packet for decoding:" << errbuf;
                } else {
                    AVFrame *frame = av_frame_alloc();
                    if (!frame) {
                        qDebug() << "Error allocating the frame";
                        return;
                    }

                    while (ret >= 0) {
                        ret = avcodec_receive_frame(imageCodec_Ctx, frame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        } else if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));
                            qDebug() << "Error during decoding:" << errbuf;
                            break;
                        }

                        // 编码视频帧并写入输出文件
                        AVPacket outPacket;
                        av_init_packet(&outPacket);
                        outPacket.data = nullptr;
                        outPacket.size = 0;

                        ret = avcodec_send_frame(videoCodec_Ctx, frame);
                        if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));
                            qDebug() << "Error sending a frame for encoding:" << errbuf<<" code: "<<ret;
                            break;
                        }

                        while (ret >= 0) {
                            ret = avcodec_receive_packet(videoCodec_Ctx, &outPacket);
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                                break;
                            } else if (ret < 0) {
                                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                                av_strerror(ret, errbuf, sizeof(errbuf));
                                qDebug() << "Error during encoding:" << errbuf;
                                break;
                            }

                            // 将编码后的包写入输出文件
                            ret = av_write_frame(outPutVideoFmt_Ctx, &outPacket);
                            if (ret < 0) {
                                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                                av_strerror(ret, errbuf, sizeof(errbuf));
                                qDebug() << "Error writing frame to output file:" << errbuf;
                            } else {
                                qDebug() << "成功写入视频数据包，大小:" << outPacket.size;
                            }
                            av_packet_unref(&outPacket);
                        }
                        av_frame_free(&frame);
                    }
                }
            }
            av_packet_unref(&packet);
        } else {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qDebug() << "Error reading frame from input:" << errbuf;
        }

        // 从音频流中读取数据包
        if (av_read_frame(inputSoundFmt_Ctx, &packet) >= 0 && packet.stream_index == audioStreamIndex) {
            // 解码音频数据包
            int ret = avcodec_send_packet(soundCodec_Ctx, &packet);
            if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                qDebug() << "Error sending a packet for audio decoding:" << errbuf;
            } else {
                AVFrame *frame = av_frame_alloc();
                if (!frame) {
                    qDebug() << "Error allocating the audio frame";
                    return;
                }

                while (ret >= 0) {
                    ret = avcodec_receive_frame(soundCodec_Ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        char errbuf[AV_ERROR_MAX_STRING_SIZE];
                        av_strerror(ret, errbuf, sizeof(errbuf));
                        qDebug() << "Error during audio decoding:" << errbuf;
                        break;
                    }

                    // 编码音频帧并写入输出文件
                    AVPacket outPacket;
                    av_init_packet(&outPacket);
                    outPacket.data = nullptr;
                    outPacket.size = 0;

                    ret = avcodec_send_frame(soundCodec_Ctx, frame);
                    if (ret < 0) {
                        char errbuf[AV_ERROR_MAX_STRING_SIZE];
                        av_strerror(ret, errbuf, sizeof(errbuf));
                        qDebug() << "Error sending a frame for audio encoding:" << errbuf;
                        break;
                    }

                    while (ret >= 0) {
                        ret = avcodec_receive_packet(soundCodec_Ctx, &outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        } else if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));
                            qDebug() << "Error during audio encoding:" << errbuf;
                            break;
                        }

                        // 将编码后的音频包写入输出文件
                        ret = av_write_frame(outPutVideoFmt_Ctx, &outPacket);
                        if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));
                            qDebug() << "Error writing audio frame to output file:" << errbuf;
                        }
                        av_packet_unref(&outPacket);
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

bool ffmpegThread::initInputFormatContext() {
    // 申请资源
    inputImageFmt_Ctx = avformat_alloc_context();
    inputSoundFmt_Ctx = avformat_alloc_context();
    imageCodec_Ctx = avcodec_alloc_context3(nullptr);
    soundCodec_Ctx = avcodec_alloc_context3(nullptr);

    if (!inputImageFmt_Ctx) {
        qDebug() << "Failed to allocate input image format context";
        return false;
    }

    if (!inputSoundFmt_Ctx) {
        qDebug() << "Failed to allocate input sound format context";
        return false;
    }

    if (!imageCodec_Ctx) {
        qDebug() << "Failed to allocate image codec context";
        return false;
    }

    if (!soundCodec_Ctx) {
        qDebug() << "Failed to allocate sound codec context";
        return false;
    }

    // 配置桌面捕获输入
    const AVInputFormat *inputVideoFormat = av_find_input_format("gdigrab");
    if (!inputVideoFormat) {
        qDebug() << "Failed to find video input format";
        return false;
    }

    if (avformat_open_input(&inputImageFmt_Ctx, "desktop", inputVideoFormat, nullptr) < 0) {
        qDebug() << "Unable to open desktop capture input";
        return false;
    }
    qDebug() << "Desktop capture input opened successfully";

    // 配置音频输入
    const AVInputFormat *inputAudioFormat = av_find_input_format("dshow");
    if (!inputAudioFormat) {
        qDebug() << "Failed to find audio input format";
        return false;
    }

    QStringList deviceNames = listAudioInputDevices();
    if (deviceNames.size() <= 1) {
        qWarning() << "No audio input devices found or not enough devices";
        return false;
    }

    QString audioUrl = QString("audio=%1").arg(deviceNames[1]); // 选择某个设备

    if (avformat_open_input(&inputSoundFmt_Ctx, audioUrl.toStdString().c_str(), inputAudioFormat, nullptr) < 0) {
        qDebug() << "Unable to open audio input";
        return false;
    }
    qDebug() << "Audio input opened successfully";

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
    const AVCodec *videoavcodec=(avcodec_find_encoder(AV_CODEC_ID_H264));
    videoCodec_Ctx = avcodec_alloc_context3(videoavcodec);
    outVideoStream->codecpar->codec_id = AV_CODEC_ID_H264;

    // 设置视频编码参数
    videoCodec_Ctx->codec_id = AV_CODEC_ID_H264;      // 编码器ID
    videoCodec_Ctx->codec_type = AVMEDIA_TYPE_VIDEO;  // 媒体类型
    videoCodec_Ctx->width = 2560;   // 视频宽度
    videoCodec_Ctx->height = 1440;  // 视频高度
    videoCodec_Ctx->bit_rate = 400000;  // 比特率，单位为bps
    videoCodec_Ctx->time_base = {1, 30};  // 时间基，假设为30fps
    videoCodec_Ctx->framerate = {30, 1};  // 帧率
    videoCodec_Ctx->gop_size = 12;  // 每个GOP中的帧数 (通常12帧一个GOP,即1个I帧 + 11个P/B帧)
    videoCodec_Ctx->max_b_frames = 2;  // B帧的最大数量，通常设置为2
    videoCodec_Ctx->pix_fmt = AV_PIX_FMT_YUV420P;  // 像素格式, 这里使用YUV420P
    // 选择合适的编码级别和配置
    av_opt_set(videoCodec_Ctx->priv_data, "preset", "medium", 0);  // 编码器预设级别，影响编码速度和质量，选项有"ultrafast", "superfast", "fast", "medium", "slow", "veryslow"
    av_opt_set(videoCodec_Ctx->priv_data, "profile", "main", 0);   // 编码器配置，"baseline", "main", "high" 是常见的配置
    // 帧内预测，影响编码效率
    videoCodec_Ctx->qmin = 10;  // 最小量化参数
    videoCodec_Ctx->qmax = 51;  // 最大量化参数
    videoCodec_Ctx->thread_count = 4;  // 使用多线程编码，设置线程数量
    videoCodec_Ctx->thread_type = FF_THREAD_SLICE | FF_THREAD_FRAME;  // 线程类型，通常设置为FF_THREAD_SLICE（按块多线程）和FF_THREAD_FRAME（按帧多线程）

    int ret = avcodec_open2(videoCodec_Ctx, videoavcodec, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "Could not open video codec:" << errbuf;
        return false;
    }
    avcodec_parameters_from_context(outVideoStream->codecpar, videoCodec_Ctx);

    // 创建音频编码器
    AVStream *outAudioStream = avformat_new_stream(outPutVideoFmt_Ctx, nullptr);
    if (!outAudioStream) {
        qDebug() << "Error creating new audio stream";
        return false;
    }
    soundCodec_Ctx = avcodec_alloc_context3(avcodec_find_encoder(AV_CODEC_ID_AAC));
    if (!soundCodec_Ctx) {
        qDebug() << "Could not allocate audio codec context";
        return false;
    }
    outAudioStream->codecpar->codec_id = AV_CODEC_ID_AAC;

    // 设置音频编码参数
    soundCodec_Ctx->sample_rate = 44100;
    AVChannelLayout ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    ret = av_channel_layout_copy(&soundCodec_Ctx->ch_layout, &ch_layout);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "Error copying channel layout:" << errbuf<<" error code: "<<ret;
        return false;
    }
    soundCodec_Ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    soundCodec_Ctx->bit_rate = 128000;
    ret = avcodec_open2(soundCodec_Ctx, avcodec_find_encoder(AV_CODEC_ID_AAC), nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "Error opening audio codec:" << errbuf<<" error code: "<<ret;
        return false;
    }
    ret = avcodec_parameters_from_context(outAudioStream->codecpar, soundCodec_Ctx);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "Error copying codec parameters to stream:" << errbuf;
        return false;
    }

    // 打开输出流
    if (!(outPutVideoFmt_Ctx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outPutVideoFmt_Ctx->pb, url.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
            qDebug() << "无法打开输出文件";
            return false;
        }
    }


    // 设置RTSP传输协议选项
    AVDictionary *options = nullptr;
    if(av_dict_set(&options, "rtsp_transport", "tcp", 0)<0) // 或 "udp"
    {
        qDebug()<<"options设置失败";
        return false;
    }
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
        qDebug()<<"初始化失败！";
        return;
    }
    running=true;
    captureAndStream();

    cleanupFFmpeg();
}
