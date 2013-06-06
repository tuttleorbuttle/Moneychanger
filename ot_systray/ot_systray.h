#ifndef OT_SYSTRAY_H
#define OT_SYSTRAY_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QIcon>

class ot_systray : public QWidget
{
    Q_OBJECT
    
public:
    ot_systray(QWidget *parent = 0);
    ~ot_systray();

private:
    /** ** ** ** ** ** ** **
     ** Private variables **
     **/
        /* System Tray */
            QSystemTrayIcon * ot_systrayicon; //Object
            QMenu * ot_systray_menu; //Tray menu
            QIcon ot_systray_idle_qicon; //Icon Image Resource(s)

            /* QMenu actions to system tray */
            QAction * quitAction;
};

#endif // OT_SYSTRAY_H
