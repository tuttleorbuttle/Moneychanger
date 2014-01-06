#ifndef IBTCJSON_H
#define IBTCJSON_H

#include <string>
#include <list>
#include <map>

class IBtcJson
{
public:
    virtual void Initialize();       // should make this part of all modules

    virtual void GetInfo();

    virtual int64_t GetBalance(QString account = NULL);

    // Gets the default address for the specified account
    virtual std::string GetAccountAddress(const std::string &account = NULL);

    // Returns list of all addresses belonging to account
    virtual std::list<std::string> GetAddressesByAccount(std::string account = "");    // list addresses for account, "" is the default account.

    // Add new address to account
    virtual std::string GetNewAddress(std::string account = NULL);

    // Validate an address
    virtual BtcAddressInfoRef ValidateAddress(const std::string &address);

    virtual std::string GetPublicKey(const std::string& address);

    // Get private key for address (calls DumpPrivKey())
    virtual std::string GetPrivateKey(const std::string &address);

    // Get private key for address
    virtual std::string DumpPrivKey(std::string address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    virtual BtcMultiSigAddressRef AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account = NULL);

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    virtual BtcMultiSigAddressRef CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys);

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    virtual std::string GetRedeemScript(int nRequired, std::list<std::string> keys);

    // Returns list of account names
    // Could also return the balance of each account but I find that confusing
    virtual std::list<std::string> ListAccounts();

    virtual std::string SendToAddress(std::string btcAddress, int64_t amount);

    // Send to multiple addresses at once
    // txTargets maps amounts (int64 satoshis) to addresses (QString)
    virtual std::string SendMany(std::map<std::string, int64_t> txTargets, std::string fromAccount = NULL);

    virtual bool SetTxFee(int64_t fee);

    virtual BtcTransactionRef GetTransaction(std::string txID);

    virtual std::string GetRawTransaction(std::string txID);

    virtual BtcRawTransactionRef GetDecodedRawTransaction(std::string txID);

    virtual BtcRawTransactionRef DecodeRawTransaction(std::string rawTransaction);

    virtual std::string CreateRawTransaction(QList<BtcOutput> unspentOutputs, QMap<std::string, int64_t> txTargets);

    virtual BtcSignedTransactionRef SignRawTransaction(std::string rawTransaction, QList<BtcSigningPrequisite> previousTransactions = QList<BtcSigningPrequisite>(), QStringList privateKeys = QStringList());

    virtual BtcSignedTransactionRef CombineSignedTransactions(std::string rawTransaction);

    virtual std::string SendRawTransaction(QString rawTransaction);

    virtual std::list<std::string> GetRawMemPool();

    virtual int GetBlockCount();

    virtual std::string GetBlockHash(int blockNumber);

    virtual BtcBlockRef GetBlock(QString blockHash);

private:
     virtual QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");

     // Checks the reply object received from bitcoin-qt for errors and returns the reply
     virtual bool ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue &result);
     // Splits the reply object received from bitcoin-qt into error and result objects
     virtual void ProcessRpcString(QSharedPointer<QByteArray> jsonString, std::string &id, QJsonValue& error, QJsonValue& result);
};

#endif // IBTCJSON_H
