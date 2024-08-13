#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QThread>
#include <QImage>
#include <QPixmap>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

class VideoDecoder : public QThread
{
    Q_OBJECT
public:
    explicit VideoDecoder(QObject *parent = nullptr);
    ~VideoDecoder();

    void setUrl(const QString &url);
    void stop();

signals:
    void frameReady(const QImage &image);

protected:
    void run() override;

private:
    QString url;
    bool running;
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    SwsContext *swsContext;
    int videoStreamIndex;
    AVFrame *frame;
    AVFrame *rgbFrame;
    uint8_t *buffer;
};

#endif // VIDEODECODER_H
