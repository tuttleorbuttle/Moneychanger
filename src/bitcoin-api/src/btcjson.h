#ifndef BTCJSON_H
#define BTCJSON_H

#include "ibtcjson.h"
#include "btcmodules.h"

#include <tr1/memory>

class BtcJson : public IBtcJson
{
public:
    BtcJson(BtcModules* modules);
    ~BtcJson();

    virtual void Initialize();       // should make this part of all modules

    virtual void GetInfo();

    virtual int64_t GetBalance(const char *account = NULL /*TODO: int minConfirmations*/);

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(const std::string &account = "");

    // Returns list of all addresses belonging to account
    // account "" is default
    virtual std::list<std::string> GetAddressesByAccount(const std::string &account = "");

    // Add new address to account
    virtual std::string GetNewAddress(const std::string &account = "");

    // Validate an address
    virtual BtcAddressInfoRef ValidateAddress(const std::string &address);

    virtual std::string GetPublicKey(const std::string& address);

    // Get private key for address (calls DumpPrivKey())
    virtual std::string GetPrivateKey(const std::string &address);

    // Get private key for address
    virtual std::string DumpPrivKey(const std::string &address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    virtual BtcMultiSigAddressRef AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account = "");

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressRef CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys);

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(int nRequired, std::list<std::string> keys);

    // Returns list of account names
    // Could also return the balance of each account
    virtual std::vector<std::string> ListAccounts();

    virtual std::string SendToAddress(const std::string &btcAddress, int64_t amount);

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(std::map<std::string, int64_t> txTargets, const std::string &fromAccount = "");

    virtual bool SetTxFee(int64_t fee);

    virtual BtcTransactionRef GetTransaction(std::string txID);

    virtual std::string GetRawTransaction(std::string txID);

    virtual BtcRawTransactionRef GetDecodedRawTransaction(std::string txID);

    virtual BtcRawTransactionRef DecodeRawTransaction(std::string rawTransaction);

    virtual std::string CreateRawTransaction(const std::list<BtcOutput> &unspentOutputs, std::map<std::string, int64_t> txTargets);

    virtual BtcSignedTransactionRef SignRawTransaction(const std::string &rawTransaction, const std::list<BtcSigningPrequisite> &previousTransactions = std::list<BtcSigningPrequisite>(), const std::stringList &privateKeys = std::list<std::string>());

    virtual BtcSignedTransactionRef CombineSignedTransactions(std::string rawTransaction);

    virtual std::string SendRawTransaction(const std::string &rawTransaction);

    virtual std::vector<std::string> GetRawMemPool();

    virtual int GetBlockCount();

    virtual std::string GetBlockHash(int blockNumber);

    virtual BtcBlockRef GetBlock(const std::string &blockHash);

private:
     virtual BtcRpcPacketRef CreateJsonQuery(const std::string &command, Json::Value params = Json::Value(), std::string id = "");

     // Checks the reply object received from bitcoin-qt for errors and returns the reply
    virtual bool ProcessRpcString(BtcRpcPacketRef jsonString, Json::Value &result);
    // Splits the reply object received from bitcoin-qt into error and result objects
    virtual void ProcessRpcString(BtcRpcPacketRef jsonString, std::string &id, Json::Value& error, Json::Value& result);

    BtcModules* modules;
};

typedef std::shared_ptr<BtcJson> BtcJsonRef;

#endif // BTCJSON_H
