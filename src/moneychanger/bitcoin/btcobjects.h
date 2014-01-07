#ifndef BTCOBJECTS_H
#define BTCOBJECTS_H

#include <string.h>
#include <list>
#include <jsoncpp/json/json.h>
#include <tr1/memory>
#include "btchelper.h"


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

    BtcTransaction(Json::Value reply)
    {
        SetDefaults();

        if(!reply["error"].isNull())
            return;

        this->Confirmations = (int)(reply["confirmations"].asDouble());
        this->Amount = BtcHelper::CoinsToSatoshis(reply["amount"].asDouble());

        this->Fee = BtcHelper::CoinsToSatoshis(reply["fee"].asDouble());
        this->TxID = reply["txid"].asString();
        this->Time = reply["time"].asDouble();

        // details
        if(!reply["details"].isArray())
            return;
        Json::Value details = reply["details"];
        if(details.size() == 0)
            return;

        for(int i = 0; i < details.size(); i++)
        {
            Json::Value detail = details[i];
            std::string address = detail["address"].asString();
            std::string category = detail["category"].asString();
            int64_t amount = BtcHelper::CoinsToSatoshis(detail["amount"].asDouble());
            if(category == "send")
            {
                this->AddressesSent.push_back(address);
                this->AmountSent += amount; // will be 0 or less
            }
            else if(category == "receive")
            {
                this->AddressesRecv.push_back(address);
                this->AmountReceived += amount; // will be 0 or more
            }
            else if(category == "immature")
            {
                // that's block reward.
            }
        }
    }

private:
    void SetDefaults()
    {
        //TotalAmount = 0.0;
        this->Confirmations = 0;
        this->AmountReceived = 0.0;
        this->AmountSent = 0.0;
        this->Amount = 0.0;
        this->Fee = 0.0;
        this->TxID = std::string();
        this->Time = 0;
        this->AddressesRecv = std::list<std::string>();
        this->AddressesSent = std::list<std::string>();
    }
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
    std::list<VIN> inputs;

    struct VOUT
    {
        int64_t value;     // amount of btc to be sent
        int n ;              // outputs array index
        int reqSigs;        // signatures required to withdraw from the output addresses
        std::list<std::string> addresses;    // an array of addresses receiving the value.
        std::string scriptPubKeyHex;

        VOUT()
        {
            this->value = 0;
            this->n = -1;
            this->reqSigs = 0;
            this->addresses = std::list<std::string>();
            this->scriptPubKeyHex = "";
        }

        VOUT(int64_t value, int n, int reqSigs, std::list<std::string> addresses, std::string scriptPubKeyHex)
            :value(value), n(n), reqSigs(reqSigs), addresses(addresses), scriptPubKeyHex(scriptPubKeyHex)
        {}

    };
    std::list<VOUT> outputs;

    BtcRawTransaction(Json::Value rawTx)
    {
        this->inputs = std::list<VIN>();
        this->outputs = std::list<VOUT>();

        this->txID = rawTx["txid"].asString();

        Json::Value vin = rawTx["vin"];
        for(int i = 0; i < vin.size(); i++)
        {
            Json::Value inputObj = vin[i];
            this->inputs.push_back(VIN(inputObj["txid"].asString(), (int)inputObj["vout"].asDouble()));
        }

        Json::Value vouts  = rawTx["vout"];
        for(int i = 0; i < vouts.size(); i++)
        {
            Json::Value outputObj = vouts[i];
            VOUT output;

            output.value = BtcHelper::CoinsToSatoshis(outputObj["value"].asDouble());
            output.n = (int)outputObj["n"].asDouble();      // JSON doesn't know integers

            Json::Value scriptPubKey = outputObj["scriptPubKey"];
            output.reqSigs = (int)scriptPubKey["reqSigs"].asDouble();
            Json::Value addresses = scriptPubKey["addresses"];
            for(int i= 0; i < addresses.size(); i++)
            {
                output.addresses.push_back(addresses[i].asString());
            }
            output.scriptPubKeyHex = scriptPubKey["hex"].asString();

            this->outputs.push_back(output);
        }

    }
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

    BtcAddressInfo(Json::Value result)
    {
        // it seems we don't need to do any error checking. thanks, .
        this->address = result["address"].asString();   // if wrong type, default value will be returned
        this->pubkey = result["pubkey"].asString();
        this->account = result["account"].asString();
        this->ismine = result["ismine"].asBool();
        this->isvalid = result["isvalid"].asBool();

        // multi-sig properties:
        this->isScript = result["isscript"].asBool();
        this->addresses = result["addresses"];
    }
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

    BtcMultiSigAddress(Json::Value result, const std::stringList& publicKeys)
    {
        this->address = result["address"].asString();
        this->redeemScript = result["redeemScript"].asString();

        this->publicKeys = publicKeys;
    }
};

