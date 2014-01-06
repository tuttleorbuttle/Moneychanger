#ifndef BTCINTEGRATION_H
#define BTCINTEGRATION_H

#include <QObject>
#include <QSharedPointer>
#include "btcjsonqt.h"


class BtcInterface : public QObject
{
    Q_OBJECT
public:
    static const int MinConfirms = 0;   // default is 6

    explicit BtcInterface(QObject *parent = 0);

    // This function will faciliate two-of-two escrow
    // Or maybe I will remove it again and replace it with something more dynamic.
    QString CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey);

    // Returns the public key of an address (addresses are just hashes)
    // Pub keys need to be shared to create multi signature addresses
    QString GetPublicKey(QString address);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(QString transactionId, QString targetAddress);

    // Counts how many coins are sent to targetAddress through this transaction
    int64_t GetTotalOutput(BtcRawTransactionQtRef transaction, QString targetAddress);

    int64_t GetConfirmations(BtcRawTransactionQtRef transaction);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(BtcTransactionQtRef transaction, int minconfirms = MinConfirms);

    // Checks whether a transaction (can be non-wallet) has been confirmed often enough
    int64_t TransactionConfirmed(BtcRawTransactionQtRef transaction, int minConfirms = MinConfirms);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(int64_t amount, BtcTransactionQtRef transaction, int minConfirms = MinConfirms);

    // Checks a raw transaction for correct amount, confirmations and recipient.
    // We need this because bitcoin-qt offers no good way to watch multi-sig addresses if we don't own all the keys
    bool TransactionSuccessfull(int64_t amount, BtcRawTransactionQtRef transaction, QString targetAddress, int minConfirms = MinConfirms);


    // Halts thread execution until the transaction has enough confirmations
    // timeOutSeconds is the time in seconds after which the function will fail
    // timerMS is the delay between each confirmation check
    // returns true if sufficient confirmations were received before timeout
    bool WaitTransactionSuccessfull(int64_t amount, BtcTransactionQtRef transaction, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    bool WaitTransactionSuccessfull(int64_t amount, BtcRawTransactionQtRef transaction, QString targetAddress, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    // Halts thread execution until the bitcoin client learns about a new transaction
    bool WaitForTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the transaction once it arrives
    // Will only work if you have all keys of the receiving address.
    BtcTransactionQtRef WaitGetTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the decoded raw transaction once it arrives
    BtcRawTransactionQtRef WaitGetRawTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // Creates a raw transaction that sends all unspent outputs from an address to another
    // txSourceId: transaction that sends funds to sourceAddress
    // sourceAddress: address from which you want to withdraw
    // destinationAddress: address to which to withdraw
    // signingAddress: only this address's private key will be used to sign the tx
    // redeemScript: the script needed to withdraw btc from p2sh addresses
    BtcSignedTransactionQtRef WithdrawAllFromAddress(QString txSourceId, QString sourceAddress, QString destinationAddress, QString redeemScript = "", QString signingAddress = "");

    // cba to implement proper unit testing so for now this will have to do
    bool TestBtcJson();
    bool TestBtcJsonEscrowTwoOfTwo();


private:
    
signals:
    
public slots:
    
};

#endif // BTCINTEGRATION_H
