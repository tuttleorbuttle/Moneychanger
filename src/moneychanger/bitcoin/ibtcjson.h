#ifndef IBTCJSON_H
#define IBTCJSON_H

#include <string>
#include <list>
#include <map>
#include <tr1/memory>
#include "btcobjects.h"
#include "jsoncpp/json/value.h"

class IBtcJson
{
public:
    virtual void Initialize() = 0;       // should make this part of all modules

    virtual void GetInfo() = 0;

    virtual int64_t GetBalance(std::string account = NULL /*TODO: int minConfirmations*/) = 0;

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(std::string account = NULL) = 0;

    // Returns list of all addresses belonging to account
    virtual std::list<std::string> GetAddressesByAccount(std::string account = "") = 0;    // list addresses for account, "" is the default account.

    // Add new address to account
    virtual std::string GetNewAddress(std::string account = NULL) = 0;

    // Validate an address
    virtual BtcAddressInfoRef ValidateAddress(const std::string &address) = 0;

    virtual std::string GetPublicKey(const std::string& address) = 0;

    // Get private key for address (calls DumpPrivKey())
    virtual std::string GetPrivateKey(const std::string &address) = 0;

    // Get private key for address
    virtual std::string DumpPrivKey(std::string address) = 0;

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    virtual BtcMultiSigAddressRef AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account = NULL) = 0;

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressRef CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys) = 0;

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(int nRequired, std::list<std::string> keys) = 0;

    // Returns list of account names
    // Could also return the balance of each account but I find that confusing
    virtual std::vector<std::string> ListAccounts() = 0;

    virtual std::string SendToAddress(const std::string &btcAddress, int64_t amount) = 0;

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(std::map<std::string, int64_t> txTargets, std::string fromAccount = NULL) = 0;

    virtual bool SetTxFee(int64_t fee) = 0;

    virtual BtcTransactionRef GetTransaction(std::string txID) = 0;

    virtual std::string GetRawTransaction(std::string txID) = 0;

    virtual BtcRawTransactionRef GetDecodedRawTransaction(std::string txID) = 0;

    virtual BtcRawTransactionRef DecodeRawTransaction(std::string rawTransaction) = 0;

    virtual std::string CreateRawTransaction(const std::list<BtcOutput> &unspentOutputs, std::map<std::string, int64_t> txTargets) = 0;

    virtual BtcSignedTransactionRef SignRawTransaction(const std::string &rawTransaction, const std::list<BtcSigningPrequisite> &previousTransactions = std::list<BtcSigningPrequisite>(), const std::stringList &privateKeys = std::list<std::string>()) = 0;

    virtual BtcSignedTransactionRef CombineSignedTransactions(std::string rawTransaction) = 0;

    virtual std::string SendRawTransaction(const std::string &rawTransaction) = 0;

    virtual std::vector<std::string> GetRawMemPool() = 0;

    virtual int GetBlockCount() = 0;

    virtual std::string GetBlockHash(int blockNumber) = 0;

    virtual BtcBlockRef GetBlock(const std::string &blockHash) = 0;

private:
     virtual BtcRpcPacketRef CreateJsonQuery(const std::string &command, Json::Value params = Json::Value(), std::string id = "") = 0;

     // Checks the reply object received from bitcoin-qt for errors and returns the reply
    virtual bool ProcessRpcString(BtcRpcPacketRef jsonString, Json::Value &result) = 0;
    // Splits the reply object received from bitcoin-qt into error and result objects
    virtual void ProcessRpcString(BtcRpcPacketRef jsonString, std::string &id, Json::Value& error, Json::Value& result) = 0;
};

typedef std::shared_ptr<IBtcJson> IBtcJsonRef;

#endif // IBTCJSON_H
