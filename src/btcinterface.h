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
    explicit BtcInterface(QObject *parent = 0);

    // This function will faciliate two-of-two escrow
    // Or maybe I will remove it again and replace it with something more dynamic.
    QString CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey);

    // Checks whether a transaction has been confirmed often enough
    bool TransactionConfirmed(QSharedPointer<BtcTransaction> transaction, int minconfirms = 6);

    // Checks a transaction for correct amount and confirmations.
    bool TransactionSuccessfull(double amount, QSharedPointer<BtcTransaction> transaction, int minConfirms = 6);

    // Halts program execution until the bitcoin client learns about a new transaction
    bool WaitForTransaction(QString txID, int timerMS = 500, int maxAttempts = 20);

    // cba to implement proper unit testing so for now this will have to do
    bool TestBtcJson();


private:
    
signals:
    
public slots:
    
};

#endif // BTCINTEGRATION_H
