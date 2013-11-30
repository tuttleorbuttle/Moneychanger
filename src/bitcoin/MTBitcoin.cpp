#include "bitcoin/MTBitcoin.h"
#include "modules.h"




MTBitcoin::MTBitcoin()
{
}

int64_t MTBitcoin::GetBalance(const char* account)
{
    Modules::btcJson->GetBalance(account);
}

std::string MTBitcoin::GetNewAddress(const char* account)
{
    Modules::btcJson->GetNewAddress(account);
}

std::string MTBitcoin::SendToAddress(int64_t lAmount, const std::string & to_address)
{
    Modules::btcJson->SendToAddress(QString(to_address.c_str()), lAmount);
}

std::string MTBitcoin::SendToMultisig(int64_t lAmount, int nRequired, const std::list<std::string> & to_publicKeys)
{
    // will send to a multi-sig address without adding it to the wallet.
    // if you want the address to be added to your wallet, use AddMultiSigAddress() and SendToAddress()

    // copy public keys to qstring list
    QStringList pubKeys;
    foreach(std::string key, to_publicKeys)
    {
        pubKeys.append(key.c_str());
    }

    // generate the multi-sig address
    BtcMultiSigAddressRef multiSigAddr = Modules::btcJson->CreateMultiSigAddress(nRequired, pubKeys);

    // send to the address
    Modules::btcJson->SendToAddress(multiSigAddr->address, lAmount);
}

BtcSignedTransactionRef MTBitcoin::VoteMultiSigRelease(std::string txToSourceId, BtcMultiSigAddressRef multiSigAddress, const std::string & destinationAddress, const std::string & signingAddress)
{
    Modules::btcInterface->WithdrawAllFromAddress(txToSourceId.c_str(), multiSigAddress->address, destinationAddress.c_str(), signingAddress.c_str());
}
