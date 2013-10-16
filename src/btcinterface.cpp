#include <OTLog.h>
#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"
#include "utils.h"

using namespace BtcJsonObjects;


BtcInterface::BtcInterface(QObject *parent) :
    QObject(parent)
{
}

// order of keys doesn't really matter
QString BtcInterface::CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey)
{
    QJsonArray keys;
    keys.append(myKey); keys.append(hisKey);
    return Modules::btcJson->AddMultiSigAddress(2, keys, "moneychanger-twooftwo");
}

bool BtcInterface::TestBtcJson()
{
    // initialize bitcoin rpc servers
    // on my computer I use the two bitcoin-testnet-box servers
    // with a slightly modified config so my regular testnet bitcoin-qt client can connect to them.
    // the GUI and debug console is sometimes easier to read than the terminal output of bitcoind
    QSharedPointer<BitcoinServer> server1, server2, bitcoinqt;

    server1.reset(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));
    server2.reset(new BitcoinServer("admin2", "123", "http://127.0.0.1", 19011));
    bitcoinqt.reset(new BitcoinServer("moneychanger", "money1234", "http://127.0.0.1", 8332));

    //-----------------------
    // test various simple rpc calls
    //-----------------------
    Modules::btcRpc->ConnectToBitcoin(server1);
    Modules::btcJson->GetInfo();
    double balance = Modules::btcJson->GetBalance();
    QStringList accounts = Modules::btcJson->ListAccounts();
    QString address = Modules::btcJson->GetNewAddress();
    if(address != NULL)
        OTLog::vOutput(0, "New address created: \"%s\"\n", address.toStdString().c_str());

    OTLog::vOutput(0, "Balance: %f\n", balance);

    // validate address
    if(!Modules::btcJson->ValidateAddress(address))
        return false;
    if(Modules::btcJson->ValidateAddress("notanaddress"))
        return false;

    //------------------------
    // create multisig address
    //------------------------
    QStringList keys;   // list of public keys or addresses
    keys.append(Modules::btcJson->GetNewAddress("testmultisig"));
    keys.append(Modules::btcJson->GetNewAddress("testmultisig"));
    // add new multisig address to account "testmultisig"
    QString multiSigAddr = Modules::btcJson->AddMultiSigAddress(2, QJsonArray::fromStringList(keys), "testmultisig");

    if(multiSigAddr != NULL && multiSigAddr != "")
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr.toStdString().c_str());
    else
        return false;

    keys.clear();
    keys += "3invalidkey";
    keys += "3invalidkey2";
    // attempt to create an address with invalid keys
    QString multiSigAddrInvalid = Modules::btcJson->AddMultiSigAddress(2, QJsonArray::fromStringList(keys), "testmultisig");
    if(multiSigAddrInvalid != NULL && multiSigAddrInvalid != "")
        return false;   // if that's the case we'll have to check the keys for validity

    //--------------
    // sending funds
    //--------------
    // receive to bitcoin-testnet-box #2
    Modules::btcRpc->ConnectToBitcoin(server2);
    QString recvAddr = Modules::btcJson->GetNewAddress("testAccount");
    Modules::btcJson->SetTxFee(10.2);

    // send from bitcoin-testnet-box #1
    Modules::btcRpc->ConnectToBitcoin(server1);

    // set transaction fee
    Modules::btcJson->SetTxFee(10.1);

    // remember how much we send (want) so we can verify the tx later
    double amountRequested = 1.23456789;

    // send the funds
    QString txID = Modules::btcJson->SendToAddress(recvAddr, amountRequested);

    if(txID == NULL || txID == "")
        return false;

    //-----------------------------
    // validate simple transaction (the one we just sent)
    //-----------------------------
    Modules::btcRpc->ConnectToBitcoin(server2);

    // if we call GetTransaction before this client knows about the transaction,
    // it will return error "Invalid or non-wallet transaction id".
    // so we wait for it:
    QSharedPointer<BtcTransaction> transaction = WaitGetTransaction(txID);
    if(transaction == NULL)
        return false;

    double fee = transaction->Fee;
    double amountReceived = transaction->Amount;
    int confirms = transaction->Confirmations;

    // check for correct amount and confirmations
    bool txSuccess = TransactionSuccessfull(amountRequested, transaction, MinConfirms);

    // output some details about transaction success
    QString txSuccessStr = txSuccess ? "" : "not ";
    OTLog::vOutput(0, "Transaction\n\
                   ID %s\n\
                   %i of %i confirmations\n\
                   did %s complete successfully:\n\
                   %fBTC received (you requested %f)",
                   txID.toStdString().c_str(),
                   confirms, MinConfirms,
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
    Modules::btcRpc->ConnectToBitcoin(server1);
    addresses[0] = Modules::btcJson->GetNewAddress("testsendmany");
    addresses[1] = Modules::btcJson->GetNewAddress("testsendmany");

    // connect to second recipient
    Modules::btcRpc->ConnectToBitcoin(server2);
    addresses[2] = Modules::btcJson->GetNewAddress("testsendmany");
    addresses[3] = Modules::btcJson->GetNewAddress("testsendmany");

    // connect to third recipient
    Modules::btcRpc->ConnectToBitcoin(bitcoinqt);
    addresses[4] = Modules::btcJson->GetNewAddress("testsendmany");
    addresses[5] = Modules::btcJson->GetNewAddress("testsendmany");

    // fill the target map
    for(int i = 0; i < 6; i++)
    {
        txTargets[addresses[i]] = amounts[i];
    }

    // connect to sender
    Modules::btcRpc->ConnectToBitcoin(server1);
    // send to the targets
    QString txManyID = Modules::btcJson->SendMany(txTargets);

    if(txManyID == NULL || txManyID == "")
        return false;

    //-----------------------------------------------------
    // validate sendmany transaction (the one we just sent)
    //-----------------------------------------------------
    transaction = WaitGetTransaction(txManyID);

    // validate transaction for recipient #1
    double sent;
    for(int i = 0; i < 6; i++) sent += amounts[i];
    txSuccess = TransactionSuccessfull(amounts[0] + amounts[1] - sent, transaction, MinConfirms);
    if(!txSuccess) return false;

    // validate transaction for recipient #2
    Modules::btcRpc->ConnectToBitcoin(server2);
    transaction = WaitGetTransaction(txManyID);
    txSuccess = TransactionSuccessfull(amounts[2] + amounts[3], transaction, MinConfirms);
    if(!txSuccess) return false;

    // validate transaction for recipient #3
    Modules::btcRpc->ConnectToBitcoin(bitcoinqt);
    transaction = WaitGetTransaction(txManyID);
    txSuccess = TransactionSuccessfull(amounts[4] + amounts[5], transaction, MinConfirms);
    if(!txSuccess) return false;

    return true;
}

