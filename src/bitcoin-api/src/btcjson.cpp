#include "btcjson.h"
#include <jsoncpp/json/json.h>
#include <OTLog.h>
#include <map>

// https://en.bitcoin.it/wiki/Proper_Money_Handling_%28JSON-RPC%29 on how to avoid rounding errors and such. might be worth a read someday.

// bitcoin rpc methods
#define METHOD_GETINFO              "getinfo"
#define METHOD_GETBALANCE           "getbalance"
#define METHOD_GETACCOUNTADDRESS    "getaccountaddress"
#define METHOD_GETNEWADDRESS        "getnewaddress"
#define METHOD_VALIDATEADDRESS      "validateaddress"
#define METHOD_DUMPPRIVKEY          "dumpprivkey"
#define METHOD_LISTACCOUNTS         "listaccounts"
#define METHOD_SENDTOADDRESS        "sendtoaddress"
#define METHOD_SENDMANY             "sendmany"
#define METHOD_SETTXFEE             "settxfee"
#define METHOD_ADDMULTISIGADDRESS   "addmultisigaddress"
#define METHOD_CREATEMULTISIG       "createmultisig"
#define METHOD_GETTRANSACTION       "gettransaction"
#define METHOD_GETRAWTRANSACTION    "getrawtransaction"
#define METHOD_DECODERAWTRANSACTION "decoderawtransaction"
#define METHOD_CREATERAWTRANSACTION "createrawtransaction"
#define METHOD_SIGNRAWTRANSACTION   "signrawtransaction"
#define METHOD_SENDRAWTRANSACTION   "sendrawtransaction"
#define METHOD_GETRAWMEMPOOL        "getrawmempool"
#define METHOD_GETBLOCKCOUNT        "getblockcount"
#define METHOD_GETBLOCKHASH         "getblockhash"
#define METHOD_GETBLOCK             "getblock"

BtcJson::BtcJson(BtcModules *modules)
{
    this->modules = modules;
}

BtcJson::~BtcJson()
{
    this->modules = NULL;
}

void BtcJson::Initialize()
{

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
    Json::StyledWriter debugWriter;
    OTLog::Output(0, debugWriter.write(jsonObj).c_str());
    return BtcRpcPacketRef(new BtcRpcPacket(writer.write(jsonObj)));
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
        Json::Value idVal = replyObj["id"];    // this is the same ID we sent to bitcoin- earlier.
        id = idVal.isString() ? idVal.asString() : "";      // assign to reference parameter

        error = replyObj["error"];
        if(!error.isNull())
        {
            OTLog::vOutput(0, "Error in reply to \"%s\": %s\n\n", id.c_str(), error.isObject() ? (error["message"]).asString().c_str() : "");
        }

        result = replyObj["result"];
    }
}

void BtcJson::GetInfo()
{
    BtcRpcPacketRef reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETINFO));
    std::string id;
    Json::Value error;
    Json::Value result;
    ProcessRpcString(reply, id, error, result);

    if(!error.isNull())
        return;

    int64_t balance = this->modules->btcHelper->CoinsToSatoshis(result["balance"].asDouble());
}

int64_t BtcJson::GetBalance(const char *account/*=NULL*/)
{
    // note: json and bitcoin- make a difference between NULL-strings and empty strings.

    Json::Value params = Json::Value();
    params.append(account != NULL ? account : Json::Value());      // account
    //params.append(1);       // min confirmations, 1 is default, we probably don't need this line.

    BtcRpcPacketRef reply = this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETBALANCE));

    Json::Value result;
    if(!ProcessRpcString(reply, result) || !result.isDouble())
    {
        return 0;  // error, TODO: throw error or return NaN
    }

    return this->modules->btcHelper->CoinsToSatoshis(result.asDouble());
}

std::string BtcJson::GetAccountAddress(const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETACCOUNTADDRESS, params)), result))
    {
        return "";     // error
    }

    if(!result.isString())
        return "";     // this should never happen unless the protocol was changed

    return result.asString();
}

std::stringList BtcJson::GetAddressesByAccount(const std::string &account)
{
    // not yet implemented
    return std::stringList();
}

std::string BtcJson::GetNewAddress(const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_GETNEWADDRESS, params)), result))
    {
        return "";
    }

    if(!result.isString())
        return "";     // this should never happen unless the protocol was changed

    return result.asString();
}

BtcAddressInfoRef BtcJson::ValidateAddress(const std::string &address)
{
    Json::Value params = Json::Value();
    params.append(address);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_VALIDATEADDRESS, params)), result))
        return BtcAddressInfoRef();

    if(!result.isObject())
        return BtcAddressInfoRef();   // shouldn't happen unless protocol is changed

    BtcAddressInfoRef addressInfo = BtcAddressInfoRef(new BtcAddressInfo(result));
    return addressInfo;
}

