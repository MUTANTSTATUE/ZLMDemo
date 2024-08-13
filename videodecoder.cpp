#include "VideoDecoder.h"
#include <QDebug>

VideoDecoder::VideoDecoder(QObject *parent) :
    QThread(parent),
    formatContext(nullptr),
    codecContext(nullptr),
    swsContext(nullptr),
    videoStreamIndex(-1),
    frame(nullptr),
    rgbFrame(nullptr),
    buffer(nullptr),
    running(true)
{
}

VideoDecoder::~VideoDecoder()
{
    stop();
    wait();  // 等待线程安全退出

    if (buffer) {
        av_free(buffer);
    }

    if (rgbFrame) {
        av_frame_free(&rgbFrame);
    }

    if (frame) {
        av_frame_free(&frame);
    }

    if (swsContext) {
        sws_freeContext(swsContext);
    }

    if (codecContext) {
        avcodec_free_context(&codecContext);
    }

    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}

void VideoDecoder::setUrl(const QString &url)
{
    this->url = url;
}

void VideoDecoder::stop()
{
    running = false;
}

void VideoDecoder::run()
{
    formatContext = avformat_alloc_context();

    if (avformat_open_input(&formatContext, url.toStdString().c_str(), nullptr, nullptr) != 0) {
        qDebug() << "Couldn't open stream.";
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        qDebug() << "Couldn't find stream information.";
        return;
    }

    videoStreamIndex = -1;
    const AVCodec *codec = nullptr;

    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            codec = avcodec_find_decoder(formatContext->streams[i]->codecpar->codec_id);
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qDebug() << "Couldn't find a video stream.";
        return;
    }

    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar);
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        qDebug() << "Couldn't open codec.";
        return;
    }

    swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);

    frame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24,
                         codecContext->width, codecContext->height, 1);

    AVPacket packet;
    av_init_packet(&packet);

    while (running) {
        if (av_read_frame(formatContext, &packet) >= 0) {
            if (packet.stream_index == videoStreamIndex) {
                if (avcodec_send_packet(codecContext, &packet) == 0) {
                    while (avcodec_receive_frame(codecContext, frame) == 0) {
                        sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height,
                                  rgbFrame->data, rgbFrame->linesize);

                        QImage image(rgbFrame->data[0], codecContext->width, codecContext->height, QImage::Format_RGB888);
                        emit frameReady(image);
                    }
                }
                av_packet_unref(&packet);
            }
        }
    }
}
