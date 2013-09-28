#ifndef BTCINTEGRATION_H
#define BTCINTEGRATION_H

#include <QObject>
#include <QSharedPointer>
#include "btcjson.h"

using namespace BtcJsonReplies;

class BtcInterface : public QObject
{
    Q_OBJECT
public:
    explicit BtcInterface(QObject *parent = 0);

    QString CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey);

    bool TransactionConfirmed(QSharedPointer<BtcTransaction> transaction, int minconfirms = 6);

    bool TransactionSuccessfull(double amount, QSharedPointer<BtcTransaction> transaction, int minConfirms = 6);

    // cba to implement proper unit testing for now so this will have to do:
    bool TestBtcJson();


private:
    
signals:
    
public slots:
    
};

#endif // BTCINTEGRATION_H
