#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H


#include <QMap>
#include <QMenu>
#include <QDebug>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QAction>
#include <QVariant>
#include <QtSql/QSqlDatabase>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QCoreApplication>


#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>


class Moneychanger : public QWidget
{
    Q_OBJECT

public:
    /** Constructor & Destructor **/
    Moneychanger(QWidget *parent = 0);
    ~Moneychanger();

    /** Start **/
    void bootTray();

private:

    /**           **
     ** Variables **
     **           **/
        //Open Transaction
        OT_ME * ot_me;

        //MC Address book
        int mc_addressbook_already_init;

            //Dialog
            QDialog * mc_addressbook_dialog;
            QGridLayout * mc_addressbook_gridlayout;

                //Label
                QLabel * mc_addressbook_label;

                //Table view
                QTableView * mc_addressbook_tableview;

        //MC Systray icon
        QSystemTrayIcon * mc_systrayIcon;

            QIcon mc_systrayIcon_shutdown;

            QIcon mc_systrayIcon_overview;

            QIcon mc_systrayIcon_nym;
            QIcon mc_systrayIcon_server;

            QIcon mc_systrayIcon_goldaccount;
            QIcon mc_systrayIcon_goldcashpurse;

            QIcon mc_systrayIcon_withdraw;
            QIcon mc_systrayIcon_deposit;

            QIcon mc_systrayIcon_sendfunds;
            QIcon mc_systrayIcon_requestpayment;

            QIcon mc_systrayIcon_advanced;




        //MC Systray menu
        QMenu * mc_systrayMenu;

            //Systray menu skeleton
            QAction * mc_systrayMenu_headertext;
            QAction * mc_systrayMenu_aboveBlank;

            QAction * mc_systrayMenu_shutdown;

            QAction * mc_systrayMenu_overview;

            QAction * mc_systrayMenu_nym;
            QAction * mc_systrayMenu_server;

            QAction * mc_systrayMenu_goldaccount;
            QAction * mc_systrayMenu_goldcashpurse;

            QMenu * mc_systrayMenu_withdraw;
                //Withdraw submenu
                QAction * mc_systrayMenu_withdraw_ascash;
                QAction * mc_systrayMenu_withdraw_asvoucher;

            QAction * mc_systrayMenu_deposit;

            QAction * mc_systrayMenu_sendfunds;
            QAction * mc_systrayMenu_requestpayment;

            QAction * mc_systrayMenu_advanced;

            QAction * mc_systrayMenu_bottomblank;

        //MC Systray Dialogs
            //Withdraw
                //As Cash
                int mc_withdraw_ascash_dialog_already_init;
                QDialog * mc_systrayMenu_withdraw_ascash_dialog;
                    //Grid layout
                    QGridLayout * mc_systrayMenu_withdraw_ascash_gridlayout;

                        //Withdraw (as cash) header label
                        QLabel * mc_systrayMenu_withdraw_ascash_header_label;

                        //Account Id (label)
                        QLabel * mc_systrayMenu_withdraw_ascash_accountid_label;

                        //Account Name (Dropdown box)
                        QComboBox * mc_systrayMenu_withdraw_ascash_account_dropdown;

                        //Amount (in integer for now)
                        QLineEdit * mc_systrayMenu_withdraw_ascash_amount_input;

                        //Activate withdraw button
                        QPushButton * mc_systrayMenu_withdraw_ascash_button;

                            //Withdraw as cash confirmation
                            int mc_withdraw_ascash_confirm_dialog_already_init;
                            QDialog * mc_systrayMenu_withdraw_ascash_confirm_dialog;

                                //Grid layout
                                QGridLayout * mc_systrayMenu_withdraw_ascash_confirm_gridlayout;

                                    //Label
                                    QLabel * mc_systrayMenu_withdraw_ascash_confirm_label;

                                    //Label (Amount)
                                    QLabel * mc_systrayMenu_withdraw_ascash_confirm_amount_label;

