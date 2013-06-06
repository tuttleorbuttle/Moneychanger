#include "ot_systray.h"

ot_systray::ot_systray(QWidget *parent)
    : QWidget(parent)
{

    QDir dir;
    QString exe_path = dir.absolutePath();
    //Initialize OT System Tray App code
        /** Systray **/
            //Init Systray Image/Icons
    qDebug() << exe_path;
            ot_systray_idle_qicon = QIcon(QString(exe_path+"/icons/systray_icon.png"));

            //Init Systray Object/Logic
            ot_systrayicon = new QSystemTrayIcon(ot_systray_idle_qicon, this);

            //Init Systray Menu
            ot_systray_menu = new QMenu(this);

                //Add quit action to systray menu
                quitAction = new QAction(tr("&Quit"), this);
                ot_systray_menu->addAction(quitAction);

           //Apply systray menu to systray object/logic
                ot_systrayicon->setContextMenu(ot_systray_menu);

           //Show systray visually (This will activate object/logic aswell)
                ot_systrayicon->show();

}

ot_systray::~ot_systray()
{
    
}
