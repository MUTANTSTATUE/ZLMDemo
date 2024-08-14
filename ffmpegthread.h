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
#include <libavutil/opt.h>
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
    QStringList listAudioInputDevices();



    bool running;
    QString url;
};

#endif // FFMPEGTHREAD_H
