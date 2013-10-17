#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "btcrpc.h"


//
// the actual BtcJson class is below this namespace
//
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
        QString txID = NULL;

        struct VIN
        {
            QString txInID = NULL;
            int vout = 0;   // number of txInID's output to be used as input

            VIN(QString txInID, int vout)
                :txInID(txInID), vout(vout)
            {}
        };
        QList<VIN> inputs = QList<VIN>();

        struct VOUT
        {
            double value = 0.0;     // amount of btc to be sent
            int n = 0;              // outputs array index
            int reqSigs = 0;        // signatures required to withdraw from the output addresses
            QList<QString> addresses = QList<QString>();    // an array of addresses receiving the value.

            VOUT()
            {}

            VOUT(double value, int n, int reqSigs, QList<QString> addresses)
                :value(value), n(n), reqSigs(reqSigs), addresses(addresses)
            {}

        };
        QList<VOUT> outputs = QList<VOUT>();

        BtcRawTransaction(QJsonObject rawTx)
        {
            this->txID = rawTx["txid"].toString();

            QJsonArray vin = rawTx["vin"].toArray();
            for(int i = 0; i < vin.size(); i++)
            {
                QJsonObject inputObj = vin[i].toObject();
                this->inputs += VIN(inputObj["txid"].toString(), (int)inputObj["vout"].toDouble());
            }

            QJsonArray vout  = rawTx["vout"].toArray();
            for(int i = 0; i < vout.size(); i++)
            {
                QJsonObject outputObj = vout[i].toObject();
                VOUT output;

                output.value = outputObj["value"].toDouble();
                output.n = (int)outputObj["n"].toDouble();      // JSON doesn't know integers

                QJsonObject scriptPubKey = outputObj["scriptPubKey"].toObject();
                output.reqSigs = (int)scriptPubKey["reqSigs"].toDouble();
                QJsonArray addresses = scriptPubKey["addresses"].toArray();
                for(int i= 0; i < addresses.size(); i++)
                {
                    output.addresses += addresses[i].toString();
                }

                this->outputs += output;
            }

        }
    };

    struct BtcAddressInfo
    {
        QString address = NULL;
        QString pubkey = NULL;
        bool ismine = false;
        bool isvalid = false;

        BtcAddressInfo(QJsonObject result)
        {
            // it seems we don't need to do any error checking. thanks, qt.
            this->address = result["address"].toString();
            this->pubkey = result["pubkey"].toString();
            this->ismine = result["ismine"].toBool();
            this->isvalid = result["isvalid"].toBool();
        }
    };

    struct BtcAddressAmount
    {
        QString Address;
        double Amount;
    };
}

using namespace BtcJsonObjects;

typedef QSharedPointer<BtcRawTransaction> BtcRawTransactionRef;

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

    // Validate an address
    QSharedPointer<BtcAddressInfo> ValidateAddress(QString address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // Returns the multi-sig address
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

    QString GetRawTransaction(QString txID);

    BtcRawTransactionRef GetDecodedRawTransaction(QString txID);

    BtcRawTransactionRef DecodeRawTransaction(QString rawTransaction);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
