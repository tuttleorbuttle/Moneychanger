#include "MTBitcoin.h"
#include "modules.h"




MTBitcoin::MTBitcoin(BtcModulesRef modules)
{
    this->modules = modules;
}

MTBitcoin::~MTBitcoin()
{
    this->modules.reset();
}

int64_t MTBitcoin::GetBalance(const char* account)
{
    return this->modules->btcJson->GetBalance(account);
}

std::string MTBitcoin::GetNewAddress(const char* account)
{
    return this->modules->btcJson->GetNewAddress(account);
}

std::string MTBitcoin::GetPublicKey(const std::string& address)
{
    return this->modules->btcJson->GetPublicKey(address);
}

std::string MTBitcoin::GetMultiSigAddress(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet, const char* account)
{
    BtcMultiSigAddressRef multiSigAddr;
    return (multiSigAddr = GetMultiSigAddressInfo(minSignatures, publicKeys, addToWallet, account)) == NULL ? "" : multiSigAddr->address;
}

BtcMultiSigAddressRef MTBitcoin::GetMultiSigAddressInfo(int minSignatures, const std::list<std::string> &publicKeys, bool addToWallet, const char* account)
{
    BtcMultiSigAddressRef multiSigAddr;
    if(addToWallet)
    {
        multiSigAddr = this->modules->btcJson->AddMultiSigAddress(minSignatures, publicKeys, account);
    }
    else
    {
        multiSigAddr = this->modules->btcJson->CreateMultiSigAddress(minSignatures, publicKeys);
    }

    return multiSigAddr;
}

BtcRawTransactionRef MTBitcoin::GetRawTransaction(const std::string &txId)
{
    // wait for transaction and return it
    // times out after 10 seconds by default
    return this->modules->btcHelper->WaitGetRawTransaction(txId);
}

int MTBitcoin::GetConfirmations(BtcRawTransactionRef rawTransaction)
{
    return this->modules->btcHelper->GetConfirmations(rawTransaction);
}

bool MTBitcoin::TransactionSuccessfull(int64_t amount, BtcRawTransactionRef rawTransaction, const std::string &targetAddress, int64_t confirmations)
{
    return this->modules->btcHelper->TransactionSuccessfull(amount, rawTransaction, targetAddress, confirmations);
}

std::string MTBitcoin::SendToAddress(int64_t lAmount, const std::string &to_address)
{
    return this->modules->btcJson->SendToAddress(to_address, lAmount);
}

std::string MTBitcoin::SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> &to_publicKeys)
{
    // will send to a multi-sig address without adding it to the wallet.
    // if you want the address to be added to your wallet, use AddMultiSigAddress() and SendToAddress()

    // generate the multi-sig address
    BtcMultiSigAddressRef multiSigAddr = this->modules->btcJson->CreateMultiSigAddress(nRequired, to_publicKeys);

    // send to the address
    return this->modules->btcJson->SendToAddress(multiSigAddr->address, lAmount);
}

BtcSignedTransactionRef MTBitcoin::VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript, const std::string &signingAddress)
{
    return this->modules->btcHelper->WithdrawAllFromAddress(txToSourceId,
                                                         sourceAddress,
                                                         destinationAddress,
                                                         redeemScript,
                                                         signingAddress);
}

BtcSignedTransactionRef MTBitcoin::CombineTransactions(const std::string &concatenatedRawTransactions)
{
    return this->modules->btcJson->CombineSignedTransactions(concatenatedRawTransactions);
}

std::string MTBitcoin::SendRawTransaction(const std::string &rawTransaction)
{
    return this->modules->btcJson->SendRawTransaction(rawTransaction);
}





