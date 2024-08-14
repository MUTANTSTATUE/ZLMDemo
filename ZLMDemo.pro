QT       += core gui
QT       += multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ffmpegthread.cpp \
    main.cpp \
    videodecoder.cpp \
    videoplayer.cpp \
    widget.cpp

HEADERS += \
    ffmpegthread.h \
    videodecoder.h \
    videoplayer.h \
    widget.h

FORMS += \
    widget.ui

#包含头文件
INCLUDEPATH += $$PWD/3rd/ffmpeg/include
#链接库文件
LIBS += -L$$PWD/3rd/ffmpeg/lib -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil -lavdevice -lpostproc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_apid
else:unix:!macx: LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_api


INCLUDEPATH += $$PWD/3rd/ZLMediaKit/include
DEPENDPATH += $$PWD/3rd/ZLMediaKit/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_api.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_apid.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/mk_api.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/mk_apid.lib
else:unix:!macx: PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_api.a

#ffmpeg文件导入
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rd/ffmpeg/lib/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rd/ffmpeg/lib/ -lavcodec
else:unix:!macx: LIBS += -L$$PWD/3rd/ffmpeg/lib/ -lavcode

INCLUDEPATH += $$PWD/3rd/ffmpeg/include/libavcodec
DEPENDPATH += $$PWD/3rd/ffmpeg/include/libavcodec

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ffmpeg/lib/libavcodec.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ffmpeg/lib/libavcodec.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ffmpeg/lib/avcodec.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ffmpeg/lib/avcodec.lib
else:unix:!macx: PRE_TARGETDEPS += $$PWD/3rd/ffmpeg/lib/libavcode.a

