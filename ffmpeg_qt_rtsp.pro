QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qcvdisplay.cpp \
    videoplayer.cpp

HEADERS += \
    mainwindow.h \
    qcvdisplay.h \
    videoplayer.h

FORMS += \
    mainwindow.ui


INCLUDEPATH +=  D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/include \
                D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/src \
                D:/opencv4/build/include

LIBS += D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/avcodec.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/avdevice.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/avfilter.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/avformat.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/avutil.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/postproc.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/swresample.lib \
        D:/ffmpeg-n4.4-latest-win64-gpl-shared-4.4/lib/swscale.lib \
        D:/opencv4/build/x64/vc14/lib/opencv_world455.lib \
        D:/opencv4/build/x64/vc14/lib/opencv_world455d.lib


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
