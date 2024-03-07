INCLUDEPATH += $$PWD


    greaterThan(QT_MAJOR_VERSION, 4) {
        contains(QT_CONFIG, libudev) {
            DEFINES += LINK_LIBUDEV
            INCLUDEPATH += $$QMAKE_INCDIR_LIBUDEV
            LIBS_PRIVATE += $$QMAKE_LIBS_LIBUDEV
        }
    } else {
        packagesExist(libudev) {
            CONFIG += link_pkgconfig
            DEFINES += LINK_LIBUDEV
            PKGCONFIG += libudev
        }
    }


PUBLIC_HEADERS += \
    $$PWD/qserialportglobal.h \
    $$PWD/qserialport.h \
    $$PWD/qserialportinfo.h

PRIVATE_HEADERS += \
    $$PWD/qserialport_p.h \
    $$PWD/qserialportinfo_p.h

SOURCES += \
    $$PWD/qserialport.cpp \
    $$PWD/qserialportinfo.cpp


    PRIVATE_HEADERS += \
        $$PWD/qserialport_unix_p.h

    SOURCES += \
        $$PWD/qserialport_unix.cpp \
        $$PWD/qserialportinfo_unix.cpp


SOURCES += \
     $$PWD/qlockfile.cpp \
     $$PWD/qlockfile_unix.cpp



HEADERS += \
    $$PWD/private/qcore_unix_p.h \
    $$PWD/private/qlockfile_p.h \
    $$PWD/private/qringbuffer_p.h \
    $$PWD/QtCore/qlockfile.h \
    $$PWD/QtCore/qwineventnotifier.h


HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
