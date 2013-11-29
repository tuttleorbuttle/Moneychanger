#include <OTLog.h>
#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"
#include "utils.h"


BtcInterface::BtcInterface(QObject *parent) :
    QObject(parent)
{
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
    BtcMultiSigAddressRef multiSigAddr = Modules::btcJson->AddMultiSigAddress(2, keys, "testmultisig");

    if(multiSigAddr != NULL && !multiSigAddr->address.isEmpty())
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr->address.toStdString().c_str());
    else
        return false;

    keys.clear();
    keys += "3invalidkey";
    keys += "3invalidkey2";
    // attempt to create an address with invalid keys
    BtcMultiSigAddressRef multiSigAddrInvalid = Modules::btcJson->AddMultiSigAddress(2, keys, "testmultisig");
    if(multiSigAddrInvalid != NULL && !multiSigAddrInvalid->address.isNull())
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
    BtcTransactionRef transaction = WaitGetTransaction(txID);
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

    //----------------------
    // test raw transactions
    //----------------------
    QString rawTx = Modules::btcJson->GetRawTransaction(txManyID);
    BtcRawTransactionRef decodedTx = Modules::btcJson->DecodeRawTransaction(rawTx);
    if(!decodedTx)
        return false;

    return true;
}

bool BtcInterface::TestBtcJsonEscrowTwoOfTwo()
{
    // TODO: transaction fee. currently raw transactions aren't mined.

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
    vendor.reset(new BitcoinServer("moneychanger", "money1234", "http://127.0.0.1", 8332));


    // connect to buyer (carbide81):
    Modules::btcRpc->ConnectToBitcoin(buyer);
    // buyer: create new address to be used in multi-sig
    QString addressBuyer = Modules::btcJson->GetNewAddress("testescrow");
    QString pubKeyBuyer = GetPublicKey(addressBuyer);

    // connect to vendor (carbide80):
    Modules::btcRpc->ConnectToBitcoin(vendor);
    // vendor: create new address to be used in multi-sig
    QString addressVendor = Modules::btcJson->GetNewAddress("testescrow");
    QString pubKeyVendor = GetPublicKey(addressVendor);


    // now the vendor and client have to exchange their public KEYS somehow. addresses won't work here.
    // let's imagine this happened by magic.


    // once exchanged, they can both create the same multi-sig-address using those public keys
    QStringList publicKeys;
    publicKeys.append(pubKeyBuyer);
    publicKeys.append(pubKeyVendor);

    // vendor: create multi-sig address
   BtcMultiSigAddressRef multiSigAddressVendor = Modules::btcJson->AddMultiSigAddress(2, publicKeys, "testescrow");

    // buyer: create multi-sig-address
    Modules::btcRpc->ConnectToBitcoin(buyer);
    BtcMultiSigAddressRef multiSigAddressBuyer = Modules::btcJson->AddMultiSigAddress(2, publicKeys, "testescrow");

    // buyer: pay the requested amount into the multi-sig address
    double amountRequested = 1.22;  // .22 because two of two escrow...
    QString txToEscrow = Modules::btcJson->SendToAddress(multiSigAddressBuyer->address, amountRequested);


    // now the client needs to tell the vendor what the transaction ID is
    // alternatively we could wait for any incoming transaction to that address,
    // but let's assume they exchanged the tx ID.


    // vendor: wait for the transaction to be received and confirmed
    Modules::btcRpc->ConnectToBitcoin(vendor);
    BtcRawTransactionRef transaction = WaitGetRawTransaction(txToEscrow, 500, 3);     // wait for the tx to be received
    if(!WaitTransactionSuccessfull(amountRequested, transaction, multiSigAddressVendor->address, 1))  // see if it has 1 confirmation
        return false;   // wrong btc amount or lack of confirmations after timeout period

     // vendor: create address to withdraw the funds from the p2sh
    QString withdrawAddr = Modules::btcJson->GetNewAddress("testescrow");

    // vendor: create raw withdrawal transaction
    BtcSignedTransactionRef withdrawTransactionVendor =
            WithdrawAllFromAddress(txToEscrow, multiSigAddressVendor->address, withdrawAddr, addressVendor,
                                   multiSigAddressVendor->redeemScript);

    if(withdrawTransactionVendor == NULL)
        return false;


    // vendor then tells the client which address he would like to withdraw funds to
    // this happens through magic again


    // buyer: if everything is alright, allow vendor to withdraw funds by creating a
    //      partially signed transaction sending them to the vendor withdrawal address
    Modules::btcRpc->ConnectToBitcoin(buyer);
    BtcSignedTransactionRef withdrawTransactionBuyer =
            WithdrawAllFromAddress(txToEscrow, multiSigAddressBuyer->address, withdrawAddr, addressBuyer,
                                   multiSigAddressBuyer->redeemScript);

    if(withdrawTransactionBuyer == NULL)
        return false;


    // buyer then sends the partially signed transaction to the vendor
    // *magic*


    // vendor: receive the signed tx from buyer and combines it with his half
    Modules::btcRpc->ConnectToBitcoin(vendor);
    // vendor: concatenate partially signed transactions
    QString signedTransactions =
            withdrawTransactionVendor->signedTransaction +
            withdrawTransactionBuyer->signedTransaction;
    BtcSignedTransactionRef signedWithdrawalFinal = Modules::btcJson->CombineSignedTransactions(signedTransactions);

    if(signedWithdrawalFinal == NULL)
        return false;

    // vendor: broadcast signed withdrawal transaction
    QString txFinalizedId = Modules::btcJson->SendRawTransaction(signedWithdrawalFinal->signedTransaction);

    // vendor: see if the transaction was broadcasted
    Modules::btcRpc->ConnectToBitcoin(vendor);
    BtcTransactionRef transactionToWithdrawAddr = WaitGetTransaction(txFinalizedId);
    if(transactionToWithdrawAddr == NULL)
        return false;

    // vendor: wait for the transaction to be confirmed and have right amount
    if(!WaitTransactionSuccessfull(amountRequested, transactionToWithdrawAddr, 1))
        return false;

    // if confirmed, everything is fine and we can all be happy together.

    return true;
}

