#include "btcguitestp2p.h"
#include "ui_btcguitestp2p.h"

BtcGuiTestp2p::BtcGuiTestp2p(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BtcGuiTestp2p)
{
    ui->setupUi(this);
}

BtcGuiTestp2p::~BtcGuiTestp2p()
{
    delete ui;
}

void BtcGuiTestp2p::on_simulatePoolButton_clicked()
{

}
