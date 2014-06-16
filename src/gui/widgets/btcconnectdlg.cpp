#include "btcconnectdlg.hpp"
#include "ui_btcconnectdlg.h"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>

BtcConnectDlg::BtcConnectDlg(QWidget *parent) :
    QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint),
    ui(new Ui::BtcConnectDlg)
{
    this->ui->setupUi(this);

    this->rpcServer = BitcoinServerPtr();

    if(Modules::connectionManager != NULL && Modules::connectionManager.data() != this)
    {
        this->hide();
        Modules::connectionManager->show();
    }
    else
    {
        QString url = "http://" + this->ui->editHost->text();
        int port = this->ui->editPort->text().toInt();
        QString username = this->ui->editUser->text();
        QString password = this->ui->editPass->text();

        this->rpcServer = BitcoinServerPtr(new BitcoinServer(username.toStdString(), password.toStdString(), url.toStdString(), port));
    }
}

BtcConnectDlg::~BtcConnectDlg()
{
    delete this->ui;
}

void BtcConnectDlg::show()
{
    if(Modules::connectionManager != NULL && Modules::connectionManager.data() != this)
    {
        QWidget::hide();
        Modules::connectionManager->show();
    }
    else
    {
        QWidget::show();
    }
}

void BtcConnectDlg::on_buttonConnect_clicked()
{
    QString url = "http://" + this->ui->editHost->text();
    int port = this->ui->editPort->text().toInt();
    QString username = this->ui->editUser->text();
    QString password = this->ui->editPass->text();

    this->rpcServer = BitcoinServerPtr(new BitcoinServer(username.toStdString(), password.toStdString(), url.toStdString(), port));
    if(Modules::btcModules->btcRpc->ConnectToBitcoin(this->rpcServer))
        this->ui->labelStatus->setText("Connected");
    else
        this->ui->labelStatus->setText("Failed to connect");
}

void BtcConnectDlg::on_buttonDisconnect_clicked()
{
    this->rpcServer = BitcoinServerPtr();
    Modules::btcModules->btcRpc->ConnectToBitcoin(this->rpcServer);
    this->ui->labelStatus->setText("Disconnected");
}
