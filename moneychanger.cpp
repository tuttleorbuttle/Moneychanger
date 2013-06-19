#include "moneychanger.h"

#include "opentxs/OTAPI.h"
#include "opentxs/OT_ME.h"

/**
 ** Constructor & Destructor
 **/
Moneychanger::Moneychanger(QWidget *parent)
    : QWidget(parent)
{
    /**
     ** Init variables *
     **/

    ot_me = new OT_ME();

    //SQLite databases
    addressbook_db = QSqlDatabase::addDatabase("QSQLITE", "addressBook");
    addressbook_db.setDatabaseName("./db/mc_db");
    qDebug() << addressbook_db.lastError();
    bool db_opened = addressbook_db.open();
    qDebug() << "DB OPENED " << db_opened;

    /* *** *** ***
     * Init Memory Trackers (there may be other int below then just memory trackers but generally there will be mostly memory trackers below)
     * Allows the program to boot with a low footprint -- keeps start times low no matter the program complexity;
     * Memory will expand as the operator opens dialogs;
     * Also prevents HTTP requests from overloading or spamming the operators device by only allowing one window of that request;
     * *** *** ***/
        //Address Book
            mc_addressbook_already_init = 0;
            mc_addressbook_refreshing = 0;

        //Menu
            //Withdraw
                //As Cash
                mc_withdraw_ascash_dialog_already_init = 0;
                mc_withdraw_ascash_confirm_dialog_already_init = 0;

                //As Voucher
                mc_withdraw_asvoucher_dialog_already_init = 0;
                mc_withdraw_asvoucher_confirm_dialog_already_init = 0;

    //Init MC System Tray Icon
    mc_systrayIcon = new QSystemTrayIcon(this);
    mc_systrayIcon->setIcon(QIcon(":/icons/moneychanger"));

            //Init Icon resources (Loading resources/access Harddrive first; then send to GPU; This specific order will in theory prevent bottle necking between HDD/GPU)
            mc_systrayIcon_shutdown = QIcon(":/icons/quit");

            mc_systrayIcon_overview = QIcon(":/icons/overview");

            mc_systrayIcon_nym = QIcon(":/icons/nym");
            mc_systrayIcon_server = QIcon(":/icons/server");

            mc_systrayIcon_goldaccount = QIcon(":/icons/goldaccount");
            mc_systrayIcon_goldcashpurse = QIcon(":/icons/goldpurse");

            mc_systrayIcon_withdraw = QIcon(":/icons/withdraw");
            mc_systrayIcon_deposit = QIcon(":/icons/deposit");

            mc_systrayIcon_sendfunds = QIcon(":/icons/sendfunds");
            mc_systrayIcon_requestpayment = QIcon(":/icons/requestpayment");

            mc_systrayIcon_advanced = QIcon(":/icons/advanced");

    //MC System tray menu
    mc_systrayMenu = new QMenu(this);

        //Init Skeleton of system tray menu
            //App name
            mc_systrayMenu_headertext = new QAction("Moneychanger", 0);
            mc_systrayMenu_headertext->setDisabled(1);
            mc_systrayMenu->addAction(mc_systrayMenu_headertext);

            //Shutdown Moneychanger
            mc_systrayMenu_shutdown = new QAction(mc_systrayIcon_shutdown, "Quit", 0);
            mc_systrayMenu_shutdown->setIcon(mc_systrayIcon_shutdown);
            mc_systrayMenu->addAction(mc_systrayMenu_shutdown);
                //connection
                connect(mc_systrayMenu_shutdown, SIGNAL(triggered()), this, SLOT(mc_shutdown_slot()));


            //Blank
            mc_systrayMenu_aboveBlank = new QAction(" ", 0);
            mc_systrayMenu_aboveBlank->setDisabled(1);
            mc_systrayMenu->addAction(mc_systrayMenu_aboveBlank);

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

            //Withdraw
            mc_systrayMenu_withdraw = new QMenu("Withdraw", 0);
            mc_systrayMenu_withdraw->setIcon(mc_systrayIcon_withdraw);
            mc_systrayMenu->addMenu(mc_systrayMenu_withdraw);
                //(Withdraw) as Cash
                mc_systrayMenu_withdraw_ascash = new QAction("As Cash",0);
                mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_ascash);
                    //Connect Button with re-action
                    connect(mc_systrayMenu_withdraw_ascash, SIGNAL(triggered()), this, SLOT(mc_withdraw_ascash_slot()));

                //(Withrdaw) as Voucher
                mc_systrayMenu_withdraw_asvoucher = new QAction("As Voucher", 0);
                mc_systrayMenu_withdraw->addAction(mc_systrayMenu_withdraw_asvoucher);
                    //Connect Button with re-action
                    connect(mc_systrayMenu_withdraw_asvoucher, SIGNAL(triggered()), this, SLOT(mc_withdraw_asvoucher_slot()));


            //Deposit
            mc_systrayMenu_deposit = new QAction("Deposit", 0);
            mc_systrayMenu_deposit->setIcon(mc_systrayIcon_deposit);
            mc_systrayMenu->addAction(mc_systrayMenu_deposit);


            //Seperator
            mc_systrayMenu->addSeparator();

            //Send funds
            mc_systrayMenu_sendfunds = new QAction("Send Funds", 0);
            mc_systrayMenu_sendfunds->setIcon(mc_systrayIcon_sendfunds);
            mc_systrayMenu->addAction(mc_systrayMenu_sendfunds);

            //Request payment
            mc_systrayMenu_requestpayment = new QAction("Request Payment", 0);
            mc_systrayMenu_requestpayment->setIcon(mc_systrayIcon_requestpayment);
            mc_systrayMenu->addAction(mc_systrayMenu_requestpayment);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Advanced
            mc_systrayMenu_advanced = new QAction("Advanced", 0);
            mc_systrayMenu_advanced->setIcon(mc_systrayIcon_advanced);
            mc_systrayMenu->addAction(mc_systrayMenu_advanced);

            //Seperator
            mc_systrayMenu->addSeparator();

            //Blank
            mc_systrayMenu_bottomblank = new QAction(" ", 0);
            mc_systrayMenu_bottomblank->setDisabled(1);
            mc_systrayMenu->addAction(mc_systrayMenu_bottomblank);

        //Set Skeleton to systrayIcon object code
        mc_systrayIcon->setContextMenu(mc_systrayMenu);
}

