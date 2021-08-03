QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



QT += serialport #添加Qt串口支持
CONFIG += console #显示控制台提示窗口，以方便调试，注释掉即可不显示黑色命令提示?
CONFIG += c++11
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Frame.cpp \
    main.cpp \
    mainwindow.cpp \
    port.cpp

HEADERS += \
    Frame.h \
    comData.h \
    mainwindow.h \
    port.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
