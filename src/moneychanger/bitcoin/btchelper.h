#ifndef BTCHELPER_H
#define BTCHELPER_H

#include <sys/types.h>
#include <string>
#include <tr1/memory>

#include "btcobjects.h"
//#include "btcmodules.h"

class BtcModules;

class BtcHelper
{
public:
    // default amount of confirmations to wait for
    static int MinConfirms;

    BtcHelper(BtcModules* modules);

    // converts a double bitcoin (as received through json api) to int64 satoshis
    static int64_t CoinsToSatoshis(double value);

    // converts int64 satoshis to double bitcoin
    static double SatoshisToCoins(int64_t value);

    // Returns the public key of an address (addresses are just hashes)
    // Pub keys need to be shared to create multi signature addresses
    std::string GetPublicKey(const std::string &address);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(const std::string &transactionId, const std::string &targetAddress);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(BtcRawTransactionRef transaction, const std::string &targetAddress);

    int64_t GetConfirmations(BtcRawTransactionRef transaction);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(BtcTransactionRef transaction, int minconfirms = MinConfirms);

    // Checks whether a transaction (can be non-wallet) has been confirmed often enough
    int64_t TransactionConfirmed(BtcRawTransactionRef transaction, int minConfirms = MinConfirms);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(int64_t amount, BtcTransactionRef transaction, int minConfirms = MinConfirms);

    // Checks a raw transaction for correct amount, confirmations and recipient.
    // We need this because bitcoin-qt offers no good way to watch multi-sig addresses if we don't own all the keys
    bool TransactionSuccessfull(int64_t amount, BtcRawTransactionRef transaction, const std::string &targetAddress, int minConfirms = MinConfirms);


    // Halts thread execution until the transaction has enough confirmations
    // timeOutSeconds is the time in seconds after which the function will fail
    // timerMS is the delay between each confirmation check
    // returns true if sufficient confirmations were received before timeout
    bool WaitTransactionSuccessfull(int64_t amount, BtcTransactionRef transaction, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    bool WaitTransactionSuccessfull(int64_t amount, BtcRawTransactionRef transaction, const std::string &targetAddress, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    // Halts thread execution until the bitcoin client learns about a new transaction
    bool WaitForTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the transaction once it arrives
    // Will only work if you have all keys of the receiving address.
    BtcTransactionRef WaitGetTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the decoded raw transaction once it arrives
    BtcRawTransactionRef WaitGetRawTransaction(const std::string &txID, int timerMS = 500, int maxAttempts = 20);

    // Creates a raw transaction that sends all unspent outputs from an address to another
    // txSourceId: transaction that sends funds to sourceAddress
    // sourceAddress: address from which you want to withdraw
    // destinationAddress: address to which to withdraw
    // signingAddress: only this address's private key will be used to sign the tx
    // redeemScript: the script needed to withdraw btc from p2sh addresses
    BtcSignedTransactionRef WithdrawAllFromAddress(const std::string &txSourceId, const std::string &sourceAddress, const std::string &destinationAddress, const std::string &redeemScript = "", const std::string &signingAddress = "");


private:
    BtcModules* modules;
};

typedef std::shared_ptr<BtcHelper> BtcHelperRef;

#endif // BTCHELPER_H
