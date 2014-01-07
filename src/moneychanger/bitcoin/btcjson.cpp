#include "btcjson.h"
#include <jsoncpp/json/json.h>
#include <OTLog.h>

BtcJson::BtcJson()
{
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(0, 0, root);

    if(!root.isObject())
        return;

    Json::Value error = root.get("error", Json::Value());
}

void BtcJson::Initialize()
{

}

bool BtcJson::ProcessRpcString(BtcRpcPacketRef jsonString, Json::Value &result)
{
    std::string id;
    Json::Value error;
    ProcessRpcString(jsonString, id, error, result);

    return error.isNull();  // we should rather throw exceptions but I've never done that in c++ so this will have to do for now.
}

void BtcJson::ProcessRpcString(BtcRpcPacketRef jsonString, std::string &id, Json::Value &error, Json::Value &result)
{
    if(jsonString == NULL || jsonString->data == NULL || jsonString->size <= 0)
        return;
    Json::Value replyObj;
    Json::Reader reader;
    if(!reader.parse(jsonString->data, jsonString->data + jsonString->size - 1, replyObj))
        return;

    if(replyObj.isNull() || replyObj.empty())
        return;

    Json::StyledWriter writer;
    OTLog::vOutput(0, "Received JSON:\n%s\n", writer.write(replyObj).c_str());

    if(replyObj.isObject())
    {
        Json::Value idVal = replyObj["id"];    // this is the same ID we sent to bitcoin-qt earlier.
        id = idVal.isString() ? idVal.asString() : "";      // assign to reference parameter

        error = replyObj["error"];
        if(!error.isNull())
        {
            OTLog::vOutput(0, "Error in reply to \"%s\": %s\n\n", id.c_str(), error.isObject() ? (error["message"]).asString().c_str() : "");
        }

        result = replyObj["result"];
    }
}

BtcRpcPacketRef BtcJson::CreateJsonQuery(const std::string &command, Json::Value params, std::string id)
{
    if(id.empty())
        id = command;
    Json::Value jsonObj = Json::Value();
    jsonObj["jsonrpc"] = 1.0;
    jsonObj["id"] = id;
    jsonObj["method"] = command;
    jsonObj["params"] = params;
    //"{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";

    Json::FastWriter writer;
    return BtcRpcPacketRef(new BtcRpcPacket(writer.write(jsonObj)));
}

void BtcJson::GetInfo() // if we ever need this for anything we can return a struct holding all the information
{
    reply = Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    std::string id;
    Json::Value error;
    Json::Value result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    Json::Value resultObj = result.toObject();
    int64_t balance = BtcHelper::CoinsToSatoshis(resultObj["balance"].toDouble());
}

int64_t BtcJson::GetBalance(std::string account/*=NULL*/)
{
    // note: json and bitcoin-qt make a difference between NULL-strings and empty strings.

    QJsonArray params = QJsonArray();
    params.append(account);      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    QSharedPointer<QByteArray> reply = Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    Json::Value result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;  // error, TODO: throw error or return NaN
    }

    return BtcHelper::CoinsToSatoshis(result.toDouble());
}

std::string BtcJson::GetAccountAddress(std::string account/*= NULL*/)
{
    QJsonArray params = QJsonArray();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
    {
        return NULL;     // error
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

std::stringList BtcJson::GetAddressesByAccount(std::string account)
{
    // not yet implemented
    return std::stringList();
}

std::string BtcJson::GetNewAddress(std::string account/*=NULL*/)
{
    QJsonArray params = QJsonArray();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return NULL;
    }

    if(!result.isString())
        return NULL;     // this should never happen unless the protocol was changed

    return result.toString();
}

BtcAddressInfoRef BtcJson::ValidateAddress(const std::string &address)
{
    QJsonArray params = QJsonArray();
    params.append(address);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_VALIDATEADDRESS, params)), result))
        return BtcAddressInfoRef();

    if(!result.isObject())
        return BtcAddressInfoRef();   // shouldn't happen unless protocol is changed

    BtcAddressInfoRef addressInfo = BtcAddressInfoRef(new BtcAddressInfo(result.toObject()));
    return addressInfo;
}

