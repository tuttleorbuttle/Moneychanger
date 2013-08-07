#include <QJsonArray>
#include <QJsonDocument>
//#include <QJsonValue>
#include <QJsonObject>
#include "json.h"
#include "OTLog.h"
#include <QVariant>


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
    doc = doc.fromVariant(bob);
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

    int asdf = 0;
}
