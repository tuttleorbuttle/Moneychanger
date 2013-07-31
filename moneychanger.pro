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
    src/ot_worker.cpp \
    src/MTRecordList.cpp \
    src/MTRecord.cpp \
    src/unityindicator.cpp

HEADERS  += moneychanger.h \
    ot_worker.h \
    src/ot_worker.h \
    src/MTRecordList.h \
    src/MTRecord.h \
    src/unityindicator.h

DEFINES += "OT_ZMQ_MODE=1"

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
LIBAPPINDICATOR_INCLUDE_PATH += /usr/include/libappindicator-0.1
GTK_INCLUDE_PATH += /usr/include/gtk-2.0
GDKCONFIG_INCLUDE_PATH += /usr/lib/x86_64-linux-gnu/gtk-2.0/include
GLIB_INCLUDE_PATH += /usr/include/glib-2.0
GLIBCONFIG_INCLUDE_PATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include
PANGO_INCLUDE_PATH      += /usr/include/pango-1.0
CAIRO_INCLUDE_PATH      += /usr/include/cairo
GDK_PIXBUF_INCLUDE_PATH += /usr/include/gdk-pixbuf-2.0
ATK_INCLUDE_PATH        += /usr/include/atk-1.0

INCLUDEPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$LIBAPPINDICATOR_INCLUDE_PATH $$GTK_INCLUDE_PATH $$GLIB_INCLUDE_PATH $$GLIBCONFIG_INCLUDE_PATH $$PANGO_INCLUDE_PATH $$CAIRO_INCLUDE_PATH $$GDK_PIXBUF_INCLUDE_PATH $$ATK_INCLUDE_PATH $$GDKCONFIG_INCLUDE_PATH
DEPENDPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
LIBS		+= $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
LIBS 		+= -lotapi -lssl -lcrypto -lot -lboost_thread -lgobject-2.0 -lappindicator -lgtk-x11-2.0

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
