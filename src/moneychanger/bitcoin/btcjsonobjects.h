#ifndef BTCJSONOBJECTS_H
#define BTCJSONOBJECTS_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QSharedPointer>
#include "utils.h"


// if a query returns a complex object we should convert it to a struct instead of
// passing around the QJsonObject

// This struct holds various information about a bitcoin transaction.
// Its constructor accepts the QJsonObject returned by "gettransaction".
// Does not take into account block rewards.
struct BtcTransaction
{
    int Confirmations;
    // all amounts are satoshis
    int64_t AmountReceived;  // amount received
    int64_t AmountSent;      // amount sent
    int64_t Amount;          // +received -sent
    int64_t Fee;             // only displayed when sender
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
        this->Amount = utils::CoinsToSatoshis(reply["amount"].toDouble());

        this->Fee = utils::CoinsToSatoshis(reply["fee"].toDouble());
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
            int64_t amount = utils::CoinsToSatoshis(detail["amount"].toDouble());
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
                // that's block reward.
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
    QString txID;

    struct VIN
    {
        QString txInID;
        int vout;   // number of txInID's output to be used as input

        VIN(QString txInID, int vout)
            :txInID(txInID), vout(vout)
        {}
    };
    QList<VIN> inputs;

    struct VOUT
    {
        int64_t value;     // amount of btc to be sent
        int n ;              // outputs array index
        int reqSigs;        // signatures required to withdraw from the output addresses
        QList<QString> addresses;    // an array of addresses receiving the value.
        QString scriptPubKeyHex;

        VOUT()
        {
            this->value = 0;
            this->n = -1;
            this->reqSigs = 0;
            this->addresses = QList<QString>();
            this->scriptPubKeyHex = "";
        }

        VOUT(int64_t value, int n, int reqSigs, QList<QString> addresses, QString scriptPubKeyHex)
            :value(value), n(n), reqSigs(reqSigs), addresses(addresses), scriptPubKeyHex(scriptPubKeyHex)
        {}

    };
    QList<VOUT> outputs;

    BtcRawTransaction(QJsonObject rawTx)
    {
        this->inputs = QList<VIN>();
        this->outputs = QList<VOUT>();

        this->txID = rawTx["txid"].toString();

        QJsonArray vin = rawTx["vin"].toArray();
        for(int i = 0; i < vin.size(); i++)
        {
            QJsonObject inputObj = vin[i].toObject();
            this->inputs += VIN(inputObj["txid"].toString(), (int)inputObj["vout"].toDouble());
        }

        QJsonArray vouts  = rawTx["vout"].toArray();
        for(int i = 0; i < vouts.size(); i++)
        {
            QJsonObject outputObj = vouts[i].toObject();
            VOUT output;

            output.value = utils::CoinsToSatoshis(outputObj["value"].toDouble());
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
    QString address;
    QString pubkey;
    QString account;
    bool ismine;
    bool isvalid;
    bool isScript;
    QJsonArray addresses;       // shows addresses which a multi-sig is composed of
    int sigsRequired = 0;

    BtcAddressInfo(QJsonObject result)
    {
        // it seems we don't need to do any error checking. thanks, qt.
        this->address = result["address"].toString();   // if wrong type, default value will be returned
        this->pubkey = result["pubkey"].toString();
        this->account = result["account"].toString();
        this->ismine = result["ismine"].toBool();
        this->isvalid = result["isvalid"].toBool();

        // multi-sig properties:
        this->isScript = result["isscript"].toBool();
        this->addresses = result["addresses"].toArray();
    }
};

struct BtcAddressAmount
{
    QString Address;
    double Amount;
};

struct BtcMultiSigAddress
{
    QString address;
    QString redeemScript;
    QStringList publicKeys;     // this will make everything so much more convenient

    BtcMultiSigAddress(QJsonObject result, const QStringList& publicKeys)
    {
        this->address = result["address"].toString();
        this->redeemScript = result["redeemScript"].toString();

        this->publicKeys = publicKeys;
    }
};

struct BtcBlock
{
    int confirmations;
    QList<QString> transactions;
    int height;
    QString hash;
    QString previousHash;

    BtcBlock()
    {
        this->confirmations = 0;
        this->transactions = QList<QString>();
        this->height = 0;
        this->hash = QString();
        this->previousHash = QString();
    }

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
    QString signedTransaction;
    bool complete;              // true if all (enough?) signatures were collected

    BtcSignedTransaction(QJsonObject signedTransactionObj)
    {
        this->signedTransaction = signedTransactionObj["hex"].toString();
        this->complete = signedTransactionObj["complete"].toBool();
    }
};

// used to sign some raw transactions
struct BtcSigningPrequisite : QJsonObject
{
    // an array of BtcSigningPrequisites can be passed to bitcoin-qt as an optional argument to signrawtransaction
    // [{"txid":txid,"vout":n,"scriptPubKey":hex,"redeemScript":hex},...]
    // this is required if we
    //      a) didn't add the address to our wallet (createmultisig instead of addmultisigaddress)
    //      b) want to sign a raw tx only with a particular address's private key (the privkey is passed in another argument)

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

typedef QSharedPointer<BtcTransaction> BtcTransactionRef;
typedef QSharedPointer<BtcRawTransaction> BtcRawTransactionRef;
typedef QSharedPointer<BtcAddressInfo> BtcAddressInfoRef;
typedef QSharedPointer<BtcMultiSigAddress> BtcMultiSigAddressRef;
typedef QSharedPointer<BtcBlock> BtcBlockRef;
typedef QSharedPointer<BtcOutput> BtcOutputRef;
typedef QSharedPointer<BtcSignedTransaction> BtcSignedTransactionRef;
typedef QSharedPointer<BtcSigningPrequisite> BtcSigningPrequisiteRef;


#endif // BTCJSONOBJECTS_H
