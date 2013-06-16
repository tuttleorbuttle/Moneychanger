#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moneychanger-qt
TEMPLATE = app

DEPENDPATH += .
SOURCES += main.cpp\
        moneychanger.cpp

HEADERS  += moneychanger.h

#LIBS += /usr/lib/libboost_thread.so.1.46.1 -ldl
##QMAKE_CXXFLAGS += -fPIC -DPIC --param ssp-buffer-size=4

# Set libraries and includes at end, to use platform-defined defaults if not overridden

# Dependency library locations can be customized with:
#    OPENTXS_INCLUDE_PATH, OPENTXS_LIB_PATH, CHAI_INCLUDE_PATH,
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

##OPENTXS_INCLUDE_PATH	+=
##OPENTXS_LIB_PATH	+=
##CHAI_INCLUDE_PATH	+=
##OPENSSL_INCLUDE_PATH	+=
##OPENSSL_LIB_PATH	+=

##INCLUDEPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##DEPENDPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##LIBS		+= $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
##LIBS 		+= -lotapi -lssl -lcrypto

## Automatic path from pkg-config

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opentxs

unix: PKGCONFIG += chaiscript

OTHER_FILES +=

RESOURCES += \
    resource.qrc