std::string BtcJson::GetPublicKey(const std::string &address)
{
    BtcAddressInfoRef addrInfo = ValidateAddress(address);
    if(addrInfo == NULL)
        return NULL;

    return addrInfo->pubkey;
}

std::string BtcJson::GetPrivateKey(const std::string &address)
{
    return DumpPrivKey(address);
}

std::string BtcJson::DumpPrivKey(std::string address)
{
    QJsonArray params;
    params.append(address);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_DUMPPRIVKEY, params)),
                result))
        return NULL;

    return result.toString();
}

BtcMultiSigAddressRef BtcJson::AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account)
{
    QJsonArray params = QJsonArray();
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return BtcMultiSigAddressRef();      // error

    if(!result.isString())
        return BtcMultiSigAddressRef();      // shouldn't happen unless protocol is changed

    if(result.toString().isEmpty())
        return BtcMultiSigAddressRef();     // error, shouldn't happen at all

    // re-create the address so we get more information and return that
    return CreateMultiSigAddress(nRequired, keys);
}

BtcMultiSigAddressRef BtcJson::CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys)
{
    QJsonArray params = QJsonArray();
    params.append(nRequired);
    params.append(QJsonArray::fromStringList(keys));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_CREATEMULTISIG, params)),result))
        return BtcMultiSigAddressRef();     // error

    if(!result.isObject())
        return BtcMultiSigAddressRef();     // error

    BtcMultiSigAddressRef multiSigAddr = BtcMultiSigAddressRef(new BtcMultiSigAddress(result.toObject(), keys));
    return multiSigAddr;
}

std::string BtcJson::GetRedeemScript(int nRequired, std::list<std::string> keys)
{
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

std::list<std::string> BtcJson::ListAccounts()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return std::stringList();     // error

    if(!result.isObject())
        return std::stringList();        // this shouldn't happen unless the protocol was changed

    Json::Value accountsObj = result.toObject();
    return accountsObj.keys();      // each key is an account, each value is the account's balance
}


std::string BtcJson::SendToAddress(const std::string &btcAddress, int64_t amount)
{
    // TODO: handle lack of funds, need of transaction fees and unlocking of the wallet

    QJsonArray params = QJsonArray();
    params.append(btcAddress);
    params.append(BtcHelper::SatoshisToCoins(amount));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return NULL;   // error

    if(!result.isString())
        return NULL;    // shouldn't happen unless protocol was changed

    return result.toString();
}

bool BtcJson::SetTxFee(int64_t fee)
{
    QJsonArray params = QJsonArray();
    params.append(BtcHelper::SatoshisToCoins(fee));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SETTXFEE, params)),
                result))
        return false;

    return true;    // todo: check for more errors
}

std::string BtcJson::SendMany(std::map<std::string, int64_t> txTargets, std::string fromAccount)
{
    QJsonArray params = QJsonArray();
    params.append(fromAccount);     // account to send coins from

    Json::Value txTargetsObj = Json::Value();       // convert the int satoshis to double BTC
    for(QMap<std::string, int64_t>::Iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i.key()] = BtcHelper::SatoshisToCoins(i.value());
    }
    params.append(txTargetsObj);    // append map of addresses and btc amounts

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SENDMANY, params)),
                result))
        return NULL;

    if(!result.isString())
        return NULL;

    return result.toString();
}

BtcTransactionRef BtcJson::GetTransaction(std::string txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    QJsonArray params = QJsonArray();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionRef();    // error

    if(!result.isObject())
        return BtcTransactionRef();

    BtcTransactionRef transaction = BtcTransactionRef(new BtcTransaction(result.toObject()));
    return transaction;

    // TODO:
    // for checking balance see "details"->received, confirmations, amount (and address(es)?)

    // also check what happens in multi-sig-transactions.
}

std::string BtcJson::GetRawTransaction(std::string txID)
{
    QJsonArray params = QJsonArray();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return NULL;    // error

    if(!result.isString())
        return NULL;    // error

    return result.toString();
}

