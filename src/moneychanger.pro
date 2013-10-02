#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moneychanger-qt
TEMPLATE = app

INCLUDEPATH+="/usr/local/include/"
DEPENDPATH += .
SOURCES += main.cpp\
           moneychanger.cpp \
           ot_worker.cpp \
           MTRecordList.cpp \
           MTRecord.cpp \
          modules.cpp \
          btcjson.cpp \
          btcinterface.cpp \
          btcrpc.cpp \
    unityindicator.cpp \
    utils.cpp \
    Widgets/MarketWindow.cpp \
    Widgets/overviewwindow.cpp \
    Handlers/FileHandler.cpp \
    Handlers/DBHandler.cpp \
    Widgets/addressbookwindow.cpp \
    Widgets/nymmanagerwindow.cpp \
    Widgets/assetmanagerwindow.cpp \
    Widgets/accountmanagerwindow.cpp \
    Widgets/servermanagerwindow.cpp \
    Widgets/withdrawascashwindow.cpp \
    Widgets/withdrawasvoucherwindow.cpp \
    Widgets/depositwindow.cpp \
    Widgets/sendfundswindow.cpp \
    Widgets/requestfundswindow.cpp \
    Widgets/home.cpp \
    Widgets/homedetail.cpp \
    Handlers/contacthandler.cpp \
    Widgets/createinsurancecompany.cpp \


HEADERS += moneychanger.h \
           ot_worker.h \
    MTRecord.hpp \
    MTRecordList.hpp \
            modules.h \
            IStringProcessor.h \
            MTRecord.hpp \
            MTRecordList.hpp \
            FastDelegate.h \
            FastDelegateBind.h \
    btcjson.h \
    btcrpc.h \
    btcinterface.h \
    utils.h \
    unityindicator.h \
    Widgets/MarketWindow.h \
    Widgets/overviewwindow.h \
    Handlers/FileHandler.h \
    Handlers/DBHandler.h \
    Widgets/addressbookwindow.h \
    Widgets/nymmanagerwindow.h \
    Widgets/assetmanagerwindow.h \
    Widgets/accountmanagerwindow.h \
    Widgets/servermanagerwindow.h \
    Widgets/withdrawascashwindow.h \
    Widgets/withdrawasvoucherwindow.h \
    Widgets/depositwindow.h \
    Widgets/sendfundswindow.h \
    Widgets/requestfundswindow.h \
    Widgets/home.h \
    Widgets/homedetail.h \
    Handlers/contacthandler.h \
    Widgets/createinsurancecompany.h \


DEFINES += "OT_ZMQ_MODE=1"

mac:{
    QMAKE_CXXFLAGS -= -fstack-check
	QT_CONFIG -= no-pkg-config
	LIBS += -lboost_system-mt -lboost_thread-mt -ldl
}


linux:{
	LIBS += -lboost_system -lboost_thread -ldl
}

##QMAKE_CXXFLAGS += -fPIC -DPIC --param ssp-buffer-size=4

# Set libraries and includes at end, to use platform-defined defaults if not overridden

# Dependency library locations can be customized with:
#    OPENTXS_INCLUDE_PATH, OPENTXS_LIB_PATH, CHAI_INCLUDE_PATH,
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

# Sorry about the mess here, I'll figure out how to clan that up another time.

HOME = $$(HOME)
OPENTXS_INCLUDE_PATH   += $$HOME/.local/include/opentxs
OPENTXS_LIB_PATH       += $$HOME/.local/lib
CHAI_INCLUDE_PATH      += $$HOME/.local/include
OPENSSL_INCLUDE_PATH   += /usr/lib/x86_64-linux-gnu
OPENSSL_LIB_PATH       += /usr/include

# I think all of the following includes are for unityindicator.h so they can easily be removed later.
LIBAPPINDICATOR_INCLUDE_PATH += /usr/include/libappindicator-0.1
GTK_INCLUDE_PATH        += /usr/include/gtk-2.0
GDKCONFIG_INCLUDE_PATH  += /usr/lib/x86_64-linux-gnu/gtk-2.0/include
GLIB_INCLUDE_PATH       += /usr/include/glib-2.0
GLIBCONFIG_INCLUDE_PATH += /usr/lib/x86_64-linux-gnu/glib-2.0/include
PANGO_INCLUDE_PATH      += /usr/include/pango-1.0
CAIRO_INCLUDE_PATH      += /usr/include/cairo
GDK_PIXBUF_INCLUDE_PATH += /usr/include/gdk-pixbuf-2.0
ATK_INCLUDE_PATH        += /usr/include/atk-1.0

INCLUDEPATH    += $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
INCLUDEPATH     += $$LIBAPPINDICATOR_INCLUDE_PATH $$GTK_INCLUDE_PATH
INCLUDEPATH     += $$GLIB_INCLUDE_PATH $$GLIBCONFIG_INCLUDE_PATH
INCLUDEPATH     += $$PANGO_INCLUDE_PATH $$CAIRO_INCLUDE_PATH
INCLUDEPATH     += $$GDK_PIXBUF_INCLUDE_PATH $$ATK_INCLUDE_PATH $$GDKCONFIG_INCLUDE_PATH
DEPENDPATH     += $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
LIBS           += $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
LIBS           += -lotapi -lssl -lcrypto -lot -lboost_thread -lgobject-2.0 -lappindicator -lgtk-x11-2.0

INCLUDEPATH     += include/otapi
INCLUDEPATH     += include/otlib
INCLUDEPATH     += include

## Automatic path from pkg-config

unix: CONFIG += link_pkgconfig
#unix: PKGCONFIG += opentxs

#unix: PKGCONFIG += chaiscript

#OTHER_FILES +=

RESOURCES += resource.qrc

OTHER_FILES +=

FORMS += \
    UI/marketwindow.ui \
    Widgets/home.ui \
    Widgets/homedetail.ui \ 
    UI/createinsurancecompany.ui

