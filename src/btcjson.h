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
            QString scriptPubKeyHex = NULL;

            VOUT()
            {}

            VOUT(double value, int n, int reqSigs, QList<QString> addresses, QString scriptPubKeyHex)
                :value(value), n(n), reqSigs(reqSigs), addresses(addresses), scriptPubKeyHex(scriptPubKeyHex)
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
                output.scriptPubKeyHex = scriptPubKey["hex"].toString();

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

    struct BtcBlock
    {
        int confirmations = 0;
        QList<QString> transactions = QList<QString>();
        int height = 0;
        QString hash = "";
        QString previousHash = "";

        BtcBlock()
        {}

        BtcBlock(QJsonObject block)
        {
            // latest block has 1 confirmations I think so tx->confirms == block->confirms
            this->confirmations = (int)block["confirmations"].toDouble();

            // block number (count since genesis)
            this->height = (int)block["height"].toDouble();

            this->hash = block["hash"].toString();
            this->previousHash = block["previousblockhash"].toString();

            // get list of transactions in the block
            QJsonArray transacts = block["tx"].toArray();
            for(int i = 0; i < transacts.size(); i++)
            {
                this->transactions += transacts[i].toString();
            }
        }
    };

    struct BtcOutput : QJsonObject
    {
        BtcOutput(QString txID, int vout)
        {
            (*this)["txid"] = txID;
            (*this)["vout"] = vout;
        }
    };

    struct BtcSignedTransaction
    {
        QString signedTransaction = "";
        bool complete = false;              // true if all (enough?) signatures were collected

        BtcSignedTransaction(QJsonObject signedTransactionObj)
        {
            this->signedTransaction = signedTransactionObj["hex"].toString();
            this->complete = signedTransactionObj["complete"].toBool();
        }
    };

    // used to sign some raw transactions
    // to withdraw from p2sh addresses e.g. you need to know those things
    struct BtcSigningPrequisite : QJsonObject
    {
        BtcSigningPrequisite()
        {}

        BtcSigningPrequisite(QString txId, int vout, QString scriptPubKey, QString redeemScript)
        {
            // all of these values must be set or else prequisite is invalid
            (*this)["txid"] = txId;
            (*this)["vout"] = vout;
            (*this)["scriptPubKey"] = scriptPubKey;
            (*this)["redeemScript"] = redeemScript;
        }

        // all of these values must be set or else prequisite is invalid
        void SetTxId(QString txId)
        {
            // we can get this value from the transaction used to send funds to the p2sh
            (*this)["txid"] = txId;
        }

        // all of these values must be set or else prequisite is invalid
        void SetVout(int vout)
        {
            // we can get this value from the transaction used to send funds to the p2sh
            (*this)["vout"] = vout;
        }

        // all of these values must be set or else prequisite is invalid
        void SetScriptPubKey(QString scriptPubKey)
        {
            // we can get this value from the transaction used to send funds to the p2sh
            (*this)["scriptPubKey"] = scriptPubKey;
        }

        // all of these values must be set or else prequisite is invalid
        void SetRedeemScript(QString redeemScript)
        {
            // we can get this from the createmultisig api function
            (*this)["redeemScript"] = redeemScript;
        }
    };
}

using namespace BtcJsonObjects;

typedef QSharedPointer<BtcTransaction> BtcTransactionRef;
typedef QSharedPointer<BtcRawTransaction> BtcRawTransactionRef;
typedef QSharedPointer<BtcBlock> BtcBlockRef;
typedef QSharedPointer<BtcOutput> BtcOutputRef;
typedef QSharedPointer<BtcSignedTransaction> BtcSignedTransactionRef;
typedef QSharedPointer<BtcSigningPrequisite> BtcSigningPrequisiteRef;

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

    // Get private key for address (calls DumpPrivKey())
    QString GetPrivateKey(QString address);

    // Get private key for address
    QString DumpPrivKey(QString address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // Returns the multi-sig address
    QString AddMultiSigAddress(int nRequired, QStringList keys, QString account = NULL);    // gonna continue here at home, remove this line and it should compile.

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    QString GetRedeemScript(int nRequired, QStringList keys);

    // Returns list of account names
    // Could also return the balance of each account but I find that confusing
    QStringList ListAccounts();

    QString SendToAddress(QString btcAddress, double amount);
    
    // Send to multiple addresses at once
    // txTargets maps amounts (double) to addresses (QString)
    QString SendMany(QVariantMap txTargets, QString fromAccount = NULL);

    bool SetTxFee(double fee);

    BtcTransactionRef GetTransaction(QString txID);

    QString GetRawTransaction(QString txID);

    BtcRawTransactionRef GetDecodedRawTransaction(QString txID);

    BtcRawTransactionRef DecodeRawTransaction(QString rawTransaction);

    QString CreateRawTransaction(QList<BtcOutput> unspentOutputs, QVariantMap txTargets);

    BtcSignedTransactionRef SignRawTransaction(QString rawTransaction, QList<BtcSigningPrequisite> previousTransactions = QList<BtcSigningPrequisite>(), QStringList privateKeys = QStringList());

    BtcSignedTransactionRef CombineSignedTransactions(QString rawTransaction);

    QString SendRawTransaction(QString rawTransaction);

    QList<QString> GetRawMemPool();

    int GetBlockCount();

    QString GetBlockHash(int blockNumber);

    BtcBlockRef GetBlock(QString blockHash);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
