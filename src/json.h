#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "bitcoinrpc.h"

//!
//! \brief This class will create/process json queries.
//!
class Json //: IStringProcessor
{
public:
    ///!
    //! \brief Currently just doing some testing to figure out how QJson works
    //!
    Json();
    ~Json();

    void Initialize();       // should make this part of all modules

    //void ProcessString(QSharedPointer<QByteArray>);
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString);

    void GetInfo();
    void GetBalance();
    void GetAccountAddress();
    void ListAccounts();
    void SendToAddress(QString btcAddress, double amount);

    void OnGetInfo(QJsonValue result);
    void OnGetBalance(QJsonValue result);
    void OnGetAccountAddress(QJsonValue result);
    void OnListAccounts(QJsonValue result);
    void OnSendToAddress(QJsonValue result);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");
};

#endif // JSON_H