QString BtcInterface::GetPublicKey(QString address)
{
    QSharedPointer<BtcAddressInfo> addressInfo = Modules::btcJson->ValidateAddress(address);
    if(addressInfo == NULL)
        return NULL;

    return addressInfo->pubkey;
}

double BtcInterface::GetTotalOutput(QString transactionId, QString targetAddress)
{
    // Wait for the transaction to be received
    return GetTotalOutput(WaitGetRawTransaction(transactionId), targetAddress);
}

double BtcInterface::GetTotalOutput(BtcRawTransactionRef transaction, QString targetAddress)
{
    double amountReceived = 0.0;
    for(int i = 0; i < transaction->outputs.size(); i++)
    {
        // I don't know what outputs to multiple addresses mean so I'm not gonna trust them for now.
        if(transaction->outputs[i].addresses.size() > 1)
        {
            OTLog::vOutput(0, "Multiple output addresses per output detected.");
            continue;
        }

        // TODO: vulnerability fix
        // I don't know much about scriptPubKey but I think a malicious buyer could create a
        // transaction that isn't spendable by anyone, see
        // https://en.bitcoin.it/wiki/Script#Provably_Unspendable.2FPrunable_Outputs
        // I think the easiest solution would be to check
        // if scriptPubKey.hex != "76a914<pub key hash>88ac" return false
        // as this seems to be the hex representation of the most basic standard transaction.


        if(transaction->outputs[i].addresses.contains(targetAddress))   // check if the output leads to our address
            amountReceived += transaction->outputs[i].value;            // if it does, add output to total received bitcoins
    }

    return amountReceived;
}

bool BtcInterface::TransactionConfirmed(BtcTransactionRef transaction, int minConfirms)
{
    return transaction->Confirmations >= minConfirms;   // check if confirmed often enough
}

bool BtcInterface::TransactionConfirmed(BtcRawTransactionRef transaction, int minConfirms)
{
    // firstly, see if the transaction is included in a block yet
    //getrawmempool
    QList<QString> rawMemPool = Modules::btcJson->GetRawMemPool();
    if(rawMemPool.contains(transaction->txID))
        return 0 >= minConfirms;    // 0 confirmations

    // if it is, we'll need to loop through the latest blocks until we find it
    // getblockcount --> getblockhash(count) --> getblock(hash) --> getblock(block->previous) -->...
    int latestBlock = Modules::btcJson->GetBlockCount();
    QString blockHash = Modules::btcJson->GetBlockHash(latestBlock);
    BtcBlockRef currentBlock = Modules::btcJson->GetBlock(blockHash);

    while(!currentBlock->transactions.contains(transaction->txID))
    {
        currentBlock = Modules::btcJson->GetBlock(currentBlock->previousHash);
        if(currentBlock == NULL)
            return false;   // this should never happen I think
    }

    // if we find it in an old enough block, return true
    return true;
}

// returns whether the required amount of btc was received and confirmed often enough
bool BtcInterface::TransactionSuccessfull(double amount, BtcTransactionRef transaction, int minConfirms)
{
    // TODO: check for rounding errors when comparing amounts?

    return TransactionConfirmed(transaction, minConfirms) && transaction->Amount >= amount; // check if confirmed AND enough btc
}

bool BtcInterface::TransactionSuccessfull(double amountRequested, BtcRawTransactionRef transaction, QString targetAddress, int minConfirms)
{
    if(transaction == NULL) // if it hasn't been received yet we will return.
        return false;       // use WaitForTransaction(txid) to prevent this.

    // check for sufficient confirms...
    if(!TransactionConfirmed(transaction, minConfirms))
        return false;

    // check for sufficient amount...
    if(GetTotalOutput(transaction, targetAddress) >= amountRequested)
        return true;    // if we were sent at least as much money as requested, return true

    return false;
}

