#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include <QString>
#include <QSharedPointer>
#include "btcrpc.h"
#include "bitcoin/btcjsonobjects.h"


// This class will create/process json queries and send/receive them with the help of BitcoinRpc
class BtcJson
{    
public:
    BtcJson();
    ~BtcJson();

    void Initialize();       // should make this part of all modules

    // converts a double bitcoin (as received through json api) to int64 satoshis
    int64_t DoubleToInt(double value);

    // converts int64 satoshis to double bitcoin
    double IntToDouble(int64_t value);

    // Checks the reply object received from bitcoin-qt for errors and returns the reply
    bool ProcessRpcString(QSharedPointer<QByteArray> jsonString, QJsonValue &result);
    // Splits the reply object received from bitcoin-qt into error and result objects
    void ProcessRpcString(QSharedPointer<QByteArray> jsonString, QString& id, QJsonValue& error, QJsonValue& result);

    void GetInfo();

    int64_t GetBalance(QString account = NULL);

    // Gets the default address for the specified account
    QString GetAccountAddress(QString account = NULL);

    // Returns list of all addresses belonging to account
    QStringList GetAddressesByAccount(QString account = "");    // list addresses for account, "" is the default account.

    // Add new address to account
    QString GetNewAddress(QString account = NULL);

    // Validate an address
    BtcAddressInfoRef ValidateAddress(const QString& address);

    // Get private key for address (calls DumpPrivKey())
    QString GetPrivateKey(QString address);

    // Get private key for address
    QString DumpPrivKey(QString address);

    // Adds an address requiring n keys to sign before it can spend its inputs
    // nRequired: number of signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    // account [optional]: account to add the address to
    // Returns the multi-sig address
    BtcMultiSigAddressRef AddMultiSigAddress(int nRequired, QStringList keys, QString account = NULL);

    // Creates a multi-sig address without adding it to the wallet
    // nRequired: signatures required
    // keys: list of public keys (addresses work too, if the public key is known)
    BtcMultiSigAddressRef CreateMultiSigAddress(int nRequired, QStringList keys);

    // Creates a multi-sig address and returns its redeemScript
    // the address will not be added to your address list, use AddMultiSigAddress for that
    QString GetRedeemScript(int nRequired, QStringList keys);

    // Returns list of account names
    // Could also return the balance of each account but I find that confusing
    QStringList ListAccounts();

    QString SendToAddress(QString btcAddress, int64_t amount);
    
    // Send to multiple addresses at once
    // txTargets maps amounts (double) to addresses (QString)
    QString SendMany(QVariantMap txTargets, QString fromAccount = NULL);

    bool SetTxFee(int64_t fee);

    BtcTransactionRef GetTransaction(QString txID);

    QString GetRawTransaction(QString txID);

    BtcRawTransactionRef GetDecodedRawTransaction(QString txID);

    BtcRawTransactionRef DecodeRawTransaction(QString rawTransaction);

    QString CreateRawTransaction(QList<BtcOutput> unspentOutputs, QVariantMap txTargets);

    BtcSignedTransactionRef SignRawTransaction(QString rawTransaction, QList<BtcSigningPrequisite> previousTransactions = QList<BtcSigningPrequisite>(), QStringList privateKeys = QStringList());

    BtcSignedTransactionRef CombineSignedTransactions(QString rawTransaction);

    QString SendRawTransaction(QString rawTransaction);

    QList<QString> GetRawMemPool();

    int GetBlockCount();

    QString GetBlockHash(int blockNumber);

    BtcBlockRef GetBlock(QString blockHash);

private:
     QByteArray CreateJsonQuery(QString command, QJsonArray params = QJsonArray(), QString id = "");


     QMap<QString, QJsonObject> rpcReplies;
};

#endif // JSON_H
