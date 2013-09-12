#include <QJsonArray>
#include <QJsonDocument>
//#include <QJsonValue>
#include <QJsonObject>
#include "btcjson.h"
#include "OTLog.h"
#include <QVariant>
#include "modules.h"
#include "FastDelegate.h"


// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_SENDTOADDRESS        "sendtoaddress"

using namespace fastdelegate;

BtcJson::BtcJson()
{

}

void BtcJson::Initialize()
{
    FastDelegate1<QSharedPointer<QByteArray> > processDelegate;
    processDelegate.bind(this, &BtcJson::ProcessRpcString);
    QSharedPointer<QByteArray> arr;
    processDelegate(arr);

    FastDelegate1<QSharedPointer<QByteArray> > reply;
    reply.bind(this, &BtcJson::ProcessRpcString);
    Modules::bitcoinRpc->RegisterStringProcessor("application/json", reply);
}

BtcJson::~BtcJson()
{
}

void BtcJson::ProcessRpcString(QSharedPointer<QByteArray> jsonString)
{
    if(jsonString == NULL || jsonString->length() == 0)
        return;
    QJsonDocument replyDoc = QJsonDocument::fromJson(*jsonString);
    if(replyDoc.isNull() || replyDoc.isEmpty())
        return;

    //OTLog::vOutput(0, "Received JSON:\n%s\n", QString(replyDoc.toJson()).toStdString().c_str());     // I think casting the json doc back to json adds linebreaks and stuff.

    if(replyDoc.isObject())
    {
        QJsonObject replyObj = replyDoc.object();

        QJsonValue id = replyObj["id"];    // this is the same ID we sent to bitcoin-qt earlier.
        if(!id.isString())
            return;
        QString idStr = id.toString();

        QJsonValue error = replyObj["error"];
        if(!error.isNull())
        {
            OTLog::vOutput(0, "Error in reply to \"%s\": %s\n\n", idStr.toStdString().c_str(), error.isObject() ? (error.toObject()["message"]).toString().toStdString().c_str() : "");
            //return;
        }

        QJsonValue result = replyObj["result"];

        this->rpcReplies[id.toString()] = replyObj;

        // TODO: also send error so functions can check if they were successfull
        if(idStr == METHOD_GETINFO)
            OnGetInfo(result, error);
        else if(idStr == METHOD_GETBALANCE)
            OnGetBalance(result);
        else if(idStr == METHOD_GETACCOUNTADDRESS)
            OnGetAccountAddress(result);
        else if(idStr == METHOD_LISTACCOUNTS)
            OnListAccounts(result);
        else if(idStr == METHOD_SENDTOADDRESS)
            OnSendToAddress(result);
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

void BtcJson::GetInfo()
{
    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
}

double BtcJson::GetBalance(QString account)
{
    QJsonArray params;
    params.append(account);      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    return 0.0;
}

void BtcJson::GetAccountAddress()
{

}

void BtcJson::ListAccounts()
{
    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS));
}

void BtcJson::SendToAddress(QString btcAddress, double amount)
{
    QJsonArray params;
    params.append(btcAddress);
    params.append(amount);

    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params));
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












