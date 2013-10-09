#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "btcrpc.h"


namespace BtcJsonObjects
{
    // if a query returns a complex object we should convert it to a struct instead of
    // passing around the QJsonObject

    // This struct holds various information about a bitcoin transaction.
    // Its constructor accepts the QJsonObject returned by "gettransaction".
    // Does not take into account block rewards.
    struct BtcTransaction
    {
        //double TotalAmount;   // amount with fee but we don't need that
        int Confirmations;
        double AmountReceived;  // amount received
        double AmountSent;      // amount sent
        double Amount;          // +received -sent
        double Fee;             // only displayed when sender
        QString TxID;
        double Time;
        //QString Account;
        QList<QString> AddressesRecv;   // received to addresses
        QList<QString> AddressesSent;   // sent to addresses
        //QString Category;           // "send", "receive", "immature" (unconfirmed block reward), ...?

        BtcTransaction(QJsonObject reply)
        {
            SetDefaults();

            if(!reply["error"].isNull())
                return;

            this->Confirmations = (int)(reply["confirmations"].toDouble());
            this->Amount = reply["amount"].toDouble();

            this->Fee = reply["fee"].toDouble();
            this->TxID = reply["txid"].toString();
            this->Time = reply["time"].toDouble();

            // details
            if(!reply["details"].isArray())
                return;
            QJsonArray details = reply["details"].toArray();
            if(details.size() == 0)
                return;

            for(int i = 0; i < details.size(); i++)
            {
                QJsonObject detail = details[i].toObject();
                QString address = detail["address"].toString();
                QString category = detail["category"].toString();
                double amount = detail["amount"].toDouble();
                if(category == "send")
                {
                    this->AddressesSent += address;
                    this->AmountSent += amount; // will be 0 or less
                }
                else if(category == "receive")
                {
                    this->AddressesRecv += address;
                    this->AmountReceived += amount; // will be 0 or more
                }
                else if(category == "immature")
                {
                }
            }
        }

    private:
        void SetDefaults()
        {
            //TotalAmount = 0.0;
            this->Confirmations = 0;
            this->AmountReceived = 0.0;
            this->AmountSent = 0.0;
            this->Amount = 0.0;
            this->Fee = 0.0;
            this->TxID = QString();
            this->Time = 0;
            this->AddressesRecv = QList<QString>();
            this->AddressesSent = QList<QString>();
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

// This class will create/process json queries and send/receive them with the help of BitcoinRpc
class BtcJson //: IStringProcessor
{    
public:
    BtcJson();
    ~BtcJson();

    void Initialize();       // should make this part of all modules

    // Checks the reply object received from bitcoin-qt for errors and returns the reply
    bool ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue &result);
    // Splits the reply object received from bitcoin-qt into error and result objects
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result);

    void GetInfo();
    double GetBalance(QString account = NULL);

    // Gets the default address for the specified account
    QString GetAccountAddress(QString account = NULL);

    // Returns list of all addresses belonging to account
    QStringList GetAddressesByAccount(QString account = "");    // list addresses for account, "" is the default account.

    // Add new address to account
    QString GetNewAddress(QString account = NULL);

    // Adds an address requiring n keys to sign before it can spend its inputs
    QString AddMultiSigAddress(int nRequired, QJsonArray keys, QString account = NULL);    // gonna continue here at home, remove this line and it should compile.

    // Returns list of account names
    // Could also return the balance of each account but I find that confusing
    QStringList ListAccounts();

    QString SendToAddress(QString btcAddress, double amount);
    
    // Send to multiple addresses at once
    // txOutput maps amounts (double) to addresses (QString)
    QString SendMany(QVariantMap txOutputs, QString fromAccount = NULL);

    bool SetTxFee(double fee);

    QSharedPointer<BtcJsonObjects::BtcTransaction> GetTransaction(QString txID);


private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
