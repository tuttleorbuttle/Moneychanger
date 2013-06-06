#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H

#include <QWidget>
#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDir>


class Moneychanger : public QWidget
{
    Q_OBJECT
    
public:
    Moneychanger(QWidget *parent = 0);
    ~Moneychanger();
private:

    //Variables
        //MC Systray icon
        QSystemTrayIcon * mc_systrayIcon;

            QIcon mc_systrayIcon_overview;

            QIcon mc_systrayIcon_nym;
            QIcon mc_systrayIcon_server;

            QIcon mc_systrayIcon_goldaccount;
            QIcon mc_systrayIcon_goldcashpurse;

            QIcon mc_systrayIcon_withdraw;
            QIcon mc_systrayIcon_deposit;

        //MC Systray menu
        QMenu * mc_systrayMenu;

            //Systray menu skeleton
            QAction * mc_systrayMenu_headertext;

            QAction * mc_systrayMenu_overview;

            QAction * mc_systrayMenu_nym;
            QAction * mc_systrayMenu_server;

            QAction * mc_systrayMenu_goldaccount;
            QAction * mc_systrayMenu_goldcashpurse;

            QAction * mc_systrayMenu_widthdraw;
            QAction * mc_systrayMenu_deposit;

            QAction * mc_systrayMenu_sendfunds;
            QAction * mc_systrayMenu_requestpayment;

            QAction * mc_systrayMenu_advanced;

            QAction * mc_systrayMenu_bottomblank;
};

#endif // MONEYCHANGER_H
