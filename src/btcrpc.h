#ifndef JSONRPC_H
#define JSONRPC_H

#include <QtNetwork>
#include <QString>
#include <QLinkedList>

// This struct can be used in BtcRpc to pass arguments to the ConnectToBitcoin() function
struct BitcoinServer
{
    QString user;
    QString password;
    QString url;
    int port;

    BitcoinServer(QString account, QString password, QString url, int port):
        user(account), password(password), url(url), port(port)
    {}
};

// This class will handle network communication.
// It can only connect to one http interface at a time,
// but you can connect to a new one at will using ConnectToBitcoin()
class BtcRpc : public QObject
{
public:
    BtcRpc();
    ~BtcRpc();

    // Returns whether we're connected to bitcoin's http interface
    // Not implemented properly! And probably not necessary. Will have to check to be sure.
    bool IsConnected();

    // Connects to bitcoin, sets some http header information and sends a test query (getinfo)
    // The whole connected/disconnected implementation is poorly done (my fault) but it works.
    // This function can be called again and again and nothing will crash if it fails.
    bool ConnectToBitcoin(QString user, QString password, QString url = "http://127.0.0.1", int port = 8332);

    // Overload to make code that switches between bitcoin servers more readable
    bool ConnectToBitcoin(QSharedPointer<BitcoinServer> server);

    // Sends a string over the network
    // This string should be a json-rpc call if we're talking to bitcoin,
    // but we could send anything and expand this class to also connect to other http(s) interfaces.
    QSharedPointer<QByteArray> SendRpc(const QString jsonString);

    // Sends a byte array over the network
    // Should be json-rpc if talking to bitcoin
    QSharedPointer<QByteArray> SendRpc(const QByteArray jsonString);

private:
    void InitSession();         // Called in constructor, makes sure we have a network interface or something
    void InitNAM();             // Also called in constr, creates the Network Access Manager.
    void SetHeaderInformation(); // Called by ConnectToBitcoin, sets the HTTP header

    // Processes a network reply
    // Probably not needed anymore
    void ProcessReply(QSharedPointer<QByteArray> replyContType, const QSharedPointer<QByteArray> replyContent);

    // Processes an error message
    // These can come from a failed network connection, erroneus json-rpc calls or meta-errors like lack of funds.
    void ProcessErrorMessage(const QNetworkReply *reply);

    QPointer<QNetworkSession> session;

    QScopedPointer<QNetworkAccessManager> rpcNAM;      // this is used to send http packets to bitcoin-qt
    QScopedPointer<QNetworkRequest> rpcRequest;
    QSharedPointer<QByteArray> rpcReplyContent;

    QString username = "";
    QString password = "";
    bool connected = false;

Q_OBJECT
public slots:

    // Called when there's a network reply from bitcoin
    void OnNetReplySlot(QNetworkReply* reply);

    // Called if bitcoin requires authentication (user/pw)
    void OnAuthReqSlot(QNetworkReply* reply, QAuthenticator* authenticator);
};

#endif // JSONRPC_H
