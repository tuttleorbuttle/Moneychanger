#include <QApplication>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

#include "moneychanger.h"

int main(int argc, char *argv[])
{
    //Init qApp
    QApplication a(argc, argv);
        //Config qApp
        QApplication::setQuitOnLastWindowClosed(false);


    //Moneychanger Details
        QString mc_app_name = "Moneychanger";
        QString mc_version = "v0.0.x";

        //Compiled details
        QString mc_window_title = mc_app_name+" | "+mc_version;

    /** ** ** **
     * Begin Money Changer Main Loop Code()
     **/

        //This app relies on system tray capabilites; Make sure the system has this capability before running
        if (!QSystemTrayIcon::isSystemTrayAvailable()){
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
                        QLabel * systray_notsupported_main_msg_label = new QLabel("Your system dosen't seem to support <b>System Tray</b> capabilities.<br/>This program will not run with out it.");
                        systray_notsupported_vboxlayout->addWidget(systray_notsupported_main_msg_label);

                //Show dialog
                systray_notsupported->show();
        }else{
            /* ** ** **
             *Start the Moneychanger systray app
             */
        }



    Moneychanger w;
    //w.show();
    
    return a.exec();
}
