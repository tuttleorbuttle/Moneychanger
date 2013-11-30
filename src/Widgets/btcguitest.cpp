#include "btcguitest.h"
#include "ui_btcguitest.h"
#include "modules.h"
#include "OTLog.h"

btcguitest::btcguitest(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::btcguitest)
{
    ui->setupUi(this);
}

btcguitest::~btcguitest()
{
    delete ui;
}

void btcguitest::on_testButton_clicked()
{
    if(!Modules::btcInterface->TestBtcJson())
        OTLog::vOutput(0, "Error testing bitcoin integration. Maybe test environment is not set up.\n");
    else
        OTLog::vOutput(0, "Bitcoin integration successfully tested.\n");

    if(!Modules::btcInterface->TestBtcJsonEscrowTwoOfTwo())
        OTLog::vOutput(0, "Error testing bitcoin escrow functions. Maybe test environment is not set up.\n");
    else
        OTLog::vOutput(0, "Bitcoin escrow integration sucessfully tested.\n");
}
