#include "btcmodules.h"
#include "btcrpccurl.h"
#include "btcjson.h"
#include "MTBitcoin.h"

BtcModules::BtcModules()
{
    this->btcRpc = BtcRpcCurlRef(new BtcRpcCurl(this));
    this->btcJson = BtcJsonRef(new BtcJson(this));
    this->btcHelper = BtcHelperRef(new BtcHelper(this));

    this->mtBitcoin = MTBitcoinRef(new MTBitcoin(this));
}

BtcModules::~BtcModules()
{
    this->btcRpc.reset();
    this->btcJson.reset();
    this->btcHelper.reset();

    this->mtBitcoin.reset();
}
