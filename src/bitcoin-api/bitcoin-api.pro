#-------------------------------------------------
#
# Project created by QtCreator 2014-01-20T16:23:09
#
#-------------------------------------------------

#QT       -= core gui

TARGET = bitcoin-api
TEMPLATE = lib subdirs

SUBDIRS += src

DEFINES += BITCOINAPI_LIBRARY

SOURCES +=  src/bitcoinapi.cpp

HEADERS +=  src/bitcoinapi.h\
            src/bitcoin-api_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