std::string BtcJson::GetPublicKey(const std::string &address)
{
    BtcAddressInfoRef addrInfo = ValidateAddress(address);
    if(addrInfo == NULL)
        return "";

    return addrInfo->pubkey;
}

std::string BtcJson::GetPrivateKey(const std::string &address)
{
    return DumpPrivKey(address);
}

std::string BtcJson::DumpPrivKey(const std::string &address)
{
    Json::Value params;
    params.append(address);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DUMPPRIVKEY, params)),
                result))
        return "";

    return result.asString();
}

BtcMultiSigAddressRef BtcJson::AddMultiSigAddress(int nRequired, const std::list<std::string> &keys, const std::string &account)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(std::list<std::string>::const_iterator i = keys.begin(); i != keys.end(); i++)
    {
        keysValue.append(*i);
    }
    params.append(keysValue);
    params.append(account);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_ADDMULTISIGADDRESS, params)), result))
        return BtcMultiSigAddressRef();      // error

    if(!result.isString())
        return BtcMultiSigAddressRef();      // shouldn't happen unless protocol is changed

    if(result.asString().empty())
        return BtcMultiSigAddressRef();     // error, shouldn't happen at all

    // re-create the address so we get more information and return that
    return CreateMultiSigAddress(nRequired, keys);
}

BtcMultiSigAddressRef BtcJson::CreateMultiSigAddress(int nRequired, const std::list<std::string> &keys)
{
    Json::Value params = Json::Value();
    params.append(nRequired);
    Json::Value keysValue;
    for(std::list<std::string>::const_iterator i = keys.begin(); i != keys.end(); i++)
    {
        keysValue.append(*i);
    }
    params.append(keysValue);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_CREATEMULTISIG, params)),result))
        return BtcMultiSigAddressRef();     // error

    if(!result.isObject())
        return BtcMultiSigAddressRef();     // error

    BtcMultiSigAddressRef multiSigAddr = BtcMultiSigAddressRef(new BtcMultiSigAddress(result, keys));
    return multiSigAddr;
}

std::string BtcJson::GetRedeemScript(int nRequired, std::list<std::string> keys)
{
    return CreateMultiSigAddress(nRequired, keys)->redeemScript;
}

std::vector<std::string> BtcJson::ListAccounts()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_LISTACCOUNTS)), result))
        return std::vector<std::string>();     // error

    if(!result.isObject())
        return std::vector<std::string>();        // this shouldn't happen unless the protocol was changed

    return result.getMemberNames();      // each key is an account, each value is the account's balance
}


std::string BtcJson::SendToAddress(const std::string &btcAddress, int64_t amount)
{
    // TODO: handle lack of funds, need of transaction fees and unlocking of the wallet

    Json::Value params = Json::Value();
    params.append(btcAddress);
    params.append(this->modules->btcHelper->SatoshisToCoins(amount));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(CreateJsonQuery(METHOD_SENDTOADDRESS, params)),result))
        return "";   // error

    if(!result.isString())
        return "";    // shouldn't happen unless protocol was changed

    return result.asString();
}

bool BtcJson::SetTxFee(int64_t fee)
{
    Json::Value params = Json::Value();
    params.append(this->modules->btcHelper->SatoshisToCoins(fee));

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SETTXFEE, params)),
                result))
        return false;

    return true;    // todo: check for more errors
}

std::string BtcJson::SendMany(std::map<std::string, int64_t> txTargets, const std::string &fromAccount)
{
    Json::Value params = Json::Value();
    params.append(fromAccount);     // account to send coins from

    Json::Value txTargetsObj = Json::Value();       // convert the int satoshis to double BTC
    for(std::map<std::string, int64_t>::iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i->first] = this->modules->btcHelper->SatoshisToCoins(i->second);
    }
    params.append(txTargetsObj);    // append map of addresses and btc amounts

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDMANY, params)),
                result))
        return "";

    if(!result.isString())
        return "";

    return result.asString();
}

BtcTransactionRef BtcJson::GetTransaction(std::string txID)
{
    // TODO: maybe we can automate the process of appending arguments
    //      and calling SendRPC(CreateJsonQuery..) as it's
    //      virtually the same code in every function.
    Json::Value params = Json::Value();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETTRANSACTION, params)), result))
        return BtcTransactionRef();    // error

    if(!result.isObject())
        return BtcTransactionRef();

    BtcTransactionRef transaction = BtcTransactionRef(new BtcTransaction(result));
    return transaction;

    // TODO:
    // for checking balance see "details"->received, confirmations, amount (and address(es)?)

    // also check what happens in multi-sig-transactions.
}

std::string BtcJson::GetRawTransaction(std::string txID)
{
    Json::Value params = Json::Value();
    params.append(txID);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return "";    // error

    if(!result.isString())
        return "";    // error

    return result.asString();
}

BtcRawTransactionRef BtcJson::GetDecodedRawTransaction(std::string txID)
{
    Json::Value params = Json::Value();
    params.append(txID);
    params.append(1);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction = BtcRawTransactionRef(new BtcRawTransaction(result));
    return decodedRawTransaction;


}