bool BtcInterface::TestBtcJsonEscrowTwoOfTwo()
{
    // https://people.xiph.org/~greg/escrowexample.txt

    /*
    2-of-2 escrow example

    Carbide81 wants to pay carbide80 50tnbtc but prevent carebide80 from cheating him.

    First each party creates a new address, and then shares them. Then uses the
    resulting addresses to make a p2sh address (begins with '3' for bitcoin,
    '2' for testnet) that requires both parties to sign to release:

    The distributed redemption here (where no party has all the required keys
    requires bitcoin 0.7 or later)

    Instead the parties could do a 2-of-3 which would allow a mediator to
    settle a dispute, that works exactly the same but instead of two public keys
    three would be provided.
    */

    QSharedPointer<BitcoinServer> buyer, vendor;
    buyer.reset(new BitcoinServer("admin1", "123", "http://127.0.0.1", 19001));
    vendor.reset(new BitcoinServer("admin2", "123", "http://127.0.0.1", 19011));

    // connect to buyer (carbide81):
    Modules::btcRpc->ConnectToBitcoin(buyer);
    // buyer: create new address to be used in multi-sig
    QString addressBuyer = Modules::btcJson->GetNewAddress("testescrow");

    // connect to vendor (carbide80):
    Modules::btcRpc->ConnectToBitcoin(vendor);
    // vendor: create new address to be used in multi-sig
    QString addressVendor = Modules::btcJson->GetNewAddress("testescrow");


    // now the vendor and client have to exchange their public KEYS somehow. addresses won't work here.
    // let's imagine this happened by magic.


    // once exchanged, they can both create the same multi-sig-address using those addresses
    QJsonArray keys;
    keys.append(addressBuyer);
    keys.append(addressVendor);

    // vendor: create multi-sig address
    QString multiSigAddrVendor = Modules::btcJson->AddMultiSigAddress(2, keys, "testescrow");

    // buyer: create multi-sig-address
    Modules::btcRpc->ConnectToBitcoin(buyer);
    QString multiSigAddressBuyer = Modules::btcJson->AddMultiSigAddress(2, keys, "testescrow");

    // buyer: pay the requested amount into the multi-sig address
    double amountRequested = 1.22;  // .22 because two of two escrow...
    QString txToEscrow = Modules::btcJson->SendToAddress(multiSigAddressBuyer, amountRequested);


    // now the client needs to tell the vendor what the transaction ID is
    // alternatively we could use getreceivedbyaddress to wait for any incoming transaction to that address,
    // but let's assume they exchanged the tx ID.


    // vendor: wait for the transaction to be received and confirmed
    Modules::btcRpc->ConnectToBitcoin(vendor);
    WaitForTransaction(txToEscrow);     // wait for the tx to be received
    if(!WaitTransactionSuccessfull(amountRequested, txToEscrow, 1))
        return false;   // wrong btc amount or lack of confirmations after timeout period

    // vendor: withdraw the funds from the multi-sig address into one he owns
    QString withdrawAddr = Modules::btcJson->GetNewAddress("testescrow");

    // vendor: create raw transaction to send from multisig to withdrawaddr
    // rawTx = WithdrawAllFromAddress(multiSigAddrVendor, withdrawAddr);

    // vendor: sign the raw transaction
    // rawTx = Modules::btcJson->SignRawTransaction(rawTx)


    // vendor then sends the signed raw transaction to buyer
    // this happens through magic again


    // buyer: verify that the vendor did not send us a malicious transaction request
    //      because if we blindly sign anything we could be signing a tx stealing all our coins.
    //      my idea is to loop through all rawTx inputs and see if they are the unspent inputs of multiSigAddress.. or something like that.
    // CheckRawTransaction(rawTx, txToEscrow or multiSigAddressClient or whatever)

    // buyer: if everything is alright, sign the raw transaction
    // rawTx = Modules::btcJson->SignRawTransaction(rawTx)

    // buyer: broadcast the transaction
    // Modules::btcJson->SendRawTransaction(rawTx)

    // vendor: wait for the transaction to be confirmed
    // (as above)
    // if confirmed, everything is fine and we can all be happy together.

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

bool BtcInterface::WaitTransactionSuccessfull(double amount, QString txID, int minConfirms, double timeOutSeconds, double timerSeconds)
{
    // TODO: we'll have to replace this function with a constant background check for all unconfirmed transactions
    // and to store the outstanding transactions in some database.
    // On the same machine it only takes a few ms for a transaction to be sent but in the real world
    // this can be arbitrarily long and we don't want a thread running in the background for every unconfirmed
    // transaction for hours, days, months,...

    utils::SleepSimulator sleeper;

    QSharedPointer<BtcTransaction> transaction;

    // if the transaction hasn't even be received yet we'll return.
    // it is therefore a good idea to call WaitForTransaction() before calling this function
    transaction = Modules::btcJson->GetTransaction(txID);
    if(transaction == NULL)
        return false;

    // if the transaction has the wrong amount it will never be successfull so we can return here and not wait for timeout
    if(transaction->Amount < amount)
        return false;

    while((transaction = Modules::btcJson->GetTransaction(txID)) != NULL && (timeOutSeconds -= timerSeconds) > 0)
    {
        if(TransactionSuccessfull(amount, transaction, minConfirms))
            return true;
    }

    return false;
}

bool BtcInterface::WaitForTransaction(QString txID, int timerMS, int maxAttempts)
{
    return !WaitGetTransaction(txID, timerMS, maxAttempts).isNull();
}

QSharedPointer<BtcTransaction> BtcInterface::WaitGetTransaction(QString txID, int timerMS, int maxAttempts)
{
    utils::SleepSimulator sleeper;

    QSharedPointer<BtcTransaction> transaction;

    // TODO: if this blocks the GUI then we should multithread or async it through callbacks
    // or let the user press a refresh button until he gets a result
    while((transaction = Modules::btcJson->GetTransaction(txID)) == NULL && maxAttempts--)
    {
        sleeper.sleep(timerMS);
    }

    return transaction;
}


