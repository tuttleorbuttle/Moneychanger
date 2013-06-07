#include "moneychanger.h"

Moneychanger::Moneychanger(QWidget *parent)
    : QWidget(parent)
{
    /**
     * Init variables *
     */
    QDir dir;

    //Init MC System Tray Icon
    mc_systrayIcon = new QSystemTrayIcon(this);
    mc_systrayIcon->setIcon(QIcon(QString(dir.absolutePath()+"/icons/systray_icon.png")));

            //Init Icon resources (Loading resources/access Harddrive first; then send to GPU; This specific order will in thoery prevent bottle necking between HDD/GPU)
            mc_systrayIcon_overview = QIcon(QString(dir.absolutePath()+"/icons/overview.png"));

            mc_systrayIcon_nym = QIcon(QString(dir.absolutePath()+"/icons/nyms.png"));
            mc_systrayIcon_server = QIcon(QString(dir.absolutePath()+"/icons/server.png"));

            mc_systrayIcon_goldaccount = QIcon(QString(dir.absolutePath()+"/icons/goldaccount.png"));
            mc_systrayIcon_goldcashpurse = QIcon(QString(dir.absolutePath()+"/icons/goldaccount.png"));

            mc_systrayIcon_withdraw = QIcon(QString(dir.absolutePath()+"/icons/withdraw.png"));
            mc_systrayIcon_withdraw = QIcon(QString(dir.absolutePath()+"/icons/deposit.png"));

    //MC System tray menu
    mc_systrayMenu = new QMenu(this);

        //Init Skeleton of system tray menu
            //App name
            mc_systrayMenu_headertext = new QAction("Moneychanger Menu", 0);
            mc_systrayMenu_headertext->setDisabled(1);
            mc_systrayMenu->addAction(mc_systrayMenu_headertext);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Overview button
            mc_systrayMenu_overview = new QAction("Overview", 0);
            mc_systrayMenu_overview->setIcon(mc_systrayIcon_overview);
            mc_systrayMenu->addAction(mc_systrayMenu_overview);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Nym/Account section
            mc_systrayMenu_nym = new QAction("Nym: Trader Bob", 0);
            mc_systrayMenu_nym->setIcon(mc_systrayIcon_nym);
            mc_systrayMenu->addAction(mc_systrayMenu_nym);

            //Server section
            mc_systrayMenu_server = new QAction("Server: Digitails", 0);
            mc_systrayMenu_server->setIcon(mc_systrayIcon_server);
            mc_systrayMenu->addAction(mc_systrayMenu_server);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Gold account/cash purse/wallet
            mc_systrayMenu_goldaccount = new QAction("Gold Account: $60,000", 0);
            mc_systrayMenu_goldaccount->setIcon(mc_systrayIcon_goldaccount);
            mc_systrayMenu->addAction(mc_systrayMenu_goldaccount);

            //Gold cash purse wallet
            mc_systrayMenu_goldcashpurse = new QAction("Gold Cash Purse: $40,000", 0);
            mc_systrayMenu_goldcashpurse->setIcon(mc_systrayIcon_goldcashpurse);
            mc_systrayMenu->addAction(mc_systrayMenu_goldcashpurse);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Withdrawl
            mc_systrayMenu_widthdraw = new QAction("Withdraw", 0);
            mc_systrayMenu_widthdraw->setIcon(mc_systrayIcon_withdraw);
            mc_systrayMenu->addAction(mc_systrayMenu_widthdraw);

            //Deposit
            mc_systrayMenu_deposit = new QAction("Deposit", 0);
            mc_systrayMenu->addAction(mc_systrayMenu_deposit);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Send funds
            mc_systrayMenu_sendfunds = new QAction("Send Funds", 0);
            mc_systrayMenu->addAction(mc_systrayMenu_sendfunds);

            //Request payment
            mc_systrayMenu_requestpayment = new QAction("Request Payment", 0);
            mc_systrayMenu->addAction(mc_systrayMenu_requestpayment);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Advanced
            mc_systrayMenu_advanced = new QAction("Advanced", 0);

            //Seperator

            //Blank
            mc_systrayMenu_bottomblank = new QAction(" ", 0);
            mc_systrayMenu_bottomblank->setDisabled(1);
            mc_systrayMenu->addAction(mc_systrayMenu_bottomblank);

        //Set Skeleton to systrayIcon object code
        mc_systrayIcon->setContextMenu(mc_systrayMenu);

        //Show systray
        mc_systrayIcon->show();

}

Moneychanger::~Moneychanger()
{
    
}
