#include "btcmodules.h"
#include "btcrpccurl.h"
#include "btcjson.h"
#include "MTBitcoin.h"

BtcModules* BtcModules::staticInstance;

BtcModules::BtcModules()
{
    this->btcRpc = BtcRpcCurlRef(new BtcRpcCurl(this));
    this->btcJson = BtcJsonRef(new BtcJson(this));
    this->btcHelper = BtcHelperRef(new BtcHelper(this));

    this->mtBitcoin = MTBitcoinRef(new MTBitcoin(this));

    // added this so the Sample classes in Moneychanger can access it
    // they could instead create their own instance though.
    if(this->staticInstance == NULL)
        this->staticInstance = this;
}

BtcModules::~BtcModules()
{
    this->mtBitcoin.reset();

    this->btcHelper.reset();
    this->btcJson.reset();
    this->btcRpc.reset();

    if(this->staticInstance == this)
        this->staticInstance = NULL;
}
