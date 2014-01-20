#-------------------------------------------------
#
# Project created by QtCreator 2014-01-20T16:23:09
#
#-------------------------------------------------

#QT       -= core gui

TARGET = bitcoin-api
TEMPLATE = lib subdirs

SUBDIRS += src
INCLUDEPATH += src

DEFINES += BITCOINAPI_LIBRARY

SOURCES +=  src/bitcoinapi.cpp \
    src/MTBitcoin.cpp \
    src/btcrpccurl.cpp \
    src/btchelper.cpp \
    src/btcjson.cpp \
    src/btcmodules.cpp \
    src/btcobjects.cpp \
    src/btctest.cpp

HEADERS +=  src/bitcoinapi.h\
            src/bitcoin-api_global.h \
    src/btcrpccurl.h \
    src/btcobjects.h \
    src/btchelper.h \
    src/btcjson.h \
    src/btcmodules.h \
    src/MTBitcoin.h \
    src/btctest.h \
    src/ibtcrpc.h \
    src/ibtcjson.h \
    src/imtbitcoin.h


QMAKE_CXXFLAGS += -std=c++11

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opentxs

CONFIG += debug #_and_release

LIBS += -lcurl -ljsoncpp  # cross-platform communication with bitcoind

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
