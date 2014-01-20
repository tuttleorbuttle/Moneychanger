#ifndef BTCOBJECTS_H
#define BTCOBJECTS_H

#include <string.h>
#include <list>
#include <vector>
#include <jsoncpp/json/json.h>
#include <tr1/memory>
#include <sys/types.h>


namespace std
{
    typedef list<std::string> stringList;
}


// TODO: error checking


// if a query returns a complex object we should convert it to a struct instead of
// passing around the Json::Value

// This struct holds various information about a bitcoin transaction.
// Its constructor accepts the Json::Value returned by "gettransaction".
// Does not take into account block rewards.
struct BtcTransaction
{
    int Confirmations;
    // all amounts are satoshis
    int64_t AmountReceived;  // amount received
    int64_t AmountSent;      // amount sent
    int64_t Amount;          // +received -sent
    int64_t Fee;             // only displayed when sender
    std::string TxID;
    double Time;
    //std::string Account;
    std::list<std::string> AddressesRecv;   // received to addresses
    std::list<std::string> AddressesSent;   // sent to addresses
    //std::string Category;           // "send", "receive", "immature" (unconfirmed block reward), ...?

    BtcTransaction(Json::Value reply);

private:
    void SetDefaults();
};

struct BtcRawTransaction
{
    std::string txID;

    struct VIN
    {
        std::string txInID;
        int vout;   // number of txInID's output to be used as input

        VIN(std::string txInID, int vout)
            :txInID(txInID), vout(vout)
        {}
    };
    std::vector<VIN> inputs;

    struct VOUT
    {
        int64_t value;     // amount of btc to be sent
        int n ;              // outputs array index
        int reqSigs;        // signatures required to withdraw from the output addresses
        std::vector<std::string> addresses;    // an array of addresses receiving the value.
        std::string scriptPubKeyHex;

        VOUT()
        {
            this->value = 0;
            this->n = -1;
            this->reqSigs = 0;
            this->addresses = std::vector<std::string>();
            this->scriptPubKeyHex = "";
        }

        VOUT(int64_t value, int n, int reqSigs, std::vector<std::string> addresses, std::string scriptPubKeyHex)
            :value(value), n(n), reqSigs(reqSigs), addresses(addresses), scriptPubKeyHex(scriptPubKeyHex)
        {}

    };
    std::vector<VOUT> outputs;

    BtcRawTransaction(Json::Value rawTx);
};

struct BtcAddressInfo
{
    std::string address;
    std::string pubkey;
    std::string account;
    bool ismine;
    bool isvalid;
    bool isScript;
    Json::Value addresses;       // shows addresses which a multi-sig is composed of
    int sigsRequired = 0;

    BtcAddressInfo(Json::Value result);
};

struct BtcAddressAmount
{
    std::string Address;
    double Amount;
};

struct BtcMultiSigAddress
{
    std::string address;
    std::string redeemScript;
    std::stringList publicKeys;     // this will make everything so much more convenient

    BtcMultiSigAddress(Json::Value result, const std::stringList& publicKeys);
};

struct BtcBlock
{
    int confirmations;
    std::list<std::string> transactions;
    int height;
    std::string hash;
    std::string previousHash;

    BtcBlock();

    BtcBlock(Json::Value block);
};

struct BtcOutput : Json::Value
{
    BtcOutput(std::string txID, int vout);
};

struct BtcSignedTransaction
{
    std::string signedTransaction;
    bool complete;              // true if all (enough?) signatures were collected

    BtcSignedTransaction(Json::Value signedTransactionObj);
};

// used to sign some raw transactions
struct BtcSigningPrequisite : Json::Value
{
    // an array of BtcSigningPrequisites can be passed to bitcoin- as an optional argument to signrawtransaction
    // [{"txid":txid,"vout":n,"scriptPubKey":hex,"redeemScript":hex},...]
    // this is required if we
    //      a) didn't add the address to our wallet (createmultisig instead of addmultisigaddress)
    //      b) want to sign a raw tx only with a particular address's private key (the privkey is passed in another argument)

    BtcSigningPrequisite();

    BtcSigningPrequisite(std::string txId, int vout, std::string scriptPubKey, std::string redeemScript);

    // all of these values must be set or else prequisite is invalid
    void SetTxId(std::string txId);

    // all of these values must be set or else prequisite is invalid
    void SetVout(int vout);

    // all of these values must be set or else prequisite is invalid
    void SetScriptPubKey(std::string scriptPubKey);

    // all of these values must be set or else prequisite is invalid
    void SetRedeemScript(std::string redeemScript);
};

// not really a packet.
struct BtcRpcPacket
{
    int size = 0;                   // size of data
    char* data;     // received data or data to send

    BtcRpcPacket();

    BtcRpcPacket(const char *data, int size);

    BtcRpcPacket(const std::string &data);

    typedef std::shared_ptr<BtcRpcPacket> BtcRpcPacketRef;
    BtcRpcPacket(const BtcRpcPacketRef packet);

    ~BtcRpcPacket();

    void SetDefaults();
};

typedef std::shared_ptr<BtcTransaction> BtcTransactionRef;
typedef std::shared_ptr<BtcRawTransaction> BtcRawTransactionRef;
typedef std::shared_ptr<BtcAddressInfo> BtcAddressInfoRef;
typedef std::shared_ptr<BtcMultiSigAddress> BtcMultiSigAddressRef;
typedef std::shared_ptr<BtcBlock> BtcBlockRef;
typedef std::shared_ptr<BtcOutput> BtcOutputRef;
typedef std::shared_ptr<BtcSignedTransaction> BtcSignedTransactionRef;
typedef std::shared_ptr<BtcSigningPrequisite> BtcSigningPrequisiteRef;
typedef std::shared_ptr<BtcRpcPacket> BtcRpcPacketRef;


#endif // BTCOBJECTS_H
