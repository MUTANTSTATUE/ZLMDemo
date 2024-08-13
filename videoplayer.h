#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>

class QLabel;
class QTimer;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

    void playStream(const QString &url);

private slots:
    void updateFrame();

private:
    QLabel *label;
    QTimer *timer;
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    SwsContext *swsContext;
    int videoStreamIndex;
    AVFrame *frame;
    AVFrame *rgbFrame;
    uint8_t *buffer;
};

#endif // VIDEOPLAYER_H
