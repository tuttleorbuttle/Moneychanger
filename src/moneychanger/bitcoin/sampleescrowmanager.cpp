#include<QObject>
#include "Widgets/btcguitest.h"
#include "sampleescrowmanager.h"
#include "sampleescrowclient.h"
#include "sampleescrowserver.h"
#include "modules.h"
#include "utils.h"

SampleEscrowManager::SampleEscrowManager()
{
    this->escrowPool = EscrowPoolRef(NULL);
    this->client = SampleEscrowClientRef(NULL);
}

void SampleEscrowManager::OnSimulateEscrowServers()
{
    // simulate a new pool
    EscrowPoolRef escrowPool = EscrowPoolRef(new EscrowPool());
    Modules::poolManager->AddPool(escrowPool);

    // simulate servers in pool, each using its own instance of bitcoind/bitcoin-qt
    BitcoinServerRef rpcServer;
    for(int i = 1; i < 3; i++)
    {
        // admin1..3, port 19011, 19021, 19031
        rpcServer = BitcoinServerRef(new BitcoinServer("admin"+QString(i), "123", "http://127.0.0.1", 19001 + i * 10));
        escrowPool->AddEscrowServer(SampleEscrowServerRef(new SampleEscrowServer(rpcServer)));
    }
}

void SampleEscrowManager::OnInitializeEscrow(BtcGuiTest* btcGuiTest)
{
    // create a new client
    this->client = SampleEscrowClientRef(new SampleEscrowClient());

    // connect events to update GUI
    QObject::connect(client.data(), SIGNAL(SetMultiSigAddress(const std::string&)), btcGuiTest, SLOT(SetMultiSigAddress(const std::string&)));
    QObject::connect(client.data(), SIGNAL(SetTxIdDeposit(const std::string&)), btcGuiTest, SLOT(SetTxIdDeposit(const std::string&)));
    QObject::connect(client.data(), SIGNAL(SetConfirmationsDeposit(int)), btcGuiTest, SLOT(OnSetConfirmationsDeposit(int)));
    QObject::connect(client.data(), SIGNAL(SetStatusDeposit(SampleEscrowTransaction::SUCCESS)), btcGuiTest, SLOT(SetStatusDeposit(SampleEscrowTransaction::SUCCESS)));
    QObject::connect(client.data(), SIGNAL(SetWithdrawalAddress(const std::string&)), btcGuiTest, SLOT(SetWithdrawalAddress(const std::string&)));
    QObject::connect(client.data(), SIGNAL(SetTxIdWithdrawal(const std::string&)), btcGuiTest, SLOT(SetTxIdWithdrawal(const std::string&)));
    QObject::connect(client.data(), SIGNAL(SetConfirmationsWithdrawal(int)), btcGuiTest, SLOT(OnSetConfirmationsWithdrawal(int)));
    QObject::connect(client.data(), SIGNAL(SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS)), btcGuiTest, SLOT(SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS)));

    double amountToSend = btcGuiTest->GetAmountToSend();
    int64_t amountSatoshis = utils::CoinsToSatoshis(amountToSend);

    // instruct client to start sending bitcoin to pool
    client->StartDeposit(amountSatoshis, this->escrowPool);

    // update client until transaction is confirmed
    utils::SleepSimulator sleeper;
    while(!client->CheckDepositFinished())
    {
        sleeper.sleep(500);
    }

    // update servers until transaction is confirmed
    // in this example we already waited for the client so this should complete instantly
    foreach(SampleEscrowServerRef server, this->escrowPool->escrowServers)
    {
        while(!server->CheckIncomingTransaction())
        {
            sleeper.sleep(500);
        }

        if(server->transactionDeposit->status == SampleEscrowTransaction::Successfull)
        {
            // server received funds to escrow
        }
    }    
}

void SampleEscrowManager::OnRequestWithdrawal(BtcGuiTest *BtcGuiTest)
{
    if(client->transactionDeposit->status == SampleEscrowTransaction::Successfull)
    {
        // successfully deposited funds, so we can withdraw them now
        client->StartWithdrawal();
    }

    // update client until transaction is finished
    utils::SleepSimulator sleeper;
    while(!client->CheckWithdrawalFinished())
    {
        sleeper.sleep(500);
    }

    if(client->transactionWithdrawal->status == SampleEscrowTransaction::Successfull)
    {
        // successfully received funds from escrow
    }
}
