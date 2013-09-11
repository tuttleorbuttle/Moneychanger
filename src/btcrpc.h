#ifndef JSONRPC_H
#define JSONRPC_H

#include <QtNetwork>
#include <QString>
#include <QLinkedList>
#include "FastDelegate.h"
#include "IStringProcessor.h"

typedef void (*ProcessString)(QSharedPointer<QByteArray>);

using namespace fastdelegate;

///!
//! \brief The BitcoinRPC class will handle communication with bitcoin-qt.
//!
class BtcRpc : public QObject
{
public:
    ///!
    //! \brief Currently just calling bitcoin-qt on default port and asking "getinfo"
    //!
    BtcRpc();
    ~BtcRpc();

    void ConnectToBitcoin(QString url, int port, QString user, QString password);

    void SendRpc(const QString jsonString);
    void SendRpc(const QByteArray jsonString);

    void RegisterStringProcessor(QByteArray contentType, FastDelegate1<QSharedPointer<QByteArray>, IStringProcessor> delegate);

private:
    void InitSession();         // Called in constructor, makes sure we have internet or something
    void InitBitcoinRpc();      // Also called in constr, sets header information to what bitcoin-qt is expecting
    void ConnectBitcoinRpc();   // sends getinfo to bitcoin, was used for testing but can/should be removed now.

    void ProcessReply(QSharedPointer<QByteArray> replyContType, const QSharedPointer<QByteArray> replyContent);
    void ProcessErrorMessage(const QNetworkReply *reply);

    QMap<QByteArray, FastDelegate1<QSharedPointer<QByteArray>, IStringProcessor> > StringProcessors;
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