                                    //Backend (Amount)
                                    int withdraw_ascash_confirm_amount_int;

                                    //Confirm/Cancel horizontal layout
                                    QWidget * mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget;
                                    QHBoxLayout * mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout;

                                    //Cancel amount (button)
                                    QPushButton * mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel;

                                    //Confirm amount (button)
                                    QPushButton * mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm;


                //As Voucher
                int mc_withdraw_asvoucher_dialog_already_init;
                QDialog * mc_systrayMenu_withdraw_asvoucher_dialog;
                    //Grid layout
                    QGridLayout * mc_systrayMenu_withdraw_asvoucher_gridlayout;
                        //Withdraw (as voucher) header label
                        QLabel * mc_systrayMenu_withdraw_asvoucher_header_label;

                        //Account Id (label)
                        QLabel * mc_systrayMenu_withdraw_asvoucher_accountid_label;

                        //Account Name (Dropdown box)
                        QComboBox * mc_systrayMenu_withdraw_asvoucher_account_dropdown;

                        //Nym ID (input)
                            //Horitzontal Layout for Nym ID Input
                            QWidget * mc_systrayMenu_withdraw_asvoucher_nym_holder;
                            QHBoxLayout * mc_systrayMenu_withdraw_asvoucher_nym_hbox;

                                //Nym Id (type/paste input)
                                QLineEdit * mc_systrayMenu_withdraw_asvoucher_nym_input;

                                //Address book (button as Icon)
                                QIcon mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon;
                                QPushButton * mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn;

                                //QR Code Scanner (button as Icon)
                                    //TODO ^^

                        //Amount (in integer for now)
                        QLineEdit * mc_systrayMenu_withdraw_asvoucher_amount_input;

                        //Activate withdraw button
                        QPushButton * mc_systrayMenu_withdraw_asvoucher_button;

                            //Withdraw as Voucher confirmation
                            int mc_withdraw_asvoucher_confirm_dialog_already_init;
                            QDialog * mc_systrayMenu_withdraw_asvoucher_confirm_dialog;

                                //Grid layout
                                QGridLayout * mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout;

                                    //Label
                                    QLabel * mc_systrayMenu_withdraw_asvoucher_confirm_label;

                                    //Label (Amount)
                                    QLabel * mc_systrayMenu_withdraw_asvoucher_confirm_amount_label;

                                    //Backend (Amount)
                                    int withdraw_asvoucher_confirm_amount_int;

                                    //Confirm/cancel horizontal layout
                                    QWidget * mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget;
                                    QHBoxLayout * mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout;

                                        //Cancel amount (button)
                                        QPushButton * mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel;

                                        //Confirm amount (button)
                                        QPushButton * mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm;


    /**           **
     ** Functions **
     **           **/

        //Address Book Dialog
            //Show address book
            void mc_addressbook_show();
            void mc_addressbook_show(QString);

        //Menu Dialog
            //Withdraw
                //As Cash
                void mc_withdraw_ascash_dialog();

                //As Voucher
                void mc_withdraw_asvoucher_dialog();

        //QT to OT functions


private slots:
        //Systray Menu Slots
            //Shutdown
            void mc_shutdown_slot();

            //Withdraw
                //As Cash
                 void mc_withdraw_ascash_slot();
                 void mc_withdraw_ascash_confirm_amount_dialog_slot();

                 void mc_withdraw_ascash_account_dropdown_highlighted_slot(int);
                 void mc_withdraw_ascash_confirm_amount_slot();
                 void mc_withdraw_ascash_cancel_amount_slot();

                 //As Voucher
                 void mc_withdraw_asvoucher_slot();
                 void mc_withdraw_asvoucher_show_addressbook_slot();
                 void mc_withdraw_asvoucher_confirm_amount_dialog_slot();

                 void mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int);
                 void mc_withdraw_asvoucher_confirm_amount_slot();


};

#endif // MONEYCHANGER_H
