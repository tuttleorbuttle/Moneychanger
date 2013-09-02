#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "IStringProcessor.h"
#include "bitcoinrpc.h"


class Json;
typedef void (*OnRpcReply)(QJsonValue result, QJsonValue error);


// This class will create/process json queries and send/receive them with the help of BitcoinRpc.
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

    void OnGetInfo(QJsonValue result, QJsonValue error);
    void OnGetBalance(QJsonValue result);
    void OnGetAccountAddress(QJsonValue result);
    void OnListAccounts(QJsonValue result);
    void OnSendToAddress(QJsonValue result);

    // used to send bitcoin's replies to the correct function. the function will be called if a reply with its ID is received.
    // Once called, the function will be removed from the list. If we need to always call the same function, I'll implement that in another list.
    void RegisterOnRpcReply(QString id, OnRpcReply onReply);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");

     QMap<QString, OnRpcReply> onReplyList;
};

#endif // JSON_H
