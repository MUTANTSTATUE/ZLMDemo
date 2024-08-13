#include "VideoPlayer.h"
#include "VideoDecoder.h"
#include "qlabel.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QResizeEvent>

VideoPlayer::VideoPlayer(QWidget *parent) :
    QWidget(parent),
    label(new QLabel(this)),
    decoder(new VideoDecoder(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);  // 使 QLabel 填满整个窗口

    connect(decoder, &VideoDecoder::frameReady, this, &VideoPlayer::onFrameReady);
}

VideoPlayer::~VideoPlayer()
{
    decoder->stop();
    decoder->wait();  // 确保线程安全退出
    delete decoder;
}

void VideoPlayer::playStream(const QString &url)
{
    decoder->setUrl(url);
    decoder->start();  // 启动解码线程
}

void VideoPlayer::onFrameReady(const QImage &image)
{
    // 将图像调整为 QLabel 的大小
    QImage scaledImage = image.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(QPixmap::fromImage(scaledImage));
}

void VideoPlayer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!decoder) return;

    // 重新调整 QLabel 的大小
    label->resize(event->size());

    // 触发更新，确保图像适应新尺寸
    // 注意：如果图像已经加载完毕，需要根据需求更新显示尺寸
}
