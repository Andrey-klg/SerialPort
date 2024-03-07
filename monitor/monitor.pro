QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

CONFIG += c++11

#CONFIG(release,debug|release){
#    DEFINES += QT_NO_DEBUG_OUTPUT
#    DEFINES += QT_NO_WARNING_OUTPUT
#}

MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
UI_DIR = tmp/ui
RCC_DIR = tmp/rcc

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tmi.cpp \
    tmithread.cpp

HEADERS += \
    mainwindow.h \
    tmi.h \
    tmithread.h

FORMS += \
    mainwindow.ui

#INCLUDEPATH += $${PWD}
#LIBS+= -lQt5SerialPortd

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

lessThan(QT_MAJOR_VERSION, 5): include(SerialPort/serialport-lib.pri)


RESOURCES += \
    res.qrc

RC_ICONS = res/icon.ico
