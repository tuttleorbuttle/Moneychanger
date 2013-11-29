#include <QJsonArray>
#include <QJsonDocument>
//#include <QJsonValue>
#include <QJsonObject>
#include "btcjson.h"
#include "OTLog.h"
#include <QVariant>
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

BtcJson::BtcJson()
{

}

void BtcJson::Initialize()
{
    FastDelegate4<QSharedPointer<QByteArray>, QString&, QJsonValue&, QJsonValue& > reply;
    reply.bind(this, &BtcJson::ProcessRpcString);
}

BtcJson::~BtcJson()
{
}

int64_t BtcJson::DoubleToInt(double value)
{
    // copied from the wiki
    return (int64_t)(value * 1e8 + (value < 0.0 ? -.5 : .5));
}

double BtcJson::IntToDouble(int64_t value)
{
    return (double)value / 1e8;
}

bool BtcJson::ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue& result)
{
    QString id;
    QJsonValue error;
    ProcessRpcString(jsonString, id, error, result);

    return error.isNull();  // we should rather throw exceptions but I've never done that in c++ so this will have to do for now.
}

void BtcJson::ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result)
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

QByteArray BtcJson::CreateJsonQuery(QString command, QJsonArray params, QString id)
{
    if(id == "")
        id = command;
    QJsonObject jsonObj;
    jsonObj.insert("jsonrpc", 1.0);
    jsonObj.insert("id", id);
    jsonObj.insert("method", command);
    jsonObj.insert("params", params);
    //"{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";

    return QJsonDocument(jsonObj).toJson();
}

void BtcJson::GetInfo() // if we ever need this for anything we can return a struct holding all the information
{
    QSharedPointer<QByteArray> reply = Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    QString id;
    QJsonValue error;
    QJsonValue result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    QJsonObject resultObj = result.toObject();
    double balance = resultObj["balance"].toDouble();
}

int64_t BtcJson::GetBalance(QString account/*=NULL*/)
{
    QJsonArray params;
    params.append(account);      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    QSharedPointer<QByteArray> reply = Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    QJsonValue result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return -999;  // error, TODO: throw error or return NaN but not -999
    }

    return result.toDouble();
}

QString BtcJson::GetAccountAddress(QString account/*= NULL*/)
{
    QJsonArray params;
    params.append(account);

    QJsonValue result;
    if(!ProcessRpcString(Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
    {
        return NULL;     // error
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

QStringList BtcJson::GetAddressesByAccount(QString account)
{
    return QStringList();
}

QString BtcJson::GetNewAddress(QString account/*=NULL*/)
{
    QJsonArray params;
    params.append(account);

    QJsonValue result;
    if(!ProcessRpcString(Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return NULL;
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

BtcAddressInfoRef BtcJson::ValidateAddress(const QString& address)
{
    // this function hasn't been tested yet and might not work.

    QJsonArray params;
    params.append(address);

    QJsonValue result;
    if(!ProcessRpcString(Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_VALIDATEADDRESS, params)), result))
        return BtcAddressInfoRef();

    if(!result.isObject())
        return BtcAddressInfoRef();   // shouldn't happen unless protocol is changed

    BtcAddressInfoRef addressInfo;
    addressInfo.reset(new BtcAddressInfo(result.toObject()));
    return addressInfo;
}

QString BtcJson::GetPrivateKey(QString address)
{
    return DumpPrivKey(address);
}

QString BtcJson::DumpPrivKey(QString address)
{
    QJsonArray params;
    params.append(address);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DUMPPRIVKEY, params)),
                result))
        return NULL;

    return result.toString();
}

BtcMultiSigAddressRef BtcJson::AddMultiSigAddress(int nRequired, QStringList keys, QString account)
{
    QJsonArray params;
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));
    params.append(account);

    QJsonValue result;
    if(!ProcessRpcString(Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return BtcMultiSigAddressRef();      // error

    if(!result.isString())
        return BtcMultiSigAddressRef();      // shouldn't happen unless protocol is changed

    if(result.toString().isEmpty())
        return BtcMultiSigAddressRef();     // error, shouldn't happen at all

    // re-create the address so we get more information and return that
    return CreateMultiSigAddress(nRequired, keys);
}

BtcMultiSigAddressRef BtcJson::CreateMultiSigAddress(int nRequired, QStringList keys)
{
    QJsonArray params;
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_CREATEMULTISIG, params)),result))
        return BtcMultiSigAddressRef();     // error

    if(!result.isObject())
        return BtcMultiSigAddressRef();     // error

    BtcMultiSigAddressRef multiSigAddr;
    multiSigAddr.reset(new BtcMultiSigAddress(result.toObject(), keys));
    return multiSigAddr;
}

