#ifndef JSONRPC_H
#define JSONRPC_H

#include <QtNetwork/QNetworkReply>

///!
//! \brief The BitcoinRPC class will handle communication with bitcoin-qt.
//!
class BitcoinRPC : public QObject
{
public:
    ///!
    //! \brief Currently just calling bitcoin-qt on default port and asking "getinfo"
    //!
    BitcoinRPC();
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
