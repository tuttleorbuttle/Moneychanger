//#include <QJsonArray>
#include "../qjsonrpc/src/json/qjsonarray.h"
//#include <QJsonDocument>
#include "../qjsonrpc/src/json/qjsondocument.h"
//#include <QJsonValue>
//#include <QJsonObject>
#include "../qjsonrpc/src/json/qjsonobject.h"
#include "btcjsonqt.h"
#include "OTLog.h"
#include <QVariant>
//#include <boost/concept_check.hpp>    // i can't remember adding this line
#include "modules.h"
#include "FastDelegate.h"   // I think we don't need these anymore because of Qt's Signals
#include "utils.h"

// https://en.bitcoin.it/wiki/Proper_Money_Handling_%28JSON-RPC%29 on how to avoid rounding errors and such. might be worth a read someday.

// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_GETNEWADDRESS        "getnewaddress"
#define METHOD_VALIDATEADDRESS      "validateaddress"
#define METHOD_DUMPPRIVKEY          "dumpprivkey"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_SENDTOADDRESS        "sendtoaddress"
#define METHOD_SENDMANY             "sendmany"
#define METHOD_SETTXFEE             "settxfee"
#define METHOD_ADDMULTISIGADDRESS   "addmultisigaddress"
#define METHOD_CREATEMULTISIG       "createmultisig"
#define METHOD_GETTRANSACTION       "gettransaction"
#define METHOD_GETRAWTRANSACTION    "getrawtransaction"
#define METHOD_DECODERAWTRANSACTION "decoderawtransaction"
#define METHOD_CREATERAWTRANSACTION "createrawtransaction"
#define METHOD_SIGNRAWTRANSACTION   "signrawtransaction"
#define METHOD_SENDRAWTRANSACTION   "sendrawtransaction"
#define METHOD_GETRAWMEMPOOL        "getrawmempool"
#define METHOD_GETBLOCKCOUNT        "getblockcount"
#define METHOD_GETBLOCKHASH         "getblockhash"
#define METHOD_GETBLOCK             "getblock"


using namespace fastdelegate;

BtcJsonQt::BtcJsonQt()
{

}

void BtcJsonQt::Initialize()
{
    // TODO: replace fastdelegate with signals and slots
    FastDelegate4<QSharedPointer<QByteArray>, QString&, QJsonValue&, QJsonValue& > reply;
    reply.bind(this, &BtcJsonQt::ProcessRpcString);
}

BtcJsonQt::~BtcJsonQt()
{
}

bool BtcJsonQt::ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue& result)
{
    QString id;
    QJsonValue error;
    ProcessRpcString(jsonString, id, error, result);

    return error.isNull();  // we should rather throw exceptions but I've never done that in c++ so this will have to do for now.
}

void BtcJsonQt::ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result)
{
    if(jsonString == NULL || jsonString->length() == 0)
        return;
    QJsonDocument replyDoc = QJsonDocument::fromJson(*jsonString);
    if(replyDoc.isNull() || replyDoc.isEmpty())
        return;

    OTLog::vOutput(0, "Received JSON:\n%s\n", QString(replyDoc.toJson()).toStdString().c_str());     // I think casting the json doc back to json adds linebreaks and stuff.

    if(replyDoc.isObject())
    {
        QJsonObject replyObj = replyDoc.object();

        QJsonValue idVal = replyObj["id"];    // this is the same ID we sent to bitcoin-qt earlier.
        id = idVal.isString() ? idVal.toString() : "";      // assign to reference parameter

        error = replyObj["error"];
        if(!error.isNull())
        {
            OTLog::vOutput(0, "Error in reply to \"%s\": %s\n\n", id.toStdString().c_str(), error.isObject() ? (error.toObject()["message"]).toString().toStdString().c_str() : "");
        }

        result = replyObj["result"];
    }
}

QByteArray BtcJsonQt::CreateJsonQuery(QString command, QJsonArray params, QString id)
{
    if(id.isEmpty())
        id = command;
    QJsonObject jsonObj = QJsonObject();
    jsonObj["jsonrpc"] = 1.0;
    jsonObj["id"] = id;
    jsonObj["method"] = command;
    jsonObj["params"] = params;
    //"{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";

    return QJsonDocument(jsonObj).toJson();
}

void BtcJsonQt::GetInfo() // if we ever need this for anything we can return a struct holding all the information
{
    QSharedPointer<QByteArray> reply = Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    QString id;
    QJsonValue error;
    QJsonValue result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    QJsonObject resultObj = result.toObject();
    int64_t balance = BtcHelper::CoinsToSatoshis(resultObj["balance"].toDouble());
}

int64_t BtcJsonQt::GetBalance(QString account/*=NULL*/ /*TODO: int minConfirmations*/)
{
    // note: json and bitcoin-qt make a difference between NULL-strings and empty strings.

    QJsonArray params = QJsonArray();
    params.append(account);      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    QSharedPointer<QByteArray> reply = Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    QJsonValue result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;  // error, TODO: throw error or return NaN
    }

    return BtcHelper::CoinsToSatoshis(result.toDouble());
}

