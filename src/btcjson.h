#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "btcrpc.h"


// This class will create/process json queries and send/receive them with the help of BitcoinRpc.
class BtcJson //: IStringProcessor
{    
public:
    ///!
    //! \brief Currently just doing some testing to figure out how QJson works
    //!
    BtcJson();
    ~BtcJson();

    void Initialize();       // should make this part of all modules

    //void ProcessString(QSharedPointer<QByteArray>);
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString);

    void GetInfo();
    void GetBalanceAsync(QString account = NULL);
    void GetAccountAddress();
    void ListAccounts();
    void SendToAddress(QString btcAddress, double amount);

    double GetBalance(QString account = NULL);  // new interface

    void OnGetInfo(QJsonValue result, QJsonValue error);
    void OnGetBalance(QJsonValue result);
    void OnGetAccountAddress(QJsonValue result);
    void OnListAccounts(QJsonValue result);
    void OnSendToAddress(QJsonValue result);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
