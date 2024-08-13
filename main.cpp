#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();

    // 获取应用程序的运行目录
    QString appDir = QCoreApplication::applicationDirPath();
    // 构造相对路径
    QString debugSuffix = "/debug";
    if (appDir.endsWith(debugSuffix)) {
        appDir.chop(debugSuffix.length()); // 移除结尾的 "/debug"
    }
    QString releaseSuffix = "/release";
    if (appDir.endsWith(releaseSuffix)) {
        appDir.chop(releaseSuffix.length()); // 移除结尾的 "/release"
    }
    QString relativePath = "3rd/ZLMediaKit/bin/MediaServer.exe";
    QString programPath = appDir + "/" + relativePath;
    qDebug()<<programPath;

    // 创建 QProcess 对象
    QProcess process;

    // 构造启动新窗口的命令
    QString cmd = QString("cmd /c start \"\" \"%1\"").arg(programPath);

    // 启动程序
    process.startDetached(cmd);

    return a.exec();
}
