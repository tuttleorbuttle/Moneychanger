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
        double TotalAmount;     // amount with fee
        int Confirmations;
        double Amount;          // amount without fee
        double Fee;             // I think fee is not always displayed?
        QString TxID;
        double Time;       // is this a string?
        QString Account;
        QString Address;
        QString Category;   // dafuq is this?

        BtcTransaction(QJsonObject reply)
        {
            if(!reply["error"].isNull())
            {
                SetDefaults();
                return;
            }

            this->Confirmations = (int)(reply["confirmations"].toDouble());
            this->Amount = reply["amount"].toDouble();
            this->Fee = reply["fee"].toDouble();
            this->TotalAmount = reply["amount"].toDouble(); + this->Fee;
            this->TxID = reply["txid"].toString();
            this->Time = reply["time"].toDouble();
            this->Account = reply["time"].toString();
            this->Category = reply["category"].toString();
        }

    private:
        void SetDefaults()
        {
            TotalAmount = 0.0;
            Confirmations = 0;
            Amount = 0.0;
            Fee = 0.0;
            TxID = QString();
            Time = 0;
            Account = QString();
            Address = QString();
            Category = QString();
        }
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

    bool SetTxFee(double fee);

    QSharedPointer<BtcJsonReplies::BtcTransaction> GetTransaction(QString txID);


private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
