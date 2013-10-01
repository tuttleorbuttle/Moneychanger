#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "btcrpc.h"


namespace BtcJsonObjects
{
    // if a query returns a complex object we should convert it to a struct instead of
    // passing around the QJsonObject

    struct BtcTransaction
    {
        //double TotalAmount;     // amount with fee but we don't need that
        int Confirmations;
        double Amount;          // amount without fee
        double Fee;             // only displayed when sender
        QString TxID;
        double Time;       // is this a string?
        QString Account;
        QString Address;    // recipient address. how do we find sender? also this should be an array I think because details[] is one.
        QString Category;   // "send", "receive", "immature" (unconfirmed block reward), ...?

        BtcTransaction(QJsonObject reply)
        {
            if(!reply["error"].isNull())
            {
                SetDefaults();
                return;
            }

            this->Confirmations = (int)(reply["confirmations"].toDouble());
            this->Amount = reply["amount"].toDouble();  // is negative when sending and zero if block reward or if you are sender AND receiver
                                                        // correct amounts can be found in "details"[]
            this->Fee = reply["fee"].toDouble();
            //this->TotalAmount = reply["amount"].toDouble(); + this->Fee;  not needed and doesn't work like that anyway
            this->TxID = reply["txid"].toString();
            this->Time = reply["time"].toDouble();

            if(!reply["details"].isArray())
                return;
            QJsonArray details = reply["details"].toArray();
            if(details.size() == 0)
                return;
            QJsonObject detail = details[0].toObject();
            // TODO: the entire array.
            // what if a transaction sends to multiple addresses at once but only one is ours?

            this->Address = detail["address"].toString();
            this->Category = detail["category"].toString();
            this->Account = detail["account"].toString();

            /*
             * sample gettransaction response
             * I think "details" contains additional information if you were sender or receiver.
             * TODO: test what happens when you are neither sender nor receiver of txID
             * Answer: it errors
        {
            "error": null,
            "id": "gettransaction",
            "result": {
                "amount": 0,
                "confirmations": 0,
                "details": [
                    {
                        "account": "",
                        "address": "mggsAaus69pH46TDWFA3oyL1vg1v1UNivs",
                        "amount": -1.23457,
                        "category": "send",
                        "fee": 0
                    },
                    {
                        "account": "",
                        "address": "mggsAaus69pH46TDWFA3oyL1vg1v1UNivs",
                        "amount": 1.23457,
                        "category": "receive"
                    }
                ],
                "fee": 0,
                "time": 1.38021e+09,
                "timereceived": 1.38021e+09,
                "txid": "fe6e73e6ab672f8029e98e9277a6589c1140c569d1169a71fbd0a275c51619e6"
            }
        }

            *
            * Sample gettransaction response for a block reward
            *
        {
            "amount" : 0.00000000,
            "confirmations" : 1,
            "generated" : true,
            "blockhash" : "00000000d66484041379ba97f34e9f9c3224f733b90e8201e470489ebad79e35",
            "blockindex" : 0,
            "blocktime" : 1380338924,
            "txid" : "db74b6c201be97a3d3e809c27fec6be8bdd86376f7d0cacb584f68006f525fd2",
            "time" : 1380338924,
            "timereceived" : 1380338925,
            "details" : [
                {
                    "account" : "",
                    "address" : "mvThPUNTF6RxUYu6A32LwCeXMYqkeWHQMZ",
                    "category" : "immature",
                    "amount" : 80.50000000
                }
            ]
        }

             *
            */
        }

    private:
        void SetDefaults()
        {
            //TotalAmount = 0.0;
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

    struct BtcRawTransaction
    {
        BtcRawTransaction(QJsonObject rawTx)
        {

        }
    };

    struct BtcAddressAmount
    {
        QString Address;
        double Amount;
    };
}

using namespace BtcJsonObjects;

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
    
    QString SendMany(QList<BtcAddressAmount> txTargets, QString fromAccount = NULL);

    bool SetTxFee(double fee);

    QSharedPointer<BtcJsonObjects::BtcTransaction> GetTransaction(QString txID);


private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