BtcRawTransactionRef BtcJson::DecodeRawTransaction(std::string rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_DECODERAWTRANSACTION, params)), result))
        return BtcRawTransactionRef();  // return NULL

    if(!result.isObject())
        return BtcRawTransactionRef();  // return NULL

    BtcRawTransactionRef decodedRawTransaction = BtcRawTransactionRef(new BtcRawTransaction(result));
    return decodedRawTransaction;
}

std::string BtcJson::CreateRawTransaction(const std::list<BtcOutput> &unspentOutputs, std::map<std::string, int64_t> txTargets)
{
    Json::Value params = Json::Value();
    Json::Value outputsArray = Json::Value();
    for(std::list<BtcOutput>::const_iterator i = unspentOutputs.begin(); i != unspentOutputs.end(); i++)
    {
        outputsArray.append(*i);
    }
    params.append(outputsArray);

    Json::Value txTargetsObj = Json::Value();
    for(std::map<std::string, int64_t>::iterator i = txTargets.begin(); i != txTargets.end(); i++)
    {
        txTargetsObj[i->first] = this->modules->btcHelper->SatoshisToCoins(i->second);
    }
    params.append(txTargetsObj);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_CREATERAWTRANSACTION, params)), result))
        return "";  // error

    return result.asString();
}

BtcSignedTransactionRef BtcJson::SignRawTransaction(const std::string &rawTransaction, const std::list<BtcSigningPrequisite> &previousTransactions, const std::stringList &privateKeys)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    // add array of unspent outputs
    if(previousTransactions.size() > 0)
    {
        Json::Value unspentArray = Json::Value();
        for(std::list<BtcSigningPrequisite>::const_iterator i = previousTransactions.begin(); i != previousTransactions.end(); i++)
        {
            unspentArray.append(*i);
        }
        params.append((unspentArray));
    }
    else
    {
        params.append(Json::Value(Json::nullValue));            // append null or else it won't work
    }

    // add array of private keys used to sign the transaction
    if(privateKeys.size() > 0)
    {
        Json::Value privKeysArray;               // TODO: figure out how to properly parse a string list
        for(std::stringList::const_iterator i = privateKeys.begin(); i != privateKeys.end(); i++)
        {
            privKeysArray.append(*i);
        }
        params.append(privKeysArray);
    }
    else
        params.append(Json::Value(Json::nullValue));            // if no private keys were given, append null


    // params.append(ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    if(!result.isObject())
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction = BtcSignedTransactionRef(new BtcSignedTransaction(result));
    return signedTransaction;
}

BtcSignedTransactionRef BtcJson::CombineSignedTransactions(std::string rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);  // a concatenation of partially signed tx's
    params.append(Json::Value(Json::arrayValue));    // dummy inputs and redeemscripts (must not be 'null')
    params.append(Json::Value(Json::arrayValue));    // dummy private keys (must not be 'null')
    // if dummy inputs are 'null', bitcoin will not just combine the raw transaction but do additional signing(!!!)

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SIGNRAWTRANSACTION, params)), result))
        return BtcSignedTransactionRef();   // error

    BtcSignedTransactionRef signedTransaction = BtcSignedTransactionRef(new BtcSignedTransaction(result));
    return signedTransaction;
}

std::string BtcJson::SendRawTransaction(const std::string &rawTransaction)
{
    Json::Value params = Json::Value();
    params.append(rawTransaction);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_SENDRAWTRANSACTION, params)), result))
        return "";  // error

    return result.asString();
}

std::vector<std::string> BtcJson::GetRawMemPool()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETRAWMEMPOOL, Json::Value())), result))
        return std::vector<std::string>();

    if(!result.isArray())
        return std::vector<std::string>();

    std::vector<std::string> rawMemPool = std::vector<std::string>();
    for(uint i = 0; i < result.size(); i++)
    {
        rawMemPool.push_back(result[i].asString());
    }
    return rawMemPool;
}

int BtcJson::GetBlockCount()
{
    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKCOUNT, Json::Value())), result))
        return -1;   // we should throw errors instead.

    return (int)result.asInt();
}

std::string BtcJson::GetBlockHash(int blockNumber)
{
    Json::Value params = Json::Value();
    params.append(blockNumber);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCKHASH, params)), result))
        return "";

    return result.asString();
}

BtcBlockRef BtcJson::GetBlock(const std::string &blockHash)
{
    Json::Value params = Json::Value();
    params.append(blockHash);

    Json::Value result = Json::Value();
    if(!ProcessRpcString(
                this->modules->btcRpc->SendRpc(
                    CreateJsonQuery(METHOD_GETBLOCK, params)), result))
        return BtcBlockRef();

    if(!result.isObject())
        return BtcBlockRef();

    BtcBlockRef block = BtcBlockRef(new BtcBlock(result));
    return block;
}
