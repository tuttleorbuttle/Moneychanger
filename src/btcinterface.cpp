#include <OTLog.h>
#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"

using namespace BtcJsonObjects;

#define MIN_CONFIRMS 6

BtcInterface::BtcInterface(QObject *parent) :
    QObject(parent)
{
}

// order of keys doesn't really matter
QString BtcInterface::CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey)
{
    QJsonArray keys;
    keys.append(myKey); keys.append(hisKey);
    return Modules::json->AddMultiSigAddress(2, keys, "moneychanger-twooftwo");
}

bool BtcInterface::TestBtcJson()
{
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    //Modules::bitcoinRpc->ConnectToBitcoin("moneychanger", "money1234");
    Modules::json->GetInfo();
    double balance = Modules::json->GetBalance();
    QStringList accounts = Modules::json->ListAccounts();
    QString address = Modules::json->GetNewAddress();
    QStringList keys;
    keys.append(Modules::json->GetNewAddress());    // key can be an address or public key
    keys.append(Modules::json->GetNewAddress());
    QString multiSigAddr = Modules::json->AddMultiSigAddress(2, QJsonArray::fromStringList(keys));  // maybe add them to account moneychanger-multisig or w/e to keep an overview
    Modules::json->GetInfo();

    OTLog::vOutput(0, "Balance: %f\n", balance);
    if(address != NULL)
        OTLog::vOutput(0, "New address created: \"%s\"\n", address.toStdString().c_str());
    if(multiSigAddr != NULL)
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr.toStdString().c_str());

    // test sending, receiving and validating transactions
    // receive to bitcoin-testnet-box #2
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);
    QString recvAddr = Modules::json->GetNewAddress("testAccount");
    Modules::json->SetTxFee(10.2);

    // send from bitcoin-testnet-box #1
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    Modules::json->SetTxFee(10.1);
    QString txID = Modules::json->SendToAddress(recvAddr, 1.23456789);

    if(txID == NULL || txID == "")
        return false;

    // validate transaction
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);
    QSharedPointer<BtcTransaction> transaction = Modules::json->GetTransaction(txID);
    if(transaction == NULL)
        return false;
    double fee = transaction->Fee;
    double amount = transaction->Amount;
    //double totalAmount = transaction->TotalAmount;
    int confirms = transaction->Confirmations;
    QString txAddress = transaction->Address;

    QString success = TransactionSuccessfull(transaction->Amount, transaction, MIN_CONFIRMS) ? "" : "not";
    OTLog::vOutput(0, "Transaction\n\
                   ID %s\n\
                   to Address %s\n\
                   %i of %i confirmations\n\
                   did %s complete successfully.\n",
                   txID.toStdString().c_str(),
                   txAddress.toStdString().c_str(),
                   transaction->Confirmations, MIN_CONFIRMS,
                   success.toStdString().c_str());

    QVariantMap txTargets;
    for(int i = 0; i < 4; i++)
    {
        txTargets[Modules::json->GetNewAddress] = i+0.1*i+0.01*i+0.001*i;
    }
    sendmany

    return true;

    // TODO: see if bitcoin returns an error when attempting to create a multisigaddress with invalid keys
}

// returns whether a transaction has been confirmed often enough
bool BtcInterface::TransactionConfirmed(QSharedPointer<BtcTransaction> transaction, int minConfirms)
{
    return transaction->Confirmations >= minConfirms;   // check if confirmed often enough
}

// returns whether the required amount of btc was received and confirmed often enough
bool BtcInterface::TransactionSuccessfull(double amount, QSharedPointer<BtcTransaction> transaction, int minConfirms)
{
    // TODO: check for rounding errors when comparing amounts?

    return TransactionConfirmed(transaction, minConfirms) && transaction->Amount >= amount; // check if confirmed AND enough btc
}




