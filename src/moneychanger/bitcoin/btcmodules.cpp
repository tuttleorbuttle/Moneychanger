#include "btcmodules.h"

BtcModules::BtcModules()
{
    this->btcRpc = BtcRpcCurlRef(new BtcRpcCurl(this));
    this->btcJson = BtcJsonRef(new BtcJson(this));
    this->btcHelper = BtcHelperRef(new BtcHelper(this));

    this->mtBitcoin = MTBitcoinRef(new MTBitcoin(this));
}
