#ifndef IBTCRPC_H
#define IBTCRPC_H

#include <string>
#include <tr1/memory>   // in tr1, otherwise mac users might not find it i heard


// This struct can be used in BtcRpc to pass arguments to the ConnectToBitcoin() function
struct BitcoinServer
{
    std::string user;
    std::string password;
    std::string url;
    int port;

    BitcoinServer(const std::string account, std::string password, std::string url, int port):
        user(account), password(password), url(url), port(port)
    {}
}; typedef std::shared_ptr<BitcoinServer> BitcoinServerRef;


struct BitcoinReply
{
    int size = 0;
    std::shared_ptr<char> data;
}; typedef std::shared_ptr<BitcoinReply> BitcoinReplyRef;

class IBtcRpc
{
public:
    // Returns whether we're connected to bitcoin's http interface
    // Not implemented properly! And probably not necessary. Will have to check to be sure.
    virtual bool IsConnected() = 0;

    // Connects to bitcoin, sets some http header information and sends a test query (getinfo)
    // The whole connected/disconnected implementation is poorly done (my fault) but it works.
    // This function can be called again and again and nothing will crash if it fails.
    virtual bool ConnectToBitcoin(const std::string &user, const std::string &password, const std::string &url, int port) = 0;

    // Overload to make code that switches between bitcoin servers more readable
    virtual bool ConnectToBitcoin(BitcoinServerRef server) = 0;

    // Sends a string over the network
    // This string should be a json-rpc call if we're talking to bitcoin,
    // but we could send anything and expand this class to also connect to other http(s) interfaces.
    virtual BitcoinReplyRef SendRpc(const std::string &jsonString) = 0;

    // Sends a byte array over the network
    // Should be json-rpc if talking to bitcoin
    virtual BitcoinReplyRef SendRpc(const char* jsonString) = 0;
};


#endif // IBTCRPC_H
