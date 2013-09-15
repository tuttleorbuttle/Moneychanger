#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "btcrpc.h"


namespace BtcJsonReplies
{
    class GetInfoReply
    {

    };
}

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
    bool ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue &result);
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result);

    void GetInfo();
    void GetBalanceAsync(QString account = NULL);
    QString GetAccountAddress(QString account = NULL);
    QString GetNewAddress(QString account = NULL);
    void ListAccounts();
    bool SendToAddress(QString btcAddress, double amount);
    QString AddMultiSigAddress(<nrequired> <'["key","key"]'> [account] )    // gonna continue here at home, remove this line and it should compile.

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
