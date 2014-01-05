#ifndef BTCRPCZMQ_H
#define BTCRPCZMQ_H

#include <string>
#include <tr1/memory>   // in tr1, otherwise mac users might not find it i heard



// This struct can be used in BtcRpc to pass arguments to the ConnectToBitcoin() function
struct BitcoinServer
{
    std::string user;
    std::string password;
    std::string url;
    int port;

    BitcoinServer(const std::string &account, std::string &password, std::string &url, int port):
        user(account), password(password), url(url), port(port)
    {}
}; typedef std::shared_ptr<BitcoinServer> BitcoinServerRef;


class BtcRpcCurl
{
public:
    BtcRpcCurl();
    ~BtcRpcCurl();

    // Returns whether we're connected to bitcoin's http interface
    // Not implemented properly! And probably not necessary. Will have to check to be sure.
    bool IsConnected();

    // Connects to bitcoin, sets some http header information and sends a test query (getinfo)
    // The whole connected/disconnected implementation is poorly done (my fault) but it works.
    // This function can be called again and again and nothing will crash if it fails.
    bool ConnectToBitcoin(const std::string &user, const std::string &password, const std::string &url = "http://127.0.0.1", int port = 8332);

    // Overload to make code that switches between bitcoin servers more readable
    bool ConnectToBitcoin(BitcoinServerRef server);

    // Sends a string over the network
    // This string should be a json-rpc call if we're talking to bitcoin,
    // but we could send anything and expand this class to also connect to other http(s) interfaces.
    void SendRpc(const std::string &jsonString);

    // Sends a byte array over the network
    // Should be json-rpc if talking to bitcoin
    void SendRpc(const char* jsonString);

private:
    void InitSession();         // Called in constructor, makes sure we have a network interface or something
    void InitNAM();             // Also called in constr, creates the Network Access Manager.
    void SetHeaderInformation(); // Called by ConnectToBitcoin, sets the HTTP header

    // Processes a network reply
    // Probably not needed anymore
    //void ProcessReply(QSharedPointer<QByteArray> replyContType, const QSharedPointer<QByteArray> replyContent);

    // Processes an error message
    // These can come from a failed network connection, erroneus json-rpc calls or meta-errors like lack of funds.
    //void ProcessErrorMessage(const QNetworkReply *reply);

    //QPointer<QNetworkSession> session;

    //QScopedPointer<QNetworkAccessManager> rpcNAM;      // this is used to send http packets to bitcoin-qt
    //QScopedPointer<QNetworkRequest> rpcRequest;
    //QSharedPointer<QByteArray> rpcReplyContent;

    BitcoinServerRef currentServer;
    bool connected;
    bool waitingForReply;

    /*
    Q_OBJECT
public slots:

    // Called when there's a network reply from bitcoin
    void OnNetReplySlot(QNetworkReply* reply);
    void OnNetReplySlot();

    // Called if bitcoin requires authentication (user/pw)
    void OnAuthReqSlot(QNetworkReply* reply, QAuthenticator* authenticator);
    */
};

#endif // BTCRPCZMQ_H
