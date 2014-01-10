#include "btcmodules.h"
#include "btcrpccurl.h"
#include "btcjson.h"
#include "MTBitcoin.h"

BtcModulesRef BtcModules::staticInstance;

BtcModules::BtcModules()
{
    this->btcRpc = BtcRpcCurlRef(new BtcRpcCurl(this));
    this->btcJson = BtcJsonRef(new BtcJson(this));
    this->btcHelper = BtcHelperRef(new BtcHelper(this));

    this->mtBitcoin = MTBitcoinRef(new MTBitcoin(this));

    this->staticInstance = BtcModulesRef(this);
}

BtcModules::~BtcModules()
{
    this->mtBitcoin.reset();

    this->btcHelper.reset();
    this->btcJson.reset();
    this->btcRpc.reset();

    this->staticInstance.reset();
}