Moneychanger::~Moneychanger()
{

}


/** ****** ****** ******  **
 ** Public Function/Calls **/
    //start
    void Moneychanger::bootTray(){
        //Show systray
        mc_systrayIcon->show();

        qDebug() << "BOOTING";

        //OTAPI_Wrap::checkUser("tBy5mL14qSQXCJK7Uz3WlTOKRP9M0JZksA3Eg7EnnQ1", "T1Q3wZWgeTUoaUvn9m1lzIK5tn5wITlzxzrGNI8qtaV", "T1Q3wZWgeTUoaUvn9m1lzIK5tn5wITlzxzrGNI8qtaV");
    }

/** ****** ****** ******   **
 ** Private Function/Calls **/

    /* **
     * Address Book Related Calls
     */
        /** Show Address Book **/
            //Dummy Call
            void Moneychanger::mc_addressbook_show(){
                //The caller dosen't wish to have the address book paste to anything (they just want to see/manage the address book), just call blank.
                mc_addressbook_show("");
            }


            //This will show the addressbook (and paste the selection accordingly if set)
            void Moneychanger::mc_addressbook_show(QString paste_selection_to){
                //Check if address book has been init before.
                if(mc_addressbook_already_init == 0){
                    //Init address book, then show
                    mc_addressbook_dialog = new QDialog(0);
                    mc_addressbook_dialog->setModal(1); //(Nice effect; Dims all windows except the address book and makes the address book on top upon showing
                    mc_addressbook_dialog->setWindowTitle("Address Book | Moneychanger");

                        //Set layout
                        mc_addressbook_gridlayout = new QGridLayout(0);
                        mc_addressbook_dialog->setLayout(mc_addressbook_gridlayout);

                            /* First Row in Address Book Grid */
                                //Label (Address Book)
                                mc_addressbook_label = new QLabel("<h3>Address Book</h3>");
                                mc_addressbook_label->setAlignment(Qt::AlignRight);
                                mc_addressbook_gridlayout->addWidget(mc_addressbook_label, 0,0, 1,2);

                            /* Second Row in Address Book Grid */
                                /** First column in address book grid (left side) **/
                                    //Table View (backend and visual init)
                                    mc_addressbook_tableview_itemmodel = new QStandardItemModel(0,3,0);
                                    mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Display Nym")));
                                    mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Nym ID")));
                                    mc_addressbook_tableview_itemmodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Backend DB ID")));
                                    //Connect tableviews' backend "dataChanged" signal to a re-action.
                                        connect(mc_addressbook_tableview_itemmodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(mc_addressbook_dataChanged_slot(QModelIndex,QModelIndex)));

                                    mc_addressbook_tableview = new QTableView(0);
                                    mc_addressbook_tableview->setSelectionMode(QAbstractItemView::SingleSelection);
                                    mc_addressbook_tableview->setModel(mc_addressbook_tableview_itemmodel);
                                    mc_addressbook_tableview->hideColumn(2);
                                    mc_addressbook_gridlayout->addWidget(mc_addressbook_tableview, 1,0, 1,1);


                                 /** Second column in address book grid (right side) **/
                                    //2 Buttons (Add/Remove)
                                    mc_addressbook_addremove_btngroup_widget = new QWidget(0);
                                    mc_addressbook_addremove_btngroup_holder = new QVBoxLayout(0);

                                    mc_addressbook_addremove_btngroup_widget->setLayout(mc_addressbook_addremove_btngroup_holder);
                                    mc_addressbook_gridlayout->addWidget(mc_addressbook_addremove_btngroup_widget, 1,1, 1,1, Qt::AlignTop);

                                        //Add button
                                        mc_addressbook_addremove_add_btn = new QPushButton("Add Contact", 0);
                                        mc_addressbook_addremove_add_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
                                        mc_addressbook_addremove_btngroup_holder->addWidget(mc_addressbook_addremove_add_btn, 0, Qt::AlignTop);
                                            //Connect the add contact button with a re-action
                                            connect(mc_addressbook_addremove_add_btn, SIGNAL(clicked()), this, SLOT(mc_addressbook_addblankrow_slot()));

                                        //Remove button
                                        mc_addressbook_addremove_remove_btn = new QPushButton("Remove Contact", 0);
                                        mc_addressbook_addremove_remove_btn->setStyleSheet("QPushButton{padding:0.5em;margin:0}");
                                        mc_addressbook_addremove_btngroup_holder->addWidget(mc_addressbook_addremove_remove_btn, 0, Qt::AlignTop);

                            /* Third row in Address Book Grid */
                                /** Spans 2 columns **/
                                //This "select" button will be shown if the address book was initiated with the intention of pasting the selection into a dialog/window
                                        mc_addressbook_select_nym_for_paste_btn = new QPushButton("Paste selected contact as Nym Id",0);
                                        mc_addressbook_select_nym_for_paste_btn->hide();
                                        mc_addressbook_gridlayout->addWidget(mc_addressbook_select_nym_for_paste_btn, 2,0, 1,2, Qt::AlignHCenter);
                                            //Connect the "select" button with a re-action
                                        connect(mc_addressbook_select_nym_for_paste_btn, SLOT(click()), SIGNAL(mc_addressbook_paste_selected_slot()));
                    //Show dialog
                    mc_addressbook_dialog->show();
                    mc_addressbook_dialog->activateWindow();

                    /** Flag already init **/
                    mc_addressbook_already_init = 1;

                }else{
                    //Address book already init, show it!
                    mc_addressbook_dialog->show();
                    mc_addressbook_dialog->activateWindow();
                }


                //Refresh Addressbook with listing
                    /** Flag Refreshing Address Book **/
                    mc_addressbook_refreshing = 1;

                    //remove all rows from the address book (so we can refresh any newly changed data)
                    mc_addressbook_tableview_itemmodel->removeRows(0, mc_addressbook_tableview_itemmodel->rowCount());

                    QSqlQuery mc_addressbook_query(addressbook_db);
                    mc_addressbook_query.exec(QString("SELECT `id`, `nym_display_name`, `nym_id` FROM `address_book`"));
                    qDebug() << "DB QUERY LAST ERROR: " << mc_addressbook_query.lastError();
                    //Add Rows of data to the backend of the table view (QStandardItemModel)
                    int row_index = 0;
                    while(mc_addressbook_query.next()){
                        //Extract data
                        QString addressbook_row_id = mc_addressbook_query.value(0).toString();
                        QString addressbook_row_nym_display_name = mc_addressbook_query.value(1).toString();
                        QString addressbook_row_nym_id = mc_addressbook_query.value(2).toString();

                        //Place extracted data into the table view
                        QStandardItem * col_one = new QStandardItem(addressbook_row_nym_display_name);

                        QStandardItem * col_two = new QStandardItem(addressbook_row_nym_id);

                        QStandardItem * col_three = new QStandardItem(addressbook_row_id);

                        mc_addressbook_tableview_itemmodel->setItem(row_index, 0, col_one);
                        mc_addressbook_tableview_itemmodel->setItem(row_index, 1, col_two);
                        mc_addressbook_tableview_itemmodel->setItem(row_index, 2, col_three);

                        //Increment index
                        row_index += 1;

                        //Clear address book variables
                        addressbook_row_id = "";
                        addressbook_row_nym_display_name = "";
                        addressbook_row_nym_id = "";
                    }

                    /** Un-Flag Refreshing Address Book **/
                    mc_addressbook_refreshing = 0;

                //Decide if the "Select and paste" button should be shown
                    if(paste_selection_to != ""){
                        mc_addressbook_select_nym_for_paste_btn->show();
                    }

                //Resize
                    mc_addressbook_dialog->resize(400, 300);
            }


    /* **
     * Menu Dialog Related Calls
     */
        /** Withdraw **/
            //As Cash
            void Moneychanger::mc_withdraw_ascash_dialog(){
                /** Call OT for all information we need for this dialog **/

                    QList<QVariant> account_list_id = QList<QVariant>();
                    QList<QVariant> account_list_name = QList<QVariant>();

                    //Get account(s) information
                    int32_t account_count = OTAPI_Wrap::GetAccountCount();

                    for(int a = 0; a < account_count; a++){
                            //Get OT Account ID
                            QString OT_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(a));

                            //Add to qlist
                            account_list_id.append(QVariant(OT_account_id));

                            //Get OT Account Name
                            QString OT_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OTAPI_Wrap::GetAccountWallet_ID(a)));

                            //Add to qlist
                            account_list_name.append(QVariant(OT_account_name));

                    }



                /** If the withdraw as cash dialog has already been init,
                 *  just show it, Other wise, init and show if this is the
                 *  first time.
                 **/
                if(mc_withdraw_ascash_dialog_already_init == 0){
                    //Init, then show
                        //Init
                        mc_systrayMenu_withdraw_ascash_dialog = new QDialog(0);
                            /** window properties **/
                                //Set window title
                                mc_systrayMenu_withdraw_ascash_dialog->setWindowTitle("Withdraw as Cash | Moneychanger");
                                mc_systrayMenu_withdraw_ascash_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);

                            /** layout and content **/
                                //Grid layout Input
                                mc_systrayMenu_withdraw_ascash_gridlayout = new QGridLayout(0);
                                mc_systrayMenu_withdraw_ascash_dialog->setLayout(mc_systrayMenu_withdraw_ascash_gridlayout);

                                //Withdraw As Cash (header label)
                                mc_systrayMenu_withdraw_ascash_header_label = new QLabel("<h3>Withdraw as Cash</h3>", 0);
                                mc_systrayMenu_withdraw_ascash_header_label->setAlignment(Qt::AlignRight);
                                mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_header_label, 0, 0, 1, 1);

                                //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
                                mc_systrayMenu_withdraw_ascash_accountid_label = new QLabel("", 0);
                                mc_systrayMenu_withdraw_ascash_accountid_label->setStyleSheet("QLabel{padding:0.5em;}");
                                mc_systrayMenu_withdraw_ascash_accountid_label->setAlignment(Qt::AlignHCenter);
                                mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_accountid_label, 1, 0, 1, 1);

                                //Account Dropdown (combobox)
                                mc_systrayMenu_withdraw_ascash_account_dropdown = new QComboBox(0);
                                mc_systrayMenu_withdraw_ascash_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
                                mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_account_dropdown, 2, 0, 1, 1);

                                    //Add items to account dropdown box
                                    for(int a = 0; a < account_count; a++){
                                        //Add to combobox
                                            //Get OT Account ID
                                            mc_systrayMenu_withdraw_ascash_account_dropdown->addItem(account_list_name.at(a).toString(), account_list_id.at(a).toString());
                                    }

                                    //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
                                    connect(mc_systrayMenu_withdraw_ascash_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(mc_withdraw_ascash_account_dropdown_highlighted_slot(int)));

                                //Amount Instructions
                                    //TODO ^^

                                //Amount Input
                                mc_systrayMenu_withdraw_ascash_amount_input = new QLineEdit("Amount as Integer", 0);
                                mc_systrayMenu_withdraw_ascash_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
                                mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_amount_input, 3, 0, 1, 1);

                                //Withdraw Button
                                mc_systrayMenu_withdraw_ascash_button = new QPushButton("Withdraw as Cash");
                                mc_systrayMenu_withdraw_ascash_button->setStyleSheet("QPushButton{padding:0.5em;}");
                                mc_systrayMenu_withdraw_ascash_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_button, 4, 0, 1, 1);
                                    //Connect button with re-action
                                    connect(mc_systrayMenu_withdraw_ascash_button, SIGNAL(pressed()), this, SLOT(mc_withdraw_ascash_confirm_amount_dialog_slot()));

                            /** Flag already init **/
                            mc_withdraw_ascash_dialog_already_init = 1;

                       //Show
                       mc_systrayMenu_withdraw_ascash_dialog->show();
                       mc_systrayMenu_withdraw_ascash_dialog->activateWindow();
                }else{
                    //Show
                    mc_systrayMenu_withdraw_ascash_dialog->show();
                    mc_systrayMenu_withdraw_ascash_dialog->activateWindow();
                }

                //Resize
                mc_systrayMenu_withdraw_ascash_dialog->resize(400, 120);
            }

        //As Voucher
            void Moneychanger::mc_withdraw_asvoucher_dialog(){
                /** Call OT for all information we need for this dialog **/

                    QList<QVariant> account_list_id = QList<QVariant>();
                    QList<QVariant> account_list_name = QList<QVariant>();

                    //Get account(s) information
                    int32_t account_count = OTAPI_Wrap::GetAccountCount();

                    for(int a = 0; a < account_count; a++){
                            //Get OT Account ID
                            QString OT_account_id = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_ID(a));

                            //Add to qlist
                            account_list_id.append(QVariant(OT_account_id));

                            //Get OT Account Name
                            QString OT_account_name = QString::fromStdString(OTAPI_Wrap::GetAccountWallet_Name(OTAPI_Wrap::GetAccountWallet_ID(a)));

                            //Add to qlist
                            account_list_name.append(QVariant(OT_account_name));

                    }


                    /** If the withdraw as voucher dialog has already been init,
                     *  just show it, Other wise, init and show if this is the
                     *  first time.
                     **/
                    if(mc_withdraw_asvoucher_dialog_already_init == 0){
                        //Init, then show
                            //Init
                            mc_systrayMenu_withdraw_asvoucher_dialog = new QDialog(0);
                                /** window properties **/
                                    //Set window title
                                    mc_systrayMenu_withdraw_asvoucher_dialog->setWindowTitle("Withdraw as Voucher | Moneychanger");
                                    mc_systrayMenu_withdraw_asvoucher_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);

                                /** layout and content **/
                                    //Grid layout Input
                                    mc_systrayMenu_withdraw_asvoucher_gridlayout = new QGridLayout(0);
                                    mc_systrayMenu_withdraw_asvoucher_dialog->setLayout(mc_systrayMenu_withdraw_asvoucher_gridlayout);

                                    //Label (withdraw as voucher)
                                    mc_systrayMenu_withdraw_asvoucher_header_label = new QLabel("<h3>Withdraw as Voucher</h3>", 0);
                                    mc_systrayMenu_withdraw_asvoucher_header_label->setAlignment(Qt::AlignRight);
                                    mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_header_label, 0,0, 1,1);

                                    //Account ID (label) Note: Value is set when the dropdown box is selected and/or highlighted
                                    mc_systrayMenu_withdraw_asvoucher_accountid_label = new QLabel("", 0);
                                    mc_systrayMenu_withdraw_asvoucher_accountid_label->setAlignment(Qt::AlignHCenter);
                                    mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_accountid_label, 1,0, 1,1);

                                    //Account Dropdown (combobox)
                                    mc_systrayMenu_withdraw_asvoucher_account_dropdown = new QComboBox(0);

                                    mc_systrayMenu_withdraw_asvoucher_account_dropdown->setStyleSheet("QComboBox{padding:0.5em;}");
                                    mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_account_dropdown, 2,0, 1,1);

                                        //Add items to account dropdown box
                                        for(int a = 0; a < account_count; a++){
                                            //Add to combobox
                                                //Get OT Account ID
                                                mc_systrayMenu_withdraw_asvoucher_account_dropdown->addItem(account_list_name.at(a).toString(), account_list_id.at(a).toString());
                                        }

                                        //Make connection to "hovering over items" to showing their IDs above the combobox (for user clarity and backend id indexing)
                                        connect(mc_systrayMenu_withdraw_asvoucher_account_dropdown, SIGNAL(highlighted(int)), this, SLOT(mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int)));

                                    //To Nym ID
                                        //Horizontal Box (to hold Nym Id input/Address Box Icon/QR Code Scanner Icon)
                                        mc_systrayMenu_withdraw_asvoucher_nym_holder = new QWidget(0);
                                        mc_systrayMenu_withdraw_asvoucher_nym_hbox = new QHBoxLayout(0);
                                        mc_systrayMenu_withdraw_asvoucher_nym_hbox->setMargin(0);
                                        mc_systrayMenu_withdraw_asvoucher_nym_holder->setLayout(mc_systrayMenu_withdraw_asvoucher_nym_hbox);
                                        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_holder, 3,0, 1,1);

                                            //Nym ID (Paste input)
                                            mc_systrayMenu_withdraw_asvoucher_nym_input = new QLineEdit("Recipient Nym Id", 0);
                                            mc_systrayMenu_withdraw_asvoucher_nym_input->setStyleSheet("QLineEdit{padding:0.5em;}");
                                            mc_systrayMenu_withdraw_asvoucher_nym_hbox->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_input);

                                            //Address Book (button)
                                            mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon = QIcon(":/icons/addressbook");
                                            mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn = new QPushButton(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_icon, "", 0);
                                            mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn->setStyleSheet("QPushButton{padding:0.5em;}");
                                            mc_systrayMenu_withdraw_asvoucher_nym_hbox->addWidget(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn);
                                                //Connect Address book button with a re-action
                                                connect(mc_systrayMenu_withdraw_asvoucher_nym_addressbook_btn, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_show_addressbook_slot()));

                                            //QR Code scanner (button)
                                                //TO DO^^


                                    //Amount input
                                        mc_systrayMenu_withdraw_asvoucher_amount_input = new QLineEdit("Amount as Integer", 0);
                                        mc_systrayMenu_withdraw_asvoucher_amount_input->setStyleSheet("QLineEdit{padding:0.5em;}");
                                        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_amount_input, 4,0, 1,1);


                                    //Withdraw Button
                                        mc_systrayMenu_withdraw_asvoucher_button = new QPushButton("Withdraw as Voucher");
                                        mc_systrayMenu_withdraw_asvoucher_button->setStyleSheet("QPushButton{padding:0.5em;}");
                                        mc_systrayMenu_withdraw_asvoucher_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_button, 5,0, 1,1);
                                            //Connect button with re-action
                                            connect(mc_systrayMenu_withdraw_asvoucher_button, SIGNAL(pressed()), this, SLOT(mc_withdraw_asvoucher_confirm_amount_dialog_slot()));

                                /** Flag as init **/
                                    mc_withdraw_asvoucher_dialog_already_init = 1;
                        //Show
                            mc_systrayMenu_withdraw_asvoucher_dialog->show();
                            mc_systrayMenu_withdraw_asvoucher_dialog->setFocus();
                    }else{
                       //This dialog has already been init, just show it
                        mc_systrayMenu_withdraw_asvoucher_dialog->show();
                        mc_systrayMenu_withdraw_asvoucher_dialog->setFocus();
                    }

                    //Resize
                    mc_systrayMenu_withdraw_asvoucher_dialog->resize(400, 120);
            }



