#include "sampleescrowclient.h"
#include "modules.h"
#include "sampleescrowserver.h"
#include <OTLog.h>


SampleEscrowClient::SampleEscrowClient(QObject* parent)
    :QObject(parent)
{
    this->rpcServer = BitcoinServerRef(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));

    this->minSignatures = 0;    // will be set later by server pool

    this->minConfirms = 1;      // wait for one confirmation

    this->targetPool = EscrowPoolRef(NULL);
    this->pubKeyList = std::list<std::string>();

    this->transactionDeposit = SampleEscrowTransactionRef(NULL);
    this->transactionWithdrawal = SampleEscrowTransactionRef(NULL);   
}

SampleEscrowClient::~SampleEscrowClient()
{
    this->rpcServer.clear();
    this->targetPool.clear();
    this->pubKeyList.clear();
    this->transactionDeposit.clear();
    this->transactionWithdrawal.clear();
}

void SampleEscrowClient::StartDeposit(int64_t amountToSend, EscrowPoolRef targetPool)
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    this->targetPool = targetPool;

    this->transactionDeposit.reset(new SampleEscrowTransaction(amountToSend));

    // ask the servers for an address to send money to
    // assuming no BIP32 and assuming servers don't constantly monitor the blockchain for incoming transactions
    // in reality this call would have to be forwarded through the net
    foreach(SampleEscrowServerRef server, this->targetPool->escrowServers)
    {
        server->OnRequestEscrowDeposit(this);
    }
}

void SampleEscrowClient::OnReceivePubKey(const std::string &publicKey, int minSignatures)
{
    if(publicKey == "")
        return;

    this->pubKeyList.push_back(publicKey);         // add server's key to keylist
    this->pubKeyList.sort();                    // somehow arrange all keys in the same order.

    if(this->minSignatures == 0)                // set number of required signatures
        this->minSignatures = minSignatures;
    if(this->minSignatures != minSignatures)    // see if all servers agree
        return;     // OMG HACKS!!!11

    // if we received all public keys we can start sending the bitcoins
    if(this->pubKeyList.size() == this->targetPool->escrowServers.size())
        SendToEscrow();
}

void SampleEscrowClient::SendToEscrow()
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    // create the multi-sig address of the server pool
    this->transactionDeposit->targetAddr = Modules::mtBitcoin->GetMultiSigAddress(this->minSignatures, this->pubKeyList);

    // set multi-sig address in GUI
    emit SetMultiSigAddress(this->transactionDeposit->targetAddr);

    this->transactionDeposit->SendToTarget();

    // set txid in GUI
    emit SetTxIdDeposit(this->transactionDeposit->txId);

    // notify server of incoming transaction
    foreach(SampleEscrowServerRef server, this->targetPool->escrowServers)
    {
        server->OnIncomingDeposit(this->transactionDeposit->txId);
    }
}

bool SampleEscrowClient::CheckDepositFinished()
{
    bool finished = CheckTransactionFinished(this->transactionDeposit);

    // set confirmations in GUI
    emit SetConfirmationsDeposit(this->transactionDeposit->confirmations);
    emit SetStatusDeposit(this->transactionDeposit->status);

    return finished;
}

void SampleEscrowClient::StartWithdrawal()
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    // create new transaction object
    this->transactionWithdrawal.reset(new SampleEscrowTransaction(this->transactionDeposit->amountToSend));

    // set the txId from which the funds should be released
    this->transactionWithdrawal->sourceTxId = this->transactionDeposit->txId;

    // set the address to which the funds should be sent
    this->transactionWithdrawal->targetAddr = Modules::mtBitcoin->GetNewAddress("ReceivedFromPool");

    // set address in GUI
    emit SetWithdrawalAddress(this->transactionWithdrawal->targetAddr);

    foreach(SampleEscrowServerRef server, this->targetPool->escrowServers)
    {
        server->OnRequestEscrowWithdrawal(this);
    }
}

void SampleEscrowClient::OnReceiveSignedTx(const std::string& signedTx)
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);
    
    // add the transaction's signature
    if(this->transactionWithdrawal->AddWithdrawalTransaction(signedTx))
    {
        // if enough signatures were collected, broadcast the transaction immediately
        // adding more signatures would do nothing but increase fees
        this->transactionWithdrawal->SendWithdrawalTransaction();

        // set txid in GUI
        emit SetTxIdWithdrawal(this->transactionWithdrawal->txId);
    }
}

bool SampleEscrowClient::CheckWithdrawalFinished()
{  
    bool finished = CheckTransactionFinished(this->transactionWithdrawal);

    // set confirmations in GUI
    emit SetConfirmationsWithdrawal(this->transactionWithdrawal->confirmations);
    emit SetStatusWithdrawal(this->transactionWithdrawal->status);

    return finished;
}

bool SampleEscrowClient::CheckTransactionFinished(SampleEscrowTransactionRef transaction)
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    transaction->CheckTransaction(this->minConfirms);

    if(transaction->status == SampleEscrowTransaction::Pending)
        return false;   // if transaction is pending, return false

    return true;        // if transaction is successfull or failed, return true
}








