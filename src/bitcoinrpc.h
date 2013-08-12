#ifndef JSONRPC_H
#define JSONRPC_H

#include <QtNetwork>
#include <QString>
#include <QLinkedList>
//#include "IStringProcessor.h"

typedef void (*ProcessString)(QSharedPointer<QByteArray>);

///!
//! \brief The BitcoinRPC class will handle communication with bitcoin-qt.
//!
class BitcoinRpc : public QObject
{
public:
    ///!
    //! \brief Currently just calling bitcoin-qt on default port and asking "getinfo"
    //!
    BitcoinRpc();
    ~BitcoinRpc();

    QString SendRpc(QString jsonString);
    QString SendRpc(QByteArray jsonString);
    QString SendRpc(QString jsonString, QString& response);  // accept function pointer to void foo(qstring str) or maybe SIGNAL?

    void RegisterStringProcessor(QByteArray contentType, ProcessString delegate);

private:
    void InitSession();
    void InitBitcoinRpc();
    void ConnectBitcoinRpc();

    void ProcessReply(QSharedPointer<QByteArray> replyContType, QSharedPointer<QByteArray> replyContent);

    QMap<QByteArray, ProcessString> StringProcessors;
    QPointer<QNetworkSession> session;

    QScopedPointer<QNetworkAccessManager> rpcNAM;      // this is used to send http packets to bitcoin-qt
    QScopedPointer<QNetworkRequest> rpcRequest;

Q_OBJECT
public slots:
    ///!
    //! \brief finishedSlot is called when there's a response from bitcoin-qt
    //! \param reply is aforementioned response
    //!
    void finishedSlot(QNetworkReply* reply);
    void authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
};

#endif // JSONRPC_H
