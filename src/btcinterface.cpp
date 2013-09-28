#include <OTLog.h>
#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"

using namespace BtcJsonReplies;

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
    QString recvAddr = Modules::json->GetNewAddress();
    Modules::json->SetTxFee(10.2);

    // send from bitcoin-testnet-box #1
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    Modules::json->SetTxFee(10.1);
    QString txID = Modules::json->SendToAddress(recvAddr, 1.23456789);
    QString txIDinvalid = Modules::json->SendToAddress("1qthisisnotanaddress", 1.23456789);

    if(txID == NULL || txID == "")
        return false;

    // validate transaction
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);
    QSharedPointer<BtcTransaction> transaction = Modules::json->GetTransaction(txID);
    if(transaction == NULL)
        return false;
    double fee = transaction->Fee;
    double amount = transaction->Amount;
    double totalAmount = transaction->TotalAmount;
    int confirms = transaction->Confirmations;

    QString success = TransactionSuccessfull(transaction->Amount, transaction, 0) ? "" : "not";
    OTLog::vOutput(0, "Transaction %s %s completed successfully, %i confirmations", txID.toStdString().c_str(), success.toStdString().c_str(), transaction->Confirmations);

    return true;
}

bool BtcInterface::TransactionConfirmed(QSharedPointer<BtcTransaction> transaction, int minConfirms)
{
    return transaction->Confirmations >= minConfirms;   // check if confirmed often enough
}

bool BtcInterface::TransactionSuccessfull(double amount, QSharedPointer<BtcTransaction> transaction, int minConfirms)
{
    return TransactionConfirmed(transaction, minConfirms) && transaction->Amount >= amount; // check if confirmed AND enough btc
}
