QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ffmpegthread.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    ffmpegthread.h \
    widget.h

FORMS += \
    widget.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_apid
else:unix:!macx: LIBS += -L$$PWD/3rd/ZLMediaKit/ -lmk_api

INCLUDEPATH += D:/ffmpeg-7.0.2/include
LIBS += -LD:/ffmpeg-7.0.2/lib  -lavdevice -lavformat -lavcodec -lavutil -lswscale

INCLUDEPATH += $$PWD/3rd/ZLMediaKit/include
DEPENDPATH += $$PWD/3rd/ZLMediaKit/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_api.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_apid.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/mk_api.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/mk_apid.lib
else:unix:!macx: PRE_TARGETDEPS += $$PWD/3rd/ZLMediaKit/libmk_api.a