struct BtcBlock
{
    int confirmations;
    std::list<std::string> transactions;
    int height;
    std::string hash;
    std::string previousHash;

    BtcBlock()
    {
        this->confirmations = 0;
        this->transactions = std::list<std::string>();
        this->height = 0;
        this->hash = std::string();
        this->previousHash = std::string();
    }

    BtcBlock(Json::Value block)
    {
        // latest block has 1 confirmations I think so tx->confirms == block->confirms
        this->confirmations = (int)block["confirmations"].asDouble();

        // block number (count since genesis)
        this->height = (int)block["height"].asDouble();

        this->hash = block["hash"].asString();
        this->previousHash = block["previousblockhash"].asString();

        // get list of transactions in the block
        Json::Value transacts = block["tx"];
        for(int i = 0; i < transacts.size(); i++)
        {
            this->transactions.push_back(transacts[i].asString());
        }
    }
};

struct BtcOutput : Json::Value
{
    BtcOutput(std::string txID, int vout)
    {
        (*this)["txid"] = txID;
        (*this)["vout"] = vout;
    }
};

struct BtcSignedTransaction
{
    std::string signedTransaction;
    bool complete;              // true if all (enough?) signatures were collected

    BtcSignedTransaction(Json::Value signedTransactionObj)
    {
        this->signedTransaction = signedTransactionObj["hex"].asString();
        this->complete = signedTransactionObj["complete"].asBool();
    }
};

// used to sign some raw transactions
struct BtcSigningPrequisite : Json::Value
{
    // an array of BtcSigningPrequisites can be passed to bitcoin- as an optional argument to signrawtransaction
    // [{"txid":txid,"vout":n,"scriptPubKey":hex,"redeemScript":hex},...]
    // this is required if we
    //      a) didn't add the address to our wallet (createmultisig instead of addmultisigaddress)
    //      b) want to sign a raw tx only with a particular address's private key (the privkey is passed in another argument)

    BtcSigningPrequisite()
    {}

    BtcSigningPrequisite(std::string txId, int vout, std::string scriptPubKey, std::string redeemScript)
    {
        // all of these values must be set or else prequisite is invalid
        (*this)["txid"] = txId;
        (*this)["vout"] = vout;
        (*this)["scriptPubKey"] = scriptPubKey;
        (*this)["redeemScript"] = redeemScript;
    }

    // all of these values must be set or else prequisite is invalid
    void SetTxId(std::string txId)
    {
        // we can get this value from the transaction used to send funds to the p2sh
        (*this)["txid"] = txId;
    }

    // all of these values must be set or else prequisite is invalid
    void SetVout(int vout)
    {
        // we can get this value from the transaction used to send funds to the p2sh
        (*this)["vout"] = vout;
    }

    // all of these values must be set or else prequisite is invalid
    void SetScriptPubKey(std::string scriptPubKey)
    {
        // we can get this value from the transaction used to send funds to the p2sh
        (*this)["scriptPubKey"] = scriptPubKey;
    }

    // all of these values must be set or else prequisite is invalid
    void SetRedeemScript(std::string redeemScript)
    {
        // we can get this from the createmultisig api function
        (*this)["redeemScript"] = redeemScript;
    }
};

// not really a packet.
struct BtcRpcPacket
{
    int size = 0;                   // size of data
    char* data;     // received data or data to send

    BtcRpcPacket(char* data, int size)
    {
        this->data = data;
        this->size = size;
    }

    BtcRpcPacket(const std::string &data)
    {
        this->data = new char[data.size()];
        memccpy(this->data, data.c_str(), NULL, data.size());
        this->size = data.size();
    }

    ~BtcRpcPacket()
    {
        this->size = 0;
        delete[] this->data;
        this->data = NULL;
    }
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
