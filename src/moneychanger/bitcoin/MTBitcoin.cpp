#include "MTBitcoin.h"
#include "modules.h"




MTBitcoin::MTBitcoin()
{
}

int64_t MTBitcoin::GetBalance(const char* account)
{
    return Modules::btcJson->GetBalance(account);
}

std::string MTBitcoin::GetNewAddress(const char* account)
{
    return Modules::btcJson->GetNewAddress(account).toStdString();
}

std::string MTBitcoin::GetPublicKey(const std::string& address)
{
    return Modules::btcJson->GetPublicKey(QString::fromStdString(address)).toStdString();
}

std::string MTBitcoin::GetMultiSigAddress(int minSignatures, const std::list<std::string>& publicKeys, bool addToWallet, const char* account)
{
    BtcMultiSigAddressRef multiSigAddr;
    return (multiSigAddr = GetMultiSigAddressInfo(minSignatures, publicKeys, addToWallet, account)) == NULL ? "" : multiSigAddr->address.toStdString();
}

BtcMultiSigAddressRef MTBitcoin::GetMultiSigAddressInfo(int minSignatures, const std::list<std::string> &publicKeys, bool addToWallet, const char* account)
{
    // convert std string to qstring list
    QStringList publicKeysQt;
    foreach(std::string pubKey, publicKeys)
    {
        publicKeysQt.append(QString::fromStdString(pubKey));
    }

    BtcMultiSigAddressRef multiSigAddr;
    if(addToWallet)
    {
        multiSigAddr = Modules::btcJson->AddMultiSigAddress(minSignatures, publicKeysQt, account);
    }
    else
    {
        multiSigAddr = Modules::btcJson->CreateMultiSigAddress(minSignatures, publicKeysQt);
    }

    return multiSigAddr;
}

BtcRawTransactionRef MTBitcoin::GetRawTransaction(const std::string &txId)
{
    // wait for transaction and return it
    // times out after 10 seconds by default
    return Modules::btcInterface->WaitGetRawTransaction(QString::fromStdString(txId));
}

int MTBitcoin::GetConfirmations(BtcRawTransactionRef rawTransaction)
{
    return Modules::btcInterface->GetConfirmations(rawTransaction);
}

bool MTBitcoin::TransactionSuccessfull(int64_t amount, BtcRawTransactionRef rawTransaction, const std::string &targetAddress, int64_t confirmations)
{
    return Modules::btcInterface->TransactionSuccessfull(amount, rawTransaction, QString::fromStdString(targetAddress), confirmations);
}

std::string MTBitcoin::SendToAddress(int64_t lAmount, const std::string &to_address)
{
    return Modules::btcJson->SendToAddress(QString::fromStdString(to_address), lAmount).toStdString();
}

std::string MTBitcoin::SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> &to_publicKeys)
{
    // will send to a multi-sig address without adding it to the wallet.
    // if you want the address to be added to your wallet, use AddMultiSigAddress() and SendToAddress()

    // copy public keys to qstring list
    QStringList to_publicKeysQt;
    foreach(std::string key, to_publicKeys)
    {
        to_publicKeysQt.append(QString::fromStdString(key));
    }

    // generate the multi-sig address
    BtcMultiSigAddressRef multiSigAddr = Modules::btcJson->CreateMultiSigAddress(nRequired, to_publicKeysQt);

    // send to the address
    return Modules::btcJson->SendToAddress(multiSigAddr->address, lAmount).toStdString();
}

BtcSignedTransactionRef MTBitcoin::VoteMultiSigRelease(const std::string &txToSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript, const std::string &signingAddress)
{
    return Modules::btcInterface->WithdrawAllFromAddress(QString::fromStdString(txToSourceId),
                                                         QString::fromStdString(sourceAddress),
                                                         QString::fromStdString(destinationAddress),
                                                         QString::fromStdString(redeemScript),
                                                         QString::fromStdString(signingAddress));
}

BtcSignedTransactionRef MTBitcoin::CombineTransactions(const std::string &concatenatedRawTransactions)
{
    return Modules::btcJson->CombineSignedTransactions(QString::fromStdString(concatenatedRawTransactions));
}

std::string MTBitcoin::SendRawTransaction(const std::string &rawTransaction)
{
    return Modules::btcJson->SendRawTransaction(QString::fromStdString(rawTransaction)).toStdString();
}





