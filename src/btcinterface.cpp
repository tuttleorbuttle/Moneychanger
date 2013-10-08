#include <OTLog.h>
#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"
#include "utils.h"

using namespace BtcJsonObjects;

#define MIN_CONFIRMS 0  // should be 6

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
    //-----------------------
    // test various simple rpc calls
    //-----------------------
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    Modules::json->GetInfo();
    double balance = Modules::json->GetBalance();
    QStringList accounts = Modules::json->ListAccounts();
    QString address = Modules::json->GetNewAddress();
    if(address != NULL)
        OTLog::vOutput(0, "New address created: \"%s\"\n", address.toStdString().c_str());

    OTLog::vOutput(0, "Balance: %f\n", balance);

    //------------------------
    // create multisig address
    //------------------------
    QStringList keys;   // list of public keys or addresses
    keys.append(Modules::json->GetNewAddress("testmultisig"));
    keys.append(Modules::json->GetNewAddress("testmultisig"));
    // add new multisig address to account "testmultisig"
    QString multiSigAddr = Modules::json->AddMultiSigAddress(2, QJsonArray::fromStringList(keys), "testmultisig");

    if(multiSigAddr != NULL && multiSigAddr != "")
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr.toStdString().c_str());
    else
        return false;

    keys.clear();
    keys += "3invalidkey";
    keys += "3invalidkey2";
    // attempt to create an address with invalid keys
    QString multiSigAddrInvalid = Modules::json->AddMultiSigAddress(2, QJsonArray::fromStringList(keys), "testmultisig");
    if(multiSigAddrInvalid != NULL && multiSigAddrInvalid != "")
        return false;   // if that's the case we'll have to check the keys for validity

    //--------------
    // sending funds
    //--------------
    // receive to bitcoin-testnet-box #2
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);
    QString recvAddr = Modules::json->GetNewAddress("testAccount");
    Modules::json->SetTxFee(10.2);

    // send from bitcoin-testnet-box #1
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);

    // set transaction fee
    Modules::json->SetTxFee(10.1);

    // remember how much we send (want) so we can verify the tx later
    double amountRequested = 1.23456789;

    // send the funds
    QString txID = Modules::json->SendToAddress(recvAddr, amountRequested);

    if(txID == NULL || txID == "")
        return false;

    //-----------------------------
    // validate simple transaction (the one we just sent)
    //-----------------------------
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);

    // if we call GetTransaction before this client knows about the transaction,
    // it will return error "Invalid or non-wallet transaction id".
    // so we wait for it:
    WaitForTransaction(txID);
    QSharedPointer<BtcTransaction> transaction = Modules::json->GetTransaction(txID);
    if(transaction == NULL)
        return false;

    double fee = transaction->Fee;
    double amountReceived = transaction->Amount;
    int confirms = transaction->Confirmations;

    // check for correct amount and confirmations
    bool txSuccess = TransactionSuccessfull(amountRequested, transaction, MIN_CONFIRMS);

    // output some details about transaction success
    QString txSuccessStr = txSuccess ? "" : "not ";
    OTLog::vOutput(0, "Transaction\n\
                   ID %s\n\
                   %i of %i confirmations\n\
                   did %s complete successfully:\n\
                   %fBTC received (you requested %f)",
                   txID.toStdString().c_str(),
                   confirms, MIN_CONFIRMS,
                   txSuccessStr.toStdString().c_str(),
                   amountReceived, amountRequested);
    if(!txSuccess) return false;

    //---------
    // sendmany
    //---------
    QVariantMap txTargets;  // maps amounts to addresses

    double amounts[6];      // amounts that will be sent in one tx
    QString addresses[6];   // addresses to which to send those amounts
    for(int i = 0; i < 6; i++)
    {
        amounts[i] = i + 1;     // generate the amounts (0 is invalid)
    }

    // connect to first recipient, who will also be the sender
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123","http://127.0.0.1", 19001);
    addresses[0] = Modules::json->GetNewAddress("testsendmany");
    addresses[1] = Modules::json->GetNewAddress("testsendmany");

    // connect to second recipient
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123", "http://127.0.0.1", 19011);
    addresses[2] = Modules::json->GetNewAddress("testsendmany");
    addresses[3] = Modules::json->GetNewAddress("testsendmany");

    // connect to third recipient
    Modules::bitcoinRpc->ConnectToBitcoin("moneychanger", "money1234", "http://127.0.0.1", 8332);
    addresses[4] = Modules::json->GetNewAddress("testsendmany");
    addresses[5] = Modules::json->GetNewAddress("testsendmany");

    // fill the target map
    for(int i = 0; i < 6; i++)
    {
        txTargets[addresses[i]] = amounts[i];
    }

    // connect to sender
    Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123","http://127.0.0.1", 19001);
    // send to the targets
    QString txManyID = Modules::json->SendMany(txTargets);

    if(txManyID == NULL || txManyID == "")
        return false;

    //-----------------------------------------------------
    // validate sendmany transaction (the one we just sent)
    //-----------------------------------------------------
    WaitForTransaction(txManyID);   // TODO: tx = WaitForAndGetTransaction(txID)
    transaction = Modules::json->GetTransaction(txManyID);

    // validate transaction for recipient #1
    double sent;
    for(int i = 0; i < 6; i++) sent += amounts[i];
    txSuccess = TransactionSuccessfull(amounts[0] + amounts[1] - sent, transaction, MIN_CONFIRMS);
    if(!txSuccess) return false;

    // validate transaction for recipient #2
    Modules::bitcoinRpc->ConnectToBitcoin("admin2", "123","http://127.0.0.1", 19011);
    WaitForTransaction(txManyID);
    transaction = Modules::json->GetTransaction(txManyID);
    txSuccess = TransactionSuccessfull(amounts[2] + amounts[3], transaction, MIN_CONFIRMS);
    if(!txSuccess) return false;

    // validate transaction for recipient #3
    Modules::bitcoinRpc->ConnectToBitcoin("moneychanger", "money1234", "http://127.0.0.1", 8332);
    WaitForTransaction(txManyID);
    transaction = Modules::json->GetTransaction(txManyID);
    txSuccess = TransactionSuccessfull(amounts[4] + amounts[5], transaction, MIN_CONFIRMS);
    if(!txSuccess) return false;

    return true;
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

bool BtcInterface::WaitForTransaction(QString txID, int timerMS, int maxAttempts)
{
    utils::SleepSimulator sleeper;

    // TODO: if this blocks the GUI then we should multithread or async it
    // or let the user press a refresh button until he gets a result
    while(Modules::json->GetTransaction(txID) == NULL && maxAttempts--)
    {
        sleeper.sleep(1000);
    }
}




