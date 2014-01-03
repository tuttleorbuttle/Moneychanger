#include "modules.h"
#include "sampleescrowserver.h"
#include "sampleescrowclient.h"
#include "sampleescrowtransaction.h"
#include <OTLog.h>

SampleEscrowServer::SampleEscrowServer(BitcoinServerRef rpcServer)
{
    this->rpcServer = rpcServer;

    this->addressForMultiSig = "";
    this->pubKeyForMultiSig = "";
    this->multiSigAddress = "";

    this->serverPool = EscrowPoolRef(NULL);
    this->publicKeys = std::list<std::string>();

    this->transactionDeposit = SampleEscrowTransactionRef(NULL);
    this->transactionWithdrawal = SampleEscrowTransactionRef(NULL);
}

void SampleEscrowServer::OnRequestEscrowDeposit(SampleEscrowClient* client)
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    // create a new object containing information about this deposit
    this->transactionDeposit = SampleEscrowTransactionRef(new SampleEscrowTransaction(client->transactionDeposit->amountToSend));

    // create new address to be used as base for the multi-sig address
    // and get its public key:
    std::string pubKey = GetMultiSigPubKey();

    // also ask the other servers for their public keys
    foreach(SampleEscrowServerRef server, this->serverPool->escrowServers)
    {
        if(server == this)
            continue;

        this->publicKeys.push_back(server->GetMultiSigPubKey());
    }

    // sort the keys alphabetically
    this->publicKeys.sort();

    // generate the multi sig address
    this->multiSigAddrInfo = Modules::mtBitcoin->GetMultiSigAddressInfo(this->minSignatures, this->publicKeys);
    if(this->multiSigAddrInfo != NULL)
        this->multiSigAddress = this->multiSigAddrInfo->address.toStdString();
    this->transactionDeposit->targetAddr = this->multiSigAddress;

    // tell client our public key and how many signatures the pool requires, so client can recreate the address himself
    // we could also just tell him the address, it doesn't really matter
    client->OnReceivePubKey(pubKey, this->minSignatures);    // transmitted over the network
}

std::string SampleEscrowServer::GetMultiSigPubKey()
{
    // see if we already created an address to be used for multi-sig
    if(this->addressForMultiSig == "")
    {
        Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

        // if not, create one:
        this->addressForMultiSig = Modules::mtBitcoin->GetNewAddress();
        // and get its public key
        this->pubKeyForMultiSig = Modules::mtBitcoin->GetPublicKey(this->addressForMultiSig);
        this->publicKeys.push_back(this->pubKeyForMultiSig);
    }

    // send our public key to the server asking for it
    return this->pubKeyForMultiSig;
}

void SampleEscrowServer::OnIncomingDeposit(std::string txId)
{
    // we will need to keep a list of all deposit transaction ids so we know everyone's balance
    // and because we need the tx ids to withdraw later
    this->transactionDeposit->txId = txId;
    this->transactionDeposit->status = SampleEscrowTransaction::Pending;
}

bool SampleEscrowServer::CheckIncomingTransaction()
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    // check transaction for correct amount and number of confirmations
    this->transactionDeposit->CheckTransaction(this->minConfirms);

    if(this->transactionDeposit->status != SampleEscrowTransaction::Pending)
        return true;    // if transaction isn't pending anymore, we're finished

    return false;
}

void SampleEscrowServer::OnRequestEscrowWithdrawal(SampleEscrowClient *client)
{
    Modules::btcRpc->ConnectToBitcoin(this->rpcServer);

    // check if deposit is confirmed
    if(this->transactionDeposit->status != SampleEscrowTransaction::Successfull)
        return;     // nope

    // create new transaction object
    this->transactionWithdrawal = SampleEscrowTransactionRef(new SampleEscrowTransaction(this->transactionDeposit->amountToSend));

    // create partially signed raw transaction
    this->transactionWithdrawal->CreateWithdrawalTransaction(this->transactionDeposit->txId, this->multiSigAddress, client->transactionWithdrawal->targetAddr);

    // send the partially signed transaction to the client
    client->OnReceiveSignedTx(this->transactionWithdrawal->withdrawalTransaction);
}