#include "btctransactionmanager.hpp"
#include "ui_btctransactionmanager.h"
#include "btctransactioninfo.hpp"

#include <core/modules.hpp>
#include <bitcoin-api/btcmodules.hpp>
#include <bitcoin/sampleescrowclient.hpp>
#include <bitcoin/poolmanager.hpp>

#include <QTimer>

BtcTransactionInfo* infoWindow = NULL;

BtcTransactionManager::BtcTransactionManager(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::BtcTransactionManager)
{
    this->ui->setupUi(this);

    this->updateTimer = QTimerPtr(new QTimer(parent));
    updateTimer->setInterval(1000);
    updateTimer->start();
    connect(updateTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));
}

BtcTransactionManager::~BtcTransactionManager()
{
    delete ui;
}

int refreshTransactionsCount = 1;
int refreshAllCount = 1;
void BtcTransactionManager::Update()
{
    std::string poolName = Modules::poolManager->selectedPool;
    SampleEscrowClientPtr client = Modules::sampleEscrowClient;

    if(refreshTransactionsCount-- == 0)
    {
        refreshTransactionsCount = 15;
        client->CheckPoolTransactions(Modules::poolManager->GetPoolByName(poolName));
    }

    if(static_cast<uint32_t>(this->ui->tableTxPool->rowCount()) < client->poolTxCountMap[poolName])
    {
            RefreshPoolTransactions();
    }
    else if(refreshAllCount-- == 0)
    {
        refreshAllCount = 15;
        RefreshBitcoinTransactions();
        RefreshPoolTransactions(true);
    }
}

void BtcTransactionManager::on_buttonRefresh_clicked()
{
    if(!Modules::poolManager->selectedPool.empty())
        Modules::sampleEscrowClient->CheckPoolTransactions(Modules::poolManager->GetPoolByName(Modules::poolManager->selectedPool));

    RefreshBitcoinTransactions();
    RefreshPoolTransactions(true);
}

void BtcTransactionManager::RefreshBitcoinTransactions()
{
    // clear rows
    this->ui->tableTxBtc->setRowCount(0);

    // how many transactions to fetch
    int32_t txCount = this->ui->editTxCount->text().toInt();

    // get transactions
    BtcTransactions transactions = Modules::btcModules->btcJson->ListTransactions("*", txCount);
    foreach(BtcTransactionPtr tx, transactions)
    {
        int row = 0;    // insert at top
        int column = 0;
        this->ui->tableTxBtc->insertRow(row);
        QTableWidgetItem* item;
        if(tx->Details.size() <= 0)      // listtransactions returns one item in Details[]
        {
            item = new QTableWidgetItem("error");
            this->ui->tableTxBtc->setItem(row, column++, item);
            continue;
        }

        // type
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->category));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // confirmations
        item = new QTableWidgetItem(QString::number(tx->Confirmations));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // destination
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->account));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // amount
        item = new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(tx->Amount)));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // address
        item = new QTableWidgetItem(QString::fromStdString(tx->Details[0]->address));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // txid
        item = new QTableWidgetItem(QString::fromStdString(tx->TxId));
        this->ui->tableTxBtc->setItem(row, column++, item);

        // involves watchonly
        item = new QTableWidgetItem(QString(tx->Details[0]->involvesWatchonly ? "true" : "false"));
        this->ui->tableTxBtc->setItem(row, column++, item);
    }
}

void BtcTransactionManager::RefreshPoolTransactions(bool refreshAll)
{
    std::string poolName = Modules::poolManager->selectedPool;
    SampleEscrowClientPtr client = Modules::sampleEscrowClient;

    if(poolName.empty())
        return;

    SampleEscrowTransactions::iterator txIter;

    if(refreshAll)
    {
        for(txIter = client->poolTxMap[poolName].begin(); txIter != client->poolTxMap[poolName].end(); txIter++)
        {
            (*txIter)->CheckTransaction(BtcHelper::WaitForConfirms);
        }
    }

    this->ui->tableTxPool->setRowCount(0);

    for(SampleEscrowClient::PoolTxMap::iterator i = client->poolTxMap.begin(); i != client->poolTxMap.end(); i++)
    {
        for(txIter = i->second.begin(); txIter != i->second.end(); txIter++)
        {
            SampleEscrowTransactionPtr tx = (*txIter);

            int column = 0;
            this->ui->tableTxPool->insertRow(0);

            QString status;
            switch (tx->status)
            {
            case SampleEscrowTransaction::Successfull:
                status = "Successful";
                break;
            case SampleEscrowTransaction::Pending:
                status = "Pending";
                break;
            case SampleEscrowTransaction::NotStarted:
                status = "Not started";
            case SampleEscrowTransaction::Failed:
                status = "Failed";
                break;
            }

            // status
            this->ui->tableTxPool->setItem(0, column++, new QTableWidgetItem(status));

            // type
            QString type = tx->type == SampleEscrowTransaction::Deposit ? "Deposit" : "Release";
            this->ui->tableTxPool->setItem(0, column++, new QTableWidgetItem(type));

            // amount
            this->ui->tableTxPool->setItem(0, column++, new QTableWidgetItem(QString::number(BtcHelper::SatoshisToCoins(tx->amountToSend))));

            // pool
            this->ui->tableTxPool->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(i->first)));

            // txid
            this->ui->tableTxPool->setItem(0, column++, new QTableWidgetItem(QString::fromStdString(tx->txId)));
        }
    }
}

void BtcTransactionManager::on_buttonSearchTx_clicked()
{
    QString txId = this->ui->editSearchTx->text();
    if(txId.isEmpty())
        return;

    BtcTransactionPtr txInfo = Modules::btcModules->mtBitcoin->GetTransaction(txId.toStdString());
    BtcRawTransactionPtr txRawInfo = Modules::btcModules->btcJson->DecodeRawTransaction(txInfo->Hex);

    if(infoWindow == NULL)
        infoWindow = new BtcTransactionInfo();
    infoWindow->show();
    infoWindow->Initialize(txInfo, txRawInfo);
}

void BtcTransactionManager::on_tableTxBtc_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    // get txid from 6th column:
    QTableWidgetItem* txIdItem = this->ui->tableTxBtc->item(currentRow, 5);
    if(txIdItem == NULL)
        return;

    QString txId = txIdItem->text();
    this->ui->editSearchTx->setText(txId);
}

void BtcTransactionManager::on_tableTxPool_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    // get txid from 6th column:
    QTableWidgetItem* txIdItem = this->ui->tableTxPool->item(currentRow, 4);
    if(txIdItem == NULL)
        return;

    QString txId = txIdItem->text();
    this->ui->editSearchTx->setText(txId);
}