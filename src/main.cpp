#include <QApplication>
#include <QtGui>
#include <QLibrary>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

#include "moneychanger.h"
#include "modules.h"

#include <OTAPI.h>
#include <OT_ME.h>
#include <OTLog.h>

void shutdown_app(){

}


class __OTclient_RAII
{
public:
    __OTclient_RAII()
    {
        // SSL gets initialized in here, before any keys are loaded.
        OTAPI_Wrap::AppInit();
    }
    ~__OTclient_RAII()
    {

        OTAPI_Wrap::AppCleanup();
    }
};


// ----------------------------------------



int main(int argc, char *argv[])
{
    // NB: you don't really have to do this if your library links ok
    // but I tested it and it works and gives a nice message, rw.
    //
    //QLibrary otapi("libotapi.so");     // <===== FIRST constructor called.
    //if (!otapi.load())
    //    qDebug() << otapi.errorString();
    //if (otapi.load())
    //    qDebug() << "otapi loaded";
    // ----------------------------------------
    // AppInit() is called here by this object's constructor. (And
    // AppCleanup() will be called automatically when we exit main(),
    // by this same object's destructor.)
    //
    __OTclient_RAII the_client_cleanup;  // <===== SECOND constructor is called here.
    // ----------------------------------------
    if (NULL == OTAPI_Wrap::It())
    {
        OTLog::vError(0, "Error, exiting: OTAPI_Wrap::AppInit() call must have failed.\n");
        return -1;
    }
    // ----------------------------------------
    //Init qApp
    QApplication a(argc, argv);  // <====== THIRD constructor (they are destroyed in reverse order.)

    //Config qApp
    QApplication::setQuitOnLastWindowClosed(false);
    // ----------------------------------------
    //Moneychanger Details
    QString mc_app_name = "moneychanger-qt";
    QString mc_version = "v0.0.x";

    //Compiled details
    QString mc_window_title = mc_app_name+" | "+mc_version;
    // ----------------------------------------
    // Load OTAPI Wallet
    //
    OTAPI_Wrap::It()->LoadWallet();

    // ----------------------------------------
    /** Init Moneychanger code (Start when nessecary below) **/
    Moneychanger systray;

    /** Init JSON wrapper **/

    { Modules modules; }    // run constructor once, initialize static pointers and free memory again (does this actually free any memory in a class like this?

    //Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    Modules::bitcoinRpc->ConnectToBitcoin("moneychanger", "money1234");
    Modules::json->GetInfo();
    double balance = Modules::json->GetBalance();
    QStringList accounts = Modules::json->ListAccounts();
    QString address = Modules::json->GetNewAddress();
    QStringList keys;
    keys.append(Modules::json->GetNewAddress());    // key can be an address or public key
    keys.append(Modules::json->GetNewAddress());
    QString multiSigAddr = Modules::json->AddMultiSigAddress(2, QJsonArray::fromStringList(keys));
    Modules::json->GetInfo();

    OTLog::vOutput(0, "Balance: %f\n", balance);
    if(address != NULL)
        OTLog::vOutput(0, "New address created: \"%s\"\n", address.toStdString().c_str());
    if(multiSigAddr != NULL)
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr.toStdString().c_str());


    //modules.json->SendToAddress("n3UmDg8a6W8j79GvjBVgdzRGHQcKuJ7ESk", 100.1234567); // testnet-box to bitcoin-qt, I think it only sends 100.123 instead of 100.1234567BTC.
    //modules.json->SendToAddress("n3dsfnpKhByZ3oVYzbLsCcJ2FDD8RRrJAC", 200); // bitcoin-qt to testnet-box


    /** Check Systray capabilities **/
    // This app relies on system tray capabilites;
    // Make sure the system has this capability before running
    //
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        /* ** ** **
         *Open dialog that will tell the user system tray is not available
         */
        //Create dialog
        QDialog * systray_notsupported = new QDialog(0);

        //Add details to the dialog window
        systray_notsupported->setWindowTitle(mc_window_title);

        //Create a vertical box to add to the dialog so multiple objects can be added to the window
        QVBoxLayout * systray_notsupported_vboxlayout = new QVBoxLayout(0);
        systray_notsupported->setLayout(systray_notsupported_vboxlayout);

        /* Add things to dialog */
        //Add label to dialog
        QLabel * systray_notsupported_main_msg_label = new QLabel("Your system doesn't seem to support <b>System Tray</b> capabilities.<br/>This program will not run with out it.");
        systray_notsupported_vboxlayout->addWidget(systray_notsupported_main_msg_label);

        //Show dialog
        systray_notsupported->show();
    }
    else
    {
        /* ** ** **
         *Start the Moneychanger systray app
         */
        systray.bootTray();
    }
    // ----------------------------------------------------------------
    // Leave this here for now, it will help debugging.
    int nServerCount = static_cast<int>(OTAPI_Wrap::GetServerCount());
    OTLog::vOutput(0, "Number of servers in the wallet: %d\n", nServerCount);
    // ----------------------------------------------------------------
    for (int i = 0; i < nServerCount; ++i)
    {
        std::string str_ServerID = OTAPI_Wrap::GetServer_ID(i);
        OTLog::vOutput(0, "Server at index %d has ID: %s\n", i, str_ServerID.c_str());
        // ---------------------------------------
        std::string str_ServerName = OTAPI_Wrap::GetServer_Name(str_ServerID);
        OTLog::vOutput(0, "The name for this server is: %s\n", str_ServerName.c_str());
    }
    // ----------------------------------------------------------------
    //
    int nExec = a.exec(); // <=== Here's where we run the QApplication...
    // ----------------------------------------------------------------
    OTLog::vOutput(0, "Finished executing the QApplication!\n(AppCleanup should occur "
                   "immediately after this point.)\nReturning: %d\n", nExec);
    // ----------------------------------------------------------------
    return nExec;
}
// Note: all the DESTRUCTORS for the local objects to main are called HERE -- in REVERSE ORDER.
//
// Here's the order of the constructors from the top of main():
//
// First: QLibrary otapi,
// Second: __OTclient_RAII the_client_cleanup
// Third: QApplication a
// Third: Moneychanger w
//
// Here's the order of the destructrors at the bottom of main():
//
// First:  Moneychanger w
// Second: QApplication a
// Third:  __OTclient_RAII the_client_cleanup
// Third:  QLibrary otapi,











