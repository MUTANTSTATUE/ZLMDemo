#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QImage>

class QLabel;
class QTimer;
class VideoDecoder;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

    void playStream(const QString &url);

private slots:
    void onFrameReady(const QImage &image);
    void resizeEvent(QResizeEvent *event);
private:
    QLabel *label;
    VideoDecoder *decoder;
};

#endif // VIDEOPLAYER_H
