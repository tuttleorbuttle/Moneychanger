#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moneychanger-qt
TEMPLATE = app

INCLUDEPATH+="/usr/local/include/"
DEPENDPATH += .
SOURCES += main.cpp\
        moneychanger.cpp \
    ot_worker.cpp \
    MTRecordList.cpp \
    MTRecord.cpp

HEADERS  += moneychanger.h \
    ot_worker.h \
    ot_worker.h \
    MTRecordList.h \
    MTRecord.h

DEFINES += "OT_ZMQ_MODE=1"

mac:{
QT_CONFIG -= no-pkg-config
LIBS += -lboost_system-mt -ldl
}
#LIBS += /usr/lib/libboost_thread.so.1.46.1 -ldl
##QMAKE_CXXFLAGS += -fPIC -DPIC --param ssp-buffer-size=4

# Set libraries and includes at end, to use platform-defined defaults if not overridden

# Dependency library locations can be customized with:
#    OPENTXS_INCLUDE_PATH, OPENTXS_LIB_PATH, CHAI_INCLUDE_PATH,
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

OPENTXS_INCLUDE_PATH	+= /home/ubuntu/.local/include
OPENTXS_LIB_PATH	+= /home/ubuntu/.local/lib
CHAI_INCLUDE_PATH	+= /home/ubuntu/.local/include
OPENSSL_INCLUDE_PATH	+= /usr/lib/x86_64-linux-gnu
OPENSSL_LIB_PATH	+= /usr/include
OT_LIB_PATH

INCLUDEPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
DEPENDPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
LIBS		+= $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
LIBS 		+= -lotapi -lssl -lcrypto

INCLUDEPATH     += include/otapi
INCLUDEPATH     += include/otlib
INCLUDEPATH     += include

## Automatic path from pkg-config

#unix: QT_CONFIG -= no-pkg-config

#unix: CONFIG -= no-pkg-config

#unix: CONFIG += link_pkgconfig

#unix: PKGCONFIG += chaiscript

#unix: PKGCONFIG += opentxs

OTHER_FILES +=

RESOURCES += \
    resource.qrc
