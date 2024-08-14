#ifndef FFMPEGTHREAD_H
#define FFMPEGTHREAD_H

#include <QThread>
#include <QImage>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
}

class ffmpegThread : public QThread
{
    Q_OBJECT
public:
    explicit ffmpegThread(QObject *parent = nullptr);
    ~ffmpegThread();

    void setUrl(const QString &nurl);
    void stop();

signals:
    void frameReady(const QImage &image);
    void audioDataReady(const QByteArray &data);

protected:
    void run() override;  // QThread 的 run 方法

private:
    bool initializeFFmpeg();
    void cleanupFFmpeg();
    void captureAndStream();
    bool initInputFormatContext();
    bool initOutputFormatContext();
    QStringList listAudioInputDevices();



    AVFormatContext *inputImageFmt_Ctx;//输入视频
    AVFormatContext *inputSoundFmt_Ctx;//输入音频
    AVFormatContext *outPutVideoFmt_Ctx;//输出处理后的音视频数据

    AVCodecContext *imageCodec_Ctx;
    AVCodecContext *soundCodec_Ctx;
    AVCodecContext *videoCodec_Ctx;

    int imageStreamIndex;
    int audioStreamIndex;
    bool running;
    QString url;
};

#endif // FFMPEGTHREAD_H
