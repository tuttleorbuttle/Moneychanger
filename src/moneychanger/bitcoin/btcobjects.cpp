#include "btcobjects.h"
#include "btchelper.h"


BtcTransaction::BtcTransaction(Json::Value reply)
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

void BtcTransaction::SetDefaults()
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

BtcRawTransaction::BtcRawTransaction(Json::Value rawTx)
{
    this->inputs = std::vector<VIN>();
    this->outputs = std::vector<VOUT>();

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

BtcAddressInfo::BtcAddressInfo(Json::Value result)
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

BtcMultiSigAddress::BtcMultiSigAddress(Json::Value result, const std::stringList &publicKeys)
{
    this->address = result["address"].asString();
    this->redeemScript = result["redeemScript"].asString();

    this->publicKeys = publicKeys;
}

BtcBlock::BtcBlock()
{
    this->confirmations = 0;
    this->transactions = std::list<std::string>();
    this->height = 0;
    this->hash = std::string();
    this->previousHash = std::string();
}

BtcBlock::BtcBlock(Json::Value block)
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

BtcOutput::BtcOutput(std::string txID, int vout)
{
    (*this)["txid"] = txID;
    (*this)["vout"] = vout;
}

BtcSignedTransaction::BtcSignedTransaction(Json::Value signedTransactionObj)
{
    this->signedTransaction = signedTransactionObj["hex"].asString();
    this->complete = signedTransactionObj["complete"].asBool();
}

BtcSigningPrequisite::BtcSigningPrequisite()
{

}

BtcSigningPrequisite::BtcSigningPrequisite(std::string txId, int vout, std::string scriptPubKey, std::string redeemScript)
{
    // all of these values must be set or else prequisite is invalid
    (*this)["txid"] = txId;
    (*this)["vout"] = vout;
    (*this)["scriptPubKey"] = scriptPubKey;
    (*this)["redeemScript"] = redeemScript;
}

void BtcSigningPrequisite::SetTxId(std::string txId)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["txid"] = txId;
}

void BtcSigningPrequisite::SetVout(int vout)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["vout"] = vout;
}

void BtcSigningPrequisite::SetScriptPubKey(std::string scriptPubKey)
{
    // we can get this value from the transaction used to send funds to the p2sh
    (*this)["scriptPubKey"] = scriptPubKey;
}

void BtcSigningPrequisite::SetRedeemScript(std::string redeemScript)
{
    // we can get this from the createmultisig api function
    (*this)["redeemScript"] = redeemScript;
}

BtcRpcPacket::BtcRpcPacket()
{
    this->data = NULL;
    this->size = 0;
}

BtcRpcPacket::BtcRpcPacket(const char *data, int size)
{
    if(data == NULL || size < 0)
    {
        this->data = NULL;
        size = 0;
        return;
    }
    this->data = (char*)realloc(NULL, size);
    memccpy(this->data, data, NULL, size);
    this->size = size;
}

BtcRpcPacket::BtcRpcPacket(const std::string &data)
{
    this->data = (char*)realloc(NULL, data.size());
    memccpy(this->data, data.c_str(), NULL, data.size());
    this->size = data.size();
}

BtcRpcPacket::~BtcRpcPacket()
{
    this->size = 0;
    //if(this->data != NULL)
    //    delete this->data;
    this->data = NULL;
}
