BtcRawTransactionRef BtcJson::GetDecodedRawTransaction(std::string txID)
{
    QJsonArray params = QJsonArray();
    params.append(txID);
    params.append(1);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction = BtcRawTransactionRef(new BtcRawTransaction(result.toObject()));
    return decodedRawTransaction;


}

BtcRawTransactionRef BtcJson::DecodeRawTransaction(std::string rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_DECODERAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction = BtcRawTransactionRef(new BtcRawTransaction(result.toObject()));
    return decodedRawTransaction;
}

std::string BtcJson::CreateRawTransaction(const std::list<BtcOutput> &unspentOutputs, std::map<std::string, int64_t> txTargets)
{
    QJsonArray params = QJsonArray();
    QJsonArray outputsArray = QJsonArray();
    for(int i = 0; i < unspentOutputs.size(); i++)
    {
        outputsArray.append(unspentOutputs[i]);
    }
    params.append(outputsArray);

    Json::Value txTargetsObj = Json::Value();
    for(QMap<std::string, int64_t>::Iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i.key()] = BtcHelper::SatoshisToCoins(i.value());
    }
    params.append(txTargetsObj);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_CREATERAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

BtcSignedTransactionRef BtcJson::SignRawTransaction(const std::string &rawTransaction, std::list<BtcSigningPrequisite> previousTransactions, std::privateKeys)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    // add array of unspent outputs
    if(previousTransactions.size() > 0)
    {
        QJsonArray unspentArray = QJsonArray();
        for(int i = 0; i < previousTransactions.size(); i++)
        {
            unspentArray.append(previousTransactions[i]);
        }
        params.append((unspentArray));
    }
    else
    {
        params.append(Json::Value());            // append null or else it won't work
    }

    // add array of private keys used to sign the transaction
    if(privateKeys.size() > 0)
    {
        QJsonArray privKeysArray;               // TODO: figure out how to properly parse a string list
        for(int i = 0; i < privateKeys.size(); i++)
        {
            privKeysArray.append(privateKeys[i]);
        }
        params.append(privKeysArray);
    }
    else
        params.append(Json::Value());            // if no private keys were given, append null


    // params.append(ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    if(!result.isObject())
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction = BtcSignedTransactionRef(new BtcSignedTransaction(result.toObject()));
    return signedTransaction;
}

BtcSignedTransactionRef BtcJson::CombineSignedTransactions(std::string rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);  // a concatenation of partially signed tx's
    params.append(QJsonArray());    // dummy inputs and redeemscripts (must not be 'null')
    params.append(QJsonArray());    // dummy private keys (must not be 'null')
    // if dummy inputs are 'null', bitcoin will not just combine the raw transaction but do additional signing(!!!)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction = BtcSignedTransactionRef(new BtcSignedTransaction(result.toObject()));
    return signedTransaction;
}

std::string BtcJson::SendRawTransaction(const std::string &rawTransaction)
{
    QJsonArray params = QJsonArray();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return NULL;  // error

    return result.toString();
}

std::list<std::string> BtcJson::GetRawMemPool()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWMEMPOOL, QJsonArray())), result))
        return QList<std::string>();

    if(!result.isArray())
        return QList<std::string>();

    QJsonArray resultArray = result.toArray();
    QList<std::string> rawMemPool = QList<std::string>();
    for(int i = 0; i < resultArray.size(); i++)
    {
        rawMemPool += resultArray[i].toString();
    }
    return rawMemPool;
}

int BtcJson::GetBlockCount()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKCOUNT, QJsonArray())), result))
        return -1;   // we should throw errors instead.

    return (int)result.toDouble(-1);
}

std::string BtcJson::GetBlockHash(int blockNumber)
{
    QJsonArray params = QJsonArray();
    params.append(blockNumber);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return NULL;

    return result.toString();
}

BtcBlockRef BtcJson::GetBlock(const std::string &blockHash)
{
    QJsonArray params = QJsonArray();
    params.append(blockHash);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                Modules::btcRpcQt->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCK, params)), result))
        return BtcBlockRef();

    if(!result.isObject())
        return BtcBlockRef();

    BtcBlockRef block = BtcBlockRef(new BtcBlock(result.toObject()));
    return block;
}
