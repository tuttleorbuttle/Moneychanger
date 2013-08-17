#include <QJsonArray>
#include <QJsonDocument>
//#include <QJsonValue>
#include <QJsonObject>
#include "json.h"
#include "OTLog.h"
#include <QVariant>
#include "modules.h"


// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_SENDTOADDRESS        "sendtoaddress"

Json::Json()
{
    QJsonArray a;
    QJsonValue v(5);
    a.push_back(v);
    QJsonArray a2;
    a2.append(a);
    QJsonDocument doc;
    QString query = "{ \"method\": \"echo\", \"params\": [\"Hello JSON-RPC\"], \"id\": 1}";

    QVariantList people;
    QVariantMap bob;
    bob.insert("Name", "Bob");
    bob.insert("Phonenumber", 123);
    QVariantMap alice;
    alice.insert("Name", "Alice");
    alice.insert("Phonenumber", 321);
    people.push_back(bob);
    people.push_back(alice);
    doc = doc.fromVariant(people);
    foreach (const QVariant& v, people )
    {
        QMap<QString, QVariant> vmap = v.toMap();
        foreach(const QString& s, vmap.keys())
        {
            OTLog::vOutput(0, "%s : %s\n", s.toStdString().c_str(), vmap[s].toString().toStdString().c_str());
        }
        OTLog::vOutput(0,"variant: %s\n", v.toString().toStdString().c_str());
    }
    QString docjson = QString(doc.toJson());
    OTLog::vOutput(0, "docjson: %s\n", docjson.toStdString().c_str());


    foreach (const QJsonValue& v, doc.array())
    {
        switch(v.type())
        {
        case QJsonValue::Null:
            break;
        case QJsonValue::Bool:
            OTLog::Output(0, v.toBool() ? "True" : "False");
        case QJsonValue::Double:
            OTLog::Output(0, QString::number(v.toDouble()).toStdString().c_str());
        case QJsonValue::String:
            OTLog::Output(0, v.toString().toStdString().c_str());
        case QJsonValue::Array:
            break;
        case QJsonValue::Object:
            break;
        case QJsonValue::Undefined:
        default:
            break;
        }
    }

    QString s;
    if(doc.isArray())
        a = doc.array();
    else if(doc.isObject())
        QJsonObject o = doc.object();
    foreach (const QJsonValue& v, a)
    {
        switch(v.type())
        {
        case QJsonValue::Null:
            break;
        case QJsonValue::Bool:
            OTLog::Output(0, v.toBool() ? "True" : "False");
        case QJsonValue::Double:
            OTLog::Output(0, QString::number(v.toDouble()).toStdString().c_str());
        case QJsonValue::String:
        case QJsonValue::Array:
        case QJsonValue::Object:
        case QJsonValue::Undefined:
        default:
            break;
        }
    }
}

void Json::Initialize()
{
    ProcessString reply = (ProcessString)&Json::ProcessRpcString;
    Modules::bitcoinRpc->RegisterStringProcessor("application/json", reply);
}

Json::~Json()
{
    int a = 0;
}

void Json::ProcessRpcString(QSharedPointer<QByteArray> jsonString)
{
    QJsonDocument doc = QJsonDocument::fromJson(*jsonString);
    if(doc.isNull() || doc.isEmpty())
        return;

    OTLog::vOutput(0, "Received JSON:\n%s\n", QString(doc.toJson()).toStdString().c_str());     // I think casting the json doc back to json adds linebreaks and stuff.

    if(doc.isObject())
    {
        QJsonObject o = doc.object();

        QJsonValue id = o["id"];    // this is the same ID we sent to bitcoin-qt earlier.
        if(!id.isString())
            return;
        QString idStr = id.toString();

        QJsonValue error = o["error"];
        if(!error.isNull())
        {
            OTLog::vOutput(0, "Error in reply to \"%s\": %s\n\n", idStr.toStdString().c_str(), error.isObject() ? (error.toObject()["message"]).toString().toStdString().c_str() : "");
            //return;
        }

        QJsonValue result = o["result"];

        // TODO: also send error so functions can check if they were successfull
        if(idStr == METHOD_GETINFO)
            OnGetInfo(result);
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

QByteArray Json::CreateJsonQuery(QString command, QJsonArray params, QString id)
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

void Json::GetInfo()
{
    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
}

void Json::GetBalance()
{
    QJsonArray params;
    params.append(QString(""));      // account
    params.append(1);       // min confirmations, 1 is default

    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));
}

void Json::GetAccountAddress()
{

}

void Json::ListAccounts()
{
    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS));
}

void Json::SendToAddress(QString btcAddress, double amount)
{
    QJsonArray params;
    params.append(btcAddress);
    params.append(amount);

    Modules::bitcoinRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params));
}


void Json::OnGetInfo(QJsonValue result)
{

}

void Json::OnGetBalance(QJsonValue result)
{
    if(!result.isDouble())
        return;

    double balance = (float)result.toDouble();
    OTLog::vOutput(0, "Account Balance: %F", balance);
}

void Json::OnGetAccountAddress(QJsonValue result)
{

}

void Json::OnListAccounts(QJsonValue result)
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

void Json::OnSendToAddress(QJsonValue result)
{
    if(!result.isString())
        return;

    OTLog::vOutput(0, "Transaction successfull (%s)\n", result.toString().toStdString().c_str());
    //OTLog::vOutput(0, "", 0);
}
