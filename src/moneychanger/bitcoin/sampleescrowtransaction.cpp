#include "sampleescrowtransaction.h"
#include "modules.h"
#include <OTLog.h>

SampleEscrowTransaction::SampleEscrowTransaction(int64_t amountToSend)
{
    this->amountToSend = amountToSend;

    this->status = NotStarted;
    this->txId = "";
    this->targetAddr = "";
    this->confirmations = 0;
    this->withdrawalTransaction = "";

    this->modules = BtcModules::staticInstance;
}

bool SampleEscrowTransaction::SendToTarget()
{
    // send to this address, get transaction id
    this->txId = this->modules->mtBitcoin->SendToAddress(this->amountToSend, this->targetAddr);

    // check if we got a tx id
    if(this->txId == "")
    {
        this->status == Failed;
        return false;
    }

    this->status = Pending;
    return true;
}

bool SampleEscrowTransaction::CreateWithdrawalTransaction(const std::string& sourceTxId, const std::string& multiSigSourceAddress, const std::string& targetAddr)
{
    this->sourceTxId = sourceTxId;
    this->targetAddr = targetAddr;

    // create a partially signed transaction releasing funds that were sent to multiSigAddress in transaction sourceTxId to targetAddr
    BtcSignedTransactionRef signedTx = this->modules->mtBitcoin->VoteMultiSigRelease(sourceTxId, multiSigSourceAddress, targetAddr);
    if(signedTx == NULL)
    {
        this->status = Failed;
        return "";
    }

    this->withdrawalTransaction = signedTx->signedTransaction;

    return signedTx->complete;
}

bool SampleEscrowTransaction::AddWithdrawalTransaction(const std::string &partiallySignedTx)
{
    // concatenate raw transactions and let bitcoin- merge them to one
    BtcSignedTransactionRef signedTx = this->modules->mtBitcoin->CombineTransactions(this->withdrawalTransaction + partiallySignedTx);
    if(signedTx == NULL)
        return false;

    this->withdrawalTransaction = signedTx->signedTransaction;

    // return if enough signatures were collected
    return signedTx->complete;
}

bool SampleEscrowTransaction::SendWithdrawalTransaction()
{
    this->txId = this->modules->mtBitcoin->SendRawTransaction(this->withdrawalTransaction);

    if(this->txId == "")
    {
        this->status = Failed;
        return false;
    }

    this->status = Pending;
    return true;
}

void SampleEscrowTransaction::CheckTransaction(int minConfirms)
{
    if(this->status != Pending)
        return;

    // wait for the transaction to be broadcasted over the network
    // and get an object containing info
    // we have to use raw transactions here because bitcoin- doesn't properly support multi-sig yet
    BtcRawTransactionRef rawTx = this->modules->mtBitcoin->GetRawTransaction(this->txId);

    if(rawTx == NULL)
    {
        // error, transaction apparently wasn't broadcasted or we don't have internet.
    }

    // check if transaction has enough confirmations
    if(this->modules->mtBitcoin->TransactionSuccessfull(this->amountToSend, rawTx, this->targetAddr, minConfirms))
    {
        this->confirmations = this->modules->mtBitcoin->GetConfirmations(rawTx);
        // yay
        this->status = Successfull;
    }

    this->confirmations = this->modules->mtBitcoin->GetConfirmations(rawTx);
}
