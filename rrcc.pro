QT          += core gui widgets network websockets

TARGET      = rrcc

SOURCES     += mainwindow.cpp history.cpp timer.cpp setup.cpp logger.cpp upload.cpp packager.cpp unpackager.cpp converter.cpp about.cpp zones.cpp installer.cpp uninstaller.cpp update.cpp download.cpp search.cpp onlineupd.cpp
HEADERS     += mainwindow.h   history.h   timer.h   setup.h   logger.h   upload.h   packager.h   unpackager.h   converter.h   about.h   zones.h   installer.h   uninstaller.h   update.h   download.h   search.h   onlineupd.h

INCLUDEPATH += ext/qaes
SOURCES     += ext/qaes/qaesencryption.cpp
HEADERS     += ext/qaes/qaesencryption.h

QT          += concurrent
INCLUDEPATH += ext/qarchive
SOURCES     += ext/qarchive/qarchive.cpp
HEADERS     += ext/qarchive/qarchive.h
LIBS        += -larchive

INCLUDEPATH += ext/ccrypt
SOURCES     += ext/ccrypt/ccryptlib.c ext/ccrypt/rijndael.c ext/ccrypt/tables.c ext/ccrypt/platform.c
HEADERS     += ext/ccrypt/ccryptlib.h ext/ccrypt/rijndael.h ext/ccrypt/tables.h ext/ccrypt/platform.h

INCLUDEPATH += ext/qsshsocket
SOURCES     += ext/qsshsocket/qsshsocket.cpp
HEADERS     *= ext/qsshsocket/qsshsocket.h
LIBS        += -lssh

FORMS       += res/ui/mainwindow.ui res/ui/history.ui res/ui/timer.ui res/ui/setup.ui res/ui/logger.ui res/ui/upload.ui res/ui/packager.ui res/ui/unpackager.ui res/ui/converter.ui res/ui/about.ui res/ui/zones.ui res/ui/installer.ui res/ui/uninstaller.ui res/ui/update.ui res/ui/download.ui res/ui/search.ui res/ui/onlineupd.ui
RESOURCES   += res/rrcc.qrc

TRANSLATIONS+= lng/rrcc_de.ts

unix:!macx {
QMAKE_LFLAGS+= -Wl,-rpath=.
}

win32 {
RC_ICONS    += res/ico/app.ico
}

macx {
ICON        += res/ico/app.icns
}
