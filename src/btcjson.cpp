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
    if(!ProcessRpcString(reply, result))
    {
        return 0.0;  // error
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
        return;     // error
    }

    if(!result.isString())
        return;     // this should never happen unless the protocol was changed

    return result.toString();
}

QString Bitcoin::GetNewAddress(QString account/*=NULL*/)
{
    QJsonArray params;
    params.append(account);

    QJsonValue result;
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return;
    }

    if(!result.isString())
        return;     // this should never happen unless the protocol was changed

    return result.toString();
}

void BtcJson::ListAccounts()
{
    QJsonValue reply;
    if(!ProcessRpcString(Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), reply))
    {
        return;
    }

    // TODO: something useful with the result
}

bool BtcJson::SendToAddress(QString btcAddress, double amount)
{
    QJsonArray params;
    params.append(btcAddress);
    params.append(amount);

    QJsonValue result;
    if(!ProcessRpcString(
                Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
    {
        return false;   // error
    }

    int a = 0;
}


void BtcJson::OnGetInfo(QJsonValue result, QJsonValue error)
{

}

void BtcJson::OnGetBalance(QJsonValue result)
{
    if(!result.isDouble())
        return;

    double balance = (float)result.toDouble();
    OTLog::vOutput(0, "Account Balance: %F\n", balance);
}

void BtcJson::OnGetAccountAddress(QJsonValue result)
{

}

void BtcJson::OnListAccounts(QJsonValue result)
{
    if(!result.isObject())
        return;

    QJsonObject accounts = result.toObject();
    foreach(const QString key, accounts.keys())
    {
        double balance = accounts[key].toDouble();
        OTLog::vOutput(0, "%s: %FBTC\n", key.toStdString().c_str(), balance);
    }
}

void BtcJson::OnSendToAddress(QJsonValue result)
{
    if(!result.isString())
        return;

    OTLog::vOutput(0, "Transaction successfull (%s)\n", result.toString().toStdString().c_str());
    //OTLog::vOutput(0, "", 0);
}