QString BtcJsonQt::GetAccountAddress(QString account/*= NULL*/)
{
    QJsonArray params = QJsonArray();
    params.append(account);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
    {
        return NULL;     // error
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

QStringList BtcJsonQt::GetAddressesByAccount(QString account)
{
    // not yet implemented
    return QStringList();
}

QString BtcJsonQt::GetNewAddress(QString account/*=NULL*/)
{
    QJsonArray params = QJsonArray();
    params.append(account);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return NULL;
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

BtcAddressInfoQtRef BtcJsonQt::ValidateAddress(const QString& address)
{
    QJsonArray params = QJsonArray();
    params.append(address);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_VALIDATEADDRESS, params)), result))
        return BtcAddressInfoQtRef();

    if(!result.isObject())
        return BtcAddressInfoQtRef();   // shouldn't happen unless protocol is changed

    BtcAddressInfoQtRef addressInfo = BtcAddressInfoQtRef(new BtcAddressInfoQt(result.toObject()));
    return addressInfo;
}

QString BtcJsonQt::GetPublicKey(const QString& address)
{
    BtcAddressInfoQtRef addrInfo = ValidateAddress(address);
    if(addrInfo == NULL)
        return NULL;

    return addrInfo->pubkey;
}

QString BtcJsonQt::GetPrivateKey(const QString& address)
{
    return DumpPrivKey(address);
}

QString BtcJsonQt::DumpPrivKey(QString address)
{
    QJsonArray params;
    params.append(address);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_DUMPPRIVKEY, params)),
                result))
        return NULL;

    return result.toString();
}

BtcMultiSigAddressQtRef BtcJsonQt::AddMultiSigAddress(int nRequired, QStringList keys, QString account)
{
    QJsonArray params = QJsonArray();
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));
    params.append(account);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return BtcMultiSigAddressQtRef();      // error

    if(!result.isString())
        return BtcMultiSigAddressQtRef();      // shouldn't happen unless protocol is changed

    if(result.toString().isEmpty())
        return BtcMultiSigAddressQtRef();     // error, shouldn't happen at all

    // re-create the address so we get more information and return that
    return CreateMultiSigAddress(nRequired, keys);
}

BtcMultiSigAddressQtRef BtcJsonQt::CreateMultiSigAddress(int nRequired, QStringList keys)
{
    QJsonArray params = QJsonArray();
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_CREATEMULTISIG, params)),result))
        return BtcMultiSigAddressQtRef();     // error

    if(!result.isObject())
        return BtcMultiSigAddressQtRef();     // error

    BtcMultiSigAddressQtRef multiSigAddr = BtcMultiSigAddressQtRef(new BtcMultiSigAddressQt(result.toObject(), keys));
    return multiSigAddr;
}

QString BtcJsonQt::GetRedeemScript(int nRequired, QStringList keys)
{
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

QStringList BtcJsonQt::ListAccounts()
{
    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return QStringList();     // error

    if(!result.isObject())
        return QStringList();        // this shouldn't happen unless the protocol was changed

    QJsonObject accountsObj = result.toObject();
    return accountsObj.keys();      // each key is an account, each value is the account's balance
}


QString BtcJsonQt::SendToAddress(QString btcAddress, int64_t amount)
{
    // TODO: handle lack of funds, need of transaction fees and unlocking of the wallet

    QJsonArray params = QJsonArray();
    params.append(btcAddress);
    params.append(BtcHelper::SatoshisToCoins(amount));

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return NULL;   // error

    if(!result.isString())
        return NULL;    // shouldn't happen unless protocol was changed

    return result.toString();
}

bool BtcJsonQt::SetTxFee(int64_t fee)
{
    QJsonArray params = QJsonArray();
    params.append(BtcHelper::SatoshisToCoins(fee));

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SETTXFEE, params)),
                result))
        return false;

    return true;    // todo: check for more errors
}

QString BtcJsonQt::SendMany(QMap<QString, int64_t> txTargets, QString fromAccount)
{
    QJsonArray params = QJsonArray();
    params.append(fromAccount);     // account to send coins from

    QJsonObject txTargetsObj = QJsonObject();       // convert the int satoshis to double BTC
    for(QMap<QString, int64_t>::Iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i.key()] = BtcHelper::SatoshisToCoins(i.value());
    }
    params.append(txTargetsObj);    // append map of addresses and btc amounts

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SENDMANY, params)),
                result))
        return NULL;

    if(!result.isString())
        return NULL;

    return result.toString();
}

BtcTransactionQtRef BtcJsonQt::GetTransaction(QString txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    QJsonArray params = QJsonArray();
    params.append(txID);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionQtRef();    // error

    if(!result.isObject())
        return BtcTransactionQtRef();

    BtcTransactionQtRef transaction = BtcTransactionQtRef(new BtcTransactionQt(result.toObject()));
    return transaction;

    // TODO:
    // for checking balance see "details"->received, confirmations, amount (and address(es)?)

    // also check what happens in multi-sig-transactions.
}