QString BtcJson::GetRedeemScript(int nRequired, QStringList keys)
{
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

QStringList BtcJson::ListAccounts()
{
    QJsonValue result;
    if(!ProcessRpcString(Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return QStringList();     // error

    if(!result.isObject())
        return QStringList();        // this shouldn't happen unless the protocol was changed

    QJsonObject accountsObj = result.toObject();
    return accountsObj.keys();      // each key is an account, each value is the account's balance
}


QString BtcJson::SendToAddress(QString btcAddress, int64_t amount)
{
    // TODO: handle lack of funds, need of transaction fees and unlocking of the wallet

    QJsonArray params;
    params.append(btcAddress);
    params.append(IntToDouble(amount));

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return NULL;   // error

    if(!result.isString())
        return NULL;    // shouldn't happen unless protocol was changed

    return result.toString();
}

bool BtcJson::SetTxFee(int64_t fee)
{
    QJsonArray params;
    params.append(IntToDouble(fee));

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SETTXFEE, params)),
                result))
        return false;

    return true;    // todo: check for more errors
}

QString BtcJson::SendMany(QVariantMap txTargets, QString fromAccount)
{
    QJsonArray params;
    params.append(fromAccount);
    params.append(QJsonObject::fromVariantMap(txTargets));

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDMANY, params)),
                result))
        return false;

    if(!result.isString())
        return NULL;

    return result.toString();
}

BtcTransactionRef BtcJson::GetTransaction(QString txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    QJsonArray params;
    params.append(txID);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionRef();    // error

    if(!result.isObject())
        return BtcTransactionRef();

    BtcTransactionRef transaction;
    transaction.reset(new BtcTransaction(result.toObject()));
    return transaction;

    // TODO:
    // for checking balance see "details"->received, confirmations, amount (and address(es)?)

    // also check what happens in multi-sig-transactions.
}

QString BtcJson::GetRawTransaction(QString txID)
{
    QJsonArray params;
    params.append(txID);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return NULL;    // error

    if(!result.isString())
        return NULL;    // error

    return result.toString();
}

BtcRawTransactionRef BtcJson::GetDecodedRawTransaction(QString txID)
{
    QJsonArray params;
    params.append(txID);
    params.append(1);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction;
    decodedRawTransaction.reset(new BtcRawTransaction(result.toObject()));
    return decodedRawTransaction;


}

BtcRawTransactionRef BtcJson::DecodeRawTransaction(QString rawTransaction)
{
    QJsonArray params;
    params.append(rawTransaction);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DECODERAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction;
    decodedRawTransaction.reset(new BtcRawTransaction(result.toObject()));
    return decodedRawTransaction;
}

QString BtcJson::CreateRawTransaction(QList<BtcOutput> unspentOutputs, QVariantMap txTargets)
{
    QJsonArray params;
    QJsonArray outputsArray;
    for(int i = 0; i < unspentOutputs.size(); i++)
    {
        outputsArray.append(unspentOutputs[i]);
    }
    params.append(outputsArray);
    params.append(QJsonObject::fromVariantMap(txTargets));

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_CREATERAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

BtcSignedTransactionRef BtcJson::SignRawTransaction(QString rawTransaction, QList<BtcSigningPrequisite> previousTransactions,  QStringList privateKeys)
{
    QJsonArray params;
    params.append(rawTransaction);

    QJsonArray unspentArray;
    if(previousTransactions.size() > 0)
    {
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

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    if(!result.isObject())
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction;
    signedTransaction.reset(new BtcSignedTransaction(result.toObject()));
    return signedTransaction;
}

BtcSignedTransactionRef BtcJson::CombineSignedTransactions(QString rawTransaction)
{
    QJsonArray params;
    params.append(rawTransaction);  // a concatenation of partially signed tx's
    params.append(QJsonArray());      // dummy (must not be 'null')
    params.append(QJsonArray());      // dummy (must not be 'null')

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction;
    signedTransaction.reset(new BtcSignedTransaction(result.toObject()));
    return signedTransaction;
}

QString BtcJson::SendRawTransaction(QString rawTransaction)
{
    QJsonArray params;
    params.append(rawTransaction);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

QList<QString> BtcJson::GetRawMemPool()
{
    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWMEMPOOL, QJsonArray())), result))
        return QList<QString>();

    if(!result.isArray())
        return QList<QString>();

    QJsonArray resultArray = result.toArray();
    QList<QString> rawMemPool;
    for(int i = 0; i < resultArray.size(); i++)
    {
        rawMemPool += resultArray[i].toString();
    }
    return rawMemPool;
}

int BtcJson::GetBlockCount()
{
    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKCOUNT, QJsonArray())), result))
        return -1;   // we should throw errors instead.

    return (int)result.toDouble(-1);
}

QString BtcJson::GetBlockHash(int blockNumber)
{
    QJsonArray params;
    params.append(blockNumber);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return NULL;

    return result.toString();
}

BtcBlockRef BtcJson::GetBlock(QString blockHash)
{
    QJsonArray params;
    params.append(blockHash);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCK, params)), result))
        return BtcBlockRef();

    if(!result.isObject())
        return BtcBlockRef();

    BtcBlockRef block;
    block.reset(new BtcBlock(result.toObject()));
    return block;
}








