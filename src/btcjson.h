#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "btcrpc.h"


namespace BtcJsonReplies
{
    // if a query returns a complex object we should convert it to a struct instead of
    // passing around the QJsonObject

    struct BtcTransaction
    {
        BtcTransaction(QJsonObject reply)
            :TotalAmount(reply["amount"].toDouble()),
            Confirmations((int)(reply["confirmations"].toDouble())),
            Fee(reply["fee"].toDouble())
        {

        }
        double TotalAmount;
        int Confirmations;
        double Amount;
        double Fee;
        QString TxID;
        QString Time;       // is this a string?
        QString Account;
        QString Address;
        QString Category;   // dafuq is this?
    };
}

// This class will create/process json queries and send/receive them with the help of BitcoinRpc.
class BtcJson //: IStringProcessor
{    
public:
    BtcJson();
    ~BtcJson();

    void Initialize();       // should make this part of all modules

    //void ProcessString(QSharedPointer<QByteArray>);
    bool ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue &result);
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result);

    void GetInfo();
    double GetBalance(QString account = NULL);

    QString GetAccountAddress(QString account = NULL);

    QStringList GetAddressesByAccount(QString account = "");    // list addresses for account, "" is the default account.

    QString GetNewAddress(QString account = NULL);

    QString AddMultiSigAddress(int nRequired, QJsonArray keys, QString account = NULL);    // gonna continue here at home, remove this line and it should compile.

    QStringList ListAccounts();

    QString SendToAddress(QString btcAddress, double amount);

    QSharedPointer<BtcJsonReplies::BtcTransaction> GetTransaction(QString txID);


private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
