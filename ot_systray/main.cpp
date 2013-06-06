#include "ot_systray.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Begin OT Systray App
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "NO SYSTRAY AVAILABLE";
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    ot_systray w;
    //w.show();
    
    return a.exec();
}