/** ****** ****** ****** **
 ** Private Slots        **/
    /* Address Book Slots */
        //When "add contact" is clicked, Add a blank row to the address book so the user can edit it and save their changes.
        void Moneychanger::mc_addressbook_addblankrow_slot(){
            //Get total rows from the table view
            int total_rows_in_table = 0;
            total_rows_in_table = mc_addressbook_tableview_itemmodel->rowCount();

            //Insert blank row
            int blank_row_target_index = blank_row_target_index = total_rows_in_table;
            QStandardItem * blank_row_item = new QStandardItem("");
            mc_addressbook_tableview_itemmodel->setItem(blank_row_target_index,0,blank_row_item);


        }

        //When a row is edited/updated this will be triggered to sync the changes to the database.
        void Moneychanger::mc_addressbook_dataChanged_slot(QModelIndex topLeft, QModelIndex bottomRight){

            //This slot will ignore everything while the address book is refreshing; If not refreshing, go about regular logic.
            if(mc_addressbook_refreshing == 0){
                //Extract DB ID associated with the edited tableview row.
                int target_index_row = topLeft.row();

                QModelIndex index = mc_addressbook_tableview_itemmodel->index(target_index_row, 2, QModelIndex());
                qDebug() << mc_addressbook_tableview_itemmodel->data(index);

                /*
                //Extract new data
                QString new_data = QVariant(mc_addressbook_tableview_itemmodel->data(topLeft)).toString();

                //Save/Update newly updated data to the address book.
                QSqlQuery mc_addressbook_query(addressbook_db);
                mc_addressbook_query.exec(QString("UPDATE"));
                qDebug() << "DB QUERY LAST ERROR: " << mc_addressbook_query.lastError();
*/
            }
        }

        //"paste selected" button then we will detect here where to paste and what to paste.
        void Moneychanger::mc_addressbook_paste_selected_slot(){

        }


    /* Systray menu slots */

        //Shutdown
        void Moneychanger::mc_shutdown_slot(){
            //Close qt app (no need to deinit anything as of the time of this comment)
            //TO DO: Check if the OT queue caller is still proccessing calls.... Then quit the app. (Also tell user that the OT is still calling other wise they might think it froze during OT calls)
            qApp->quit();
        }


        //Withdraw Slots
            /*
             ** AS CASH SLOTS()
             */


            /** Open the dialog window **/
            void Moneychanger::mc_withdraw_ascash_slot(){
                //The operator has requested to open the dialog to withdraw as cash.
                mc_withdraw_ascash_dialog();
            }

            /**
             ** Button from dialog window has been activated;
             ** Confirm amount;
             ** Upon confirmation call OT withdraw_cash()
             **/
            void Moneychanger::mc_withdraw_ascash_confirm_amount_dialog_slot(){
                //Close the (withdraw as cash) dialog
                mc_systrayMenu_withdraw_ascash_dialog->hide();

                //First confirm this is the correct amount before calling OT
                    //Has this dialog already been init before?
                    if(mc_withdraw_ascash_confirm_dialog_already_init == 0){
                        //First time init
                        mc_systrayMenu_withdraw_ascash_confirm_dialog = new QDialog(0);

                            //Attach layout
                            mc_systrayMenu_withdraw_ascash_confirm_gridlayout = new QGridLayout(0);
                            mc_systrayMenu_withdraw_ascash_confirm_dialog->setLayout(mc_systrayMenu_withdraw_ascash_confirm_gridlayout);

                                //Ask the operator to confirm the amount requested
                                mc_systrayMenu_withdraw_ascash_confirm_label = new QLabel("Please confirm the amount to withdraw.");
                                mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_label, 0,0, 1,1);

                                //Label (Amount)
                                QString confirm_amount_string = "<b>"+mc_systrayMenu_withdraw_ascash_amount_input->text()+"</b>";
                                mc_systrayMenu_withdraw_ascash_confirm_amount_label = new QLabel(confirm_amount_string);
                                mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_label, 1,0, 1,1);

                                //Set Withdraw as cash amount int
                                QString confirm_amount_string_int = mc_systrayMenu_withdraw_ascash_amount_input->text();

                                withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();

                                //Spacer

                                //Horizontal Box
                                mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget = new QWidget(0);
                                mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
                                mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget->setLayout(mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout);
                                mc_systrayMenu_withdraw_ascash_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);

                                    //Button (Cancel amount)
                                    mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel = new QPushButton("Cancel Amount", 0);
                                    mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel);
                                        //Connect the cancel button with a re-action
                                        connect(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(mc_withdraw_ascash_cancel_amount_slot()));

                                    //Button (Confirm amount)
                                    mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm = new QPushButton("Confirm Amount", 0);
                                    mc_systrayMenu_withdraw_ascash_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm);
                                        //Connect the Confirm button with a re-action
                                        connect(mc_systrayMenu_withdraw_ascash_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(mc_withdraw_ascash_confirm_amount_slot()));

                       /** Flag already init **/
                       mc_withdraw_ascash_confirm_dialog_already_init = 1;

                       //Show
                       mc_systrayMenu_withdraw_ascash_confirm_dialog->show();


                    }else{
                        //Not first time init, just show the dialog.

                        //Set Withdraw as cash amount int
                        QString confirm_amount_string_int = mc_systrayMenu_withdraw_ascash_amount_input->text();

                        withdraw_ascash_confirm_amount_int = confirm_amount_string_int.toInt();

                        mc_systrayMenu_withdraw_ascash_confirm_dialog->show();
                    }

            }


            /**
             ** This will display the account id that the user has selected (for convience also for backend id tracking)
             **/
            void Moneychanger::mc_withdraw_ascash_account_dropdown_highlighted_slot(int dropdown_index){
                //Change Account ID label to the highlighted(bymouse) dropdown index.
                mc_systrayMenu_withdraw_ascash_accountid_label->setText(mc_systrayMenu_withdraw_ascash_account_dropdown->itemData(dropdown_index).toString());
            }

            /**
             ** This will be triggered when the user click the "confirm amount" button from the withdraw/confirm dialog
             **/

            void Moneychanger::mc_withdraw_ascash_confirm_amount_slot(){
                //Close the dialog/window
                mc_systrayMenu_withdraw_ascash_confirm_dialog->hide();

                //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
                QString selected_account_id = mc_systrayMenu_withdraw_ascash_account_dropdown->itemData(mc_systrayMenu_withdraw_ascash_account_dropdown->currentIndex()).toString();
                std::string selected_account_id_string = selected_account_id.toStdString();

                QString amount_to_withdraw_string = mc_systrayMenu_withdraw_ascash_amount_input->text();
                int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();

                //Get Nym ID
                std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);

                //Get Server ID
                std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);

                //Call OTAPI Withdraw cash
                std::string withdraw_cash_response = ot_me->withdraw_cash(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);
                //qDebug() << QString::fromStdString(withdraw_cash_response);


            }

            /**
             ** This will be triggered when the user click the "cancel amount" button from the withdraw/confirm dialog
             **/
            void Moneychanger::mc_withdraw_ascash_cancel_amount_slot(){
                //Close the dialog/window
                mc_systrayMenu_withdraw_ascash_confirm_dialog->hide();
            }


            /*
             ** AS VOUCHER SLOTS()
             */

            /** Open a new dialog window **/
            void Moneychanger::mc_withdraw_asvoucher_slot(){
                //The operator has requested to open the dialog to withdraw as cash.
                mc_withdraw_asvoucher_dialog();
            }

            /**
             ** This will be triggered when the user hovers over a dropdown (combobox) item in the withdraw as voucher account selection
             **/
            void Moneychanger::mc_withdraw_asvoucher_account_dropdown_highlighted_slot(int dropdown_index){
                //Change Account ID label to the highlighted(bymouse) dropdown index.
                mc_systrayMenu_withdraw_asvoucher_accountid_label->setText(mc_systrayMenu_withdraw_asvoucher_account_dropdown->itemData(dropdown_index).toString());
            }


            /**
             ** Button from dialog window has been activated;
             ** Confirm amount;
             ** Upon confirmation call OT withdraw_voucher()
             **/
            void Moneychanger::mc_withdraw_asvoucher_confirm_amount_dialog_slot(){
                //Close the (withdraw as voucher) dialog
                mc_systrayMenu_withdraw_asvoucher_dialog->hide();

                //First confirm this is the correct amount before calling OT
                    //Has this dialog already been init before?
                    if(mc_withdraw_asvoucher_confirm_dialog_already_init == 0){
                        //First time init
                        mc_systrayMenu_withdraw_asvoucher_confirm_dialog = new QDialog(0);
                            //Set window properties
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setWindowTitle("Confirm Amount | Withdraw as Voucher | Moneychanger");
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setWindowFlags(Qt::WindowStaysOnTopHint);

                            //Attach layout
                            mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout = new QGridLayout(0);
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setLayout(mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout);


                            //Ask the operator to confirm the amount
                            //Ask Label
                            mc_systrayMenu_withdraw_asvoucher_confirm_label = new QLabel("<h3>Please confirm the amount to withdraw.</h3>", 0);
                            mc_systrayMenu_withdraw_asvoucher_confirm_label->setAlignment(Qt::AlignRight);
                            mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_label, 0,0, 1,1);

                            //Label (Amount)
                            QString confirm_amount_string = "<b>"+mc_systrayMenu_withdraw_asvoucher_amount_input->text()+"</b>";
                            mc_systrayMenu_withdraw_asvoucher_confirm_amount_label = new QLabel(confirm_amount_string);
                            mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_label, 1,0, 1,1);


                            //Set Withdraw as voucher amount int
                            QString confirm_amount_string_int = mc_systrayMenu_withdraw_asvoucher_amount_input->text();
                            withdraw_asvoucher_confirm_amount_int = confirm_amount_string_int.toInt();


                                //Spacer

                                //Horizontal Box
                                mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget = new QWidget(0);
                                mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout = new QHBoxLayout(0);
                                mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget->setLayout(mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout);
                                mc_systrayMenu_withdraw_asvoucher_confirm_gridlayout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_widget, 3, 0, 1, 1);


                                    //Button (Cancel amount)
                                    mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel = new QPushButton("Cancel Amount", 0);
                                    mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel);
                                        //Connect the cancel button with a re-action
                                        //connect(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_cancel, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_cancel_amount_slot()));

                                    //Button (Confirm amount)
                                    mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm = new QPushButton("Confirm Amount", 0);
                                    mc_systrayMenu_withdraw_asvoucher_confirm_amount_confirm_cancel_layout->addWidget(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm);
                                        //Connect the Confirm button with a re-action
                                        //connect(mc_systrayMenu_withdraw_asvoucher_confirm_amount_btn_confirm, SIGNAL(clicked()), this, SLOT(mc_withdraw_asvoucher_confirm_amount_slot()));


                            /** Flag already init **/
                            mc_withdraw_asvoucher_confirm_dialog_already_init = 1;

                        //Show
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->show();
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setFocus();
                    }else{
                        //Show
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->show();
                            mc_systrayMenu_withdraw_asvoucher_confirm_dialog->setFocus();
                    }
            }

            //This is activated when the user clicks "Confirm amount"
            void Moneychanger::mc_withdraw_asvoucher_confirm_amount_slot(){
                //Close the dialog/window
                mc_systrayMenu_withdraw_asvoucher_confirm_dialog->hide();

                //Collect require information to call the OT_ME::withdraw_cash(?,?,?) function
                QString selected_account_id = mc_systrayMenu_withdraw_asvoucher_account_dropdown->itemData(mc_systrayMenu_withdraw_asvoucher_account_dropdown->currentIndex()).toString();
                std::string selected_account_id_string = selected_account_id.toStdString();

                QString amount_to_withdraw_string = mc_systrayMenu_withdraw_asvoucher_amount_input->text();
                int64_t amount_to_withdraw_int = amount_to_withdraw_string.toInt();

                //Get Nym ID
                std::string nym_id = OTAPI_Wrap::GetAccountWallet_NymID(selected_account_id_string);

                //Get Server ID
                std::string selected_server_id_string = OTAPI_Wrap::GetAccountWallet_ServerID(selected_account_id_string);

                //Call OTAPI Withdraw voucher
                //std::string withdraw_voucher_response = ot_me->withdraw_voucher(selected_server_id_string, nym_id, selected_account_id_string, amount_to_withdraw_int);
                //qDebug() << QString::fromStdString(withdraw_cash_response);

            }


            //This will show the address book, the opened address book will be set to paste in recipient nym ids if/when selecting a nymid in the addressbook.
            void Moneychanger::mc_withdraw_asvoucher_show_addressbook_slot(){
                //Show address book
                mc_addressbook_show("withdraw_as_voucher");
            }