QString BtcJsonQt::GetRawTransaction(QString txID)
{
    QJsonArray params = QJsonArray();
    params.append(txID);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return NULL;    // error

    if(!result.isString())
        return NULL;    // error

    return result.toString();
}

BtcRawTransactionQtRef BtcJsonQt::GetDecodedRawTransaction(QString txID)
{
    QJsonArray params = QJsonArray();
    params.append(txID);
    params.append(1);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return BtcRawTransactionQtRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionQtRef();  // return NULL

    BtcRawTransactionQtRef decodedRawTransaction = BtcRawTransactionQtRef(new BtcRawTransactionQt(result.toObject()));
    return decodedRawTransaction;


}

BtcRawTransactionQtRef BtcJsonQt::DecodeRawTransaction(QString rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_DECODERAWTRANSACTION, params)), result))
        return BtcRawTransactionQtRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionQtRef();  // return NULL

    BtcRawTransactionQtRef decodedRawTransaction = BtcRawTransactionQtRef(new BtcRawTransactionQt(result.toObject()));
    return decodedRawTransaction;
}

QString BtcJsonQt::CreateRawTransaction(QList<BtcOutputQt> unspentOutputs, QMap<QString, int64_t> txTargets)
{
    QJsonArray params = QJsonArray();
    QJsonArray outputsArray = QJsonArray();
    for(int i = 0; i < unspentOutputs.size(); i++)
    {
        outputsArray.append(unspentOutputs[i]);
    }
    params.append(outputsArray);

    QJsonObject txTargetsObj = QJsonObject();
    for(QMap<QString, int64_t>::Iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i.key()] = BtcHelper::SatoshisToCoins(i.value());
    }
    params.append(txTargetsObj);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_CREATERAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

BtcSignedTransactionQtRef BtcJsonQt::SignRawTransaction(QString rawTransaction, QList<BtcSigningPrequisiteQt> previousTransactions,  QStringList privateKeys)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    // add array of unspent outputs
    if(previousTransactions.size() > 0)
    {
        QJsonArray unspentArray = QJsonArray();
        for(int i = 0; i < previousTransactions.size(); i++)
        {
            unspentArray.append(previousTransactions[i]);
        }
        params.append((unspentArray));
    }
    else
    {
        params.append(QJsonValue());            // append null or else it won't work
    }

    // add array of private keys used to sign the transaction
    if(privateKeys.size() > 0)
    {
        QJsonArray privKeysArray;               // TODO: figure out how to properly parse a string list
        for(int i = 0; i < privateKeys.size(); i++)
        {
            privKeysArray.append(privateKeys[i]);
        }
        params.append(privKeysArray);
    }
    else
        params.append(QJsonValue());            // if no private keys were given, append null


    // params.append(ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY)

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionQtRef();   // error

    if(!result.isObject())
        return BtcSignedTransactionQtRef();   // error

    BtcSignedTransactionQtRef signedTransaction = BtcSignedTransactionQtRef(new BtcSignedTransactionQt(result.toObject()));
    return signedTransaction;
}

BtcSignedTransactionQtRef BtcJsonQt::CombineSignedTransactions(QString rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);  // a concatenation of partially signed tx's
    params.append(QJsonArray());    // dummy inputs and redeemscripts (must not be 'null')
    params.append(QJsonArray());    // dummy private keys (must not be 'null')
    // if dummy inputs are 'null', bitcoin will not just combine the raw transaction but do additional signing(!!!)

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionQtRef();   // error

    BtcSignedTransactionQtRef signedTransaction = BtcSignedTransactionQtRef(new BtcSignedTransactionQt(result.toObject()));
    return signedTransaction;
}

QString BtcJsonQt::SendRawTransaction(QString rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

QList<QString> BtcJsonQt::GetRawMemPool()
{
    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWMEMPOOL, QJsonArray())), result))
        return QList<QString>();

    if(!result.isArray())
        return QList<QString>();

    QJsonArray resultArray = result.toArray();
    QList<QString> rawMemPool = QList<QString>();
    for(int i = 0; i < resultArray.size(); i++)
    {
        rawMemPool += resultArray[i].toString();
    }
    return rawMemPool;
}

int BtcJsonQt::GetBlockCount()
{
    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKCOUNT, QJsonArray())), result))
        return -1;   // we should throw errors instead.

    return (int)result.toDouble(-1);
}

QString BtcJsonQt::GetBlockHash(int blockNumber)
{
    QJsonArray params = QJsonArray();
    params.append(blockNumber);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return NULL;

    return result.toString();
}

BtcBlockQtRef BtcJsonQt::GetBlock(QString blockHash)
{
    QJsonArray params = QJsonArray();
    params.append(blockHash);

    QJsonValue result = QJsonValue();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCK, params)), result))
        return BtcBlockQtRef();

    if(!result.isObject())
        return BtcBlockQtRef();

    BtcBlockQtRef block = BtcBlockQtRef(new BtcBlockQt(result.toObject()));
    return block;
}