bool BtcInterface::WaitTransactionSuccessfull(double amount, BtcTransactionRef transaction, int minConfirms, double timeOutSeconds, double timerSeconds)
{
    // TODO: we'll have to replace this function with a constant background check for all unconfirmed transactions
    // and to store the outstanding transactions in some database.
    // On the same machine it only takes a few ms for a transaction to be sent but in the real world
    // this can be arbitrarily long and we don't want a different thread running in the background for every unconfirmed
    // transaction for hours, days, months,...

    if(transaction == NULL)
        return false;

    // if the transaction has the wrong amount it will never be successfull so we can return here and not wait for timeout
    if(!TransactionSuccessfull(amount, transaction, 0))
        return false;

    timerSeconds = timerSeconds > 0.001 ? timerSeconds : 0.001;   // prevent division by zero
    double timerMS = timerSeconds * 1000;
    utils::SleepSimulator sleeper;
    while((transaction = Modules::btcJson->GetTransaction(transaction->TxID)) != NULL && (timeOutSeconds -= timerSeconds) > 0)
    {
        if(TransactionSuccessfull(amount, transaction, minConfirms))
            return true;

        sleeper.sleep(timerMS);
    }

    return false;
}

bool BtcInterface::WaitTransactionSuccessfull(double amount, BtcRawTransactionRef transaction, QString targetAddress, int minConfirms, double timeOutSeconds, double timerSeconds)
{
    // TODO: See the other WaitTransactionSuccessfull()

    if(transaction == NULL)
        return false;

    if(!TransactionSuccessfull(amount, transaction, targetAddress, 0))
        return false;

    timerSeconds = timerSeconds > 0.001 ? timerSeconds : 0.001;   // prevent division by zero
    double timerMS = timerSeconds * 1000;
    utils::SleepSimulator sleeper;

    // see if the transaction is successfull...
    while((timeOutSeconds -= timerSeconds) > 0)
    {
        if(TransactionSuccessfull(amount, transaction, targetAddress, minConfirms))
            return true;

        sleeper.sleep(timerMS);
    }

    return false;
}

bool BtcInterface::WaitForTransaction(QString txID, int timerMS, int maxAttempts)
{
    return WaitGetRawTransaction(txID, timerMS, maxAttempts) != NULL;
}

BtcTransactionRef BtcInterface::WaitGetTransaction(QString txID, int timerMS, int maxAttempts)
{
    utils::SleepSimulator sleeper;

    BtcTransactionRef transaction;

    // TODO: if this blocks the GUI then we should multithread or async it through callbacks
    // or let the user press a refresh button until he gets a result
    while((transaction = Modules::btcJson->GetTransaction(txID)) == NULL && maxAttempts--)
    {
        sleeper.sleep(timerMS);
    }

    return transaction;
}

BtcRawTransactionRef BtcInterface::WaitGetRawTransaction(QString txID, int timerMS, int maxAttempts)
{
    utils::SleepSimulator sleeper;

    BtcRawTransactionRef transaction;

    // TODO: see WaitGetTransaction()
    while((transaction = Modules::btcJson->GetDecodedRawTransaction(txID)) == NULL && maxAttempts--)
    {
        sleeper.sleep(timerMS);
    }

    return transaction;
}

BtcSignedTransactionRef BtcInterface::WithdrawAllFromAddress(QString txToSourceId, QString sourceAddress, QString destinationAddress, QString signingAddress, QString redeemScript)
{
    // retrieve decoded raw transaction sending funds to our sourceAddress
    BtcRawTransactionRef rawTransaction = Modules::btcJson->GetDecodedRawTransaction(txToSourceId);
    if(rawTransaction == NULL)
        return BtcSignedTransactionRef();   // return NULL

    // count funds in source address and list outputs leading to it
    // this will only work when no btc from source tx have been spent from that address yet
    double funds = 0;
    QList<BtcOutput> unspentOutputs;
    QList<BtcSigningPrequisite> signingPrequisites;
    foreach(BtcRawTransaction::VOUT output, rawTransaction->outputs)
    {
        // check if output leads to sourceAddess
        // idk what multiple addresses per output means so we'll ignore those cases
        if(output.addresses.size() == 1 && output.addresses[0] == sourceAddress)
        {
            funds += output.value;
            unspentOutputs += BtcOutput(txToSourceId, output.n);
            signingPrequisites += BtcSigningPrequisite(txToSourceId, output.n, output.scriptPubKeyHex, redeemScript);
        }
    }

    // create raw transaction to send outputs to target address
    QVariantMap txTargets;
    txTargets[destinationAddress] = funds;  // TODO: calculate fee
    QString withdrawTransaction = Modules::btcJson->CreateRawTransaction(unspentOutputs, txTargets);

    // sign raw transaction
    // as we just created this tx ourselves, we can assume that it is safe to sign
    // however, we need to provide a private key to sign multi-sig-addresses for which we don't have all the keys
    QStringList signingKeys;
    if(signingAddress != NULL)
    {
        signingKeys += Modules::btcJson->GetPrivateKey(signingAddress);
    }
    BtcSignedTransactionRef signedTransact = Modules::btcJson->SignRawTransaction(withdrawTransaction, signingPrequisites, signingKeys);

    return signedTransact;
}








