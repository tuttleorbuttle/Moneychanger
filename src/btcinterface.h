#ifndef BTCINTEGRATION_H
#define BTCINTEGRATION_H

#include <QObject>
#include <QSharedPointer>
#include "btcjson.h"

using namespace BtcJsonObjects;

class BtcInterface : public QObject
{
    Q_OBJECT
public:
    static const int MinConfirms = 0;   // default is 6

    explicit BtcInterface(QObject *parent = 0);

    // This function will faciliate two-of-two escrow
    // Or maybe I will remove it again and replace it with something more dynamic.
    QString CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(QSharedPointer<BtcTransaction> transaction, int minconfirms = MinConfirms);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(double amount, QSharedPointer<BtcTransaction> transaction, int minConfirms = MinConfirms);

    // Halts thread execution until the transaction has enough confirmations
    // timeOutSeconds is the time in seconds after which the function will fail
    // timerMS is the delay between each confirmation check
    // returns true if sufficient confirmations were received before timeout
    bool WaitTransactionSuccessfull(double amount, QString txID, int minConfirms = MinConfirms, double timeOutSeconds = 7200, double timerSeconds = 1);

    // Halts thread execution until the bitcoin client learns about a new transaction
    bool WaitForTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // Halts thread execution and returns the transaction once it arrives
    QSharedPointer<BtcTransaction> WaitGetTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // Creates a raw transaction that sends all inputs of
    QString WithdrawAllFromAddress(QString sourceAddress, QString destinationAddress);

    // cba to implement proper unit testing so for now this will have to do
    bool TestBtcJson();
    bool TestBtcJsonEscrowTwoOfTwo();


private:
    
signals:
    
public slots:
    
};

#endif // BTCINTEGRATION_H
