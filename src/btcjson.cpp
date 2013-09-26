#include <QJsonArray>
#include <QJsonDocument>
//#include <QJsonValue>
#include <QJsonObject>
#include "btcjson.h"
#include "OTLog.h"
#include <QVariant>
#include "modules.h"
#include "FastDelegate.h"
#include "utils.h"

// https://en.bitcoin.it/wiki/Proper_Money_Handling_%28JSON-RPC%29 on how to avoid rounding errors and such. might be worth a read someday.

// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_GETNEWADDRESS        "getnewaddress"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_SENDTOADDRESS        "sendtoaddress"
#define METHOD_ADDMULTISIGADDRESS   "addmultisigaddress"
#define METHOD_GETTRANSACTION       "gettransaction"


using namespace fastdelegate;
using namespace BtcJsonReplies;

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
    QSharedPointer<QByteArray> reply = Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    QString id;
    QJsonValue error;
    QJsonValue result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    QJsonObject resultObj = result.toObject();
    double balance = resultObj["balance"].toDouble();
}

double BtcJson::GetBalance(QString account/*=NULL*/)
{
    QJsonArray params;
    params.append(account);      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    QSharedPointer<QByteArray> reply = Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

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
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
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
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return NULL;
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

QString BtcJson::AddMultiSigAddress(int nRequired, QJsonArray keys, QString account)
{
    QJsonArray params;
    params.append(nRequired);
    params.append(keys);
    params.append(account);

    QJsonValue result;
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return NULL;      // error

    if(!result.isString())
        return NULL;      // shouldn't happen unless protocol is changed

    return result.toString();
}

QStringList BtcJson::ListAccounts()
{
    QJsonValue result;
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return QStringList();     // error

    if(!result.isObject())
        return QStringList();        // this shouldn't happen unless the protocol was changed

    QJsonObject accountsObj = result.toObject();
    return accountsObj.keys();      // each key is an account, each value is the account's balance
}


QString BtcJson::SendToAddress(QString btcAddress, double amount)
{
    QJsonArray params;
    params.append(btcAddress);
    params.append(amount);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return NULL;   // error

    if(!result.isString())
        return NULL;    // shouldn't happen unless protocol was changed

    return result.toString();
}

QSharedPointer<BtcTransaction> BtcJson::GetTransaction(QString txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    QJsonArray params;
    params.append(txID);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::bitcoinRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return QSharedPointer<BtcTransaction>();    // error

    if(!result.isObject())
        return QSharedPointer<BtcTransaction>();

    //TODO: Finish implementation of BtcTransaction and of this function
    return QSharedPointer<BtcTransaction>();
    QSharedPointer<BtcTransaction> transaction;
    transaction.reset(new BtcTransaction(result.toObject()));
    return transaction;

    /*
     * sample gettransaction response
     * I think "details" contains additional information if you were sender or receiver.
     * TODO: test what happens when you are neither sender nor receiver of txID
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
    */
}












