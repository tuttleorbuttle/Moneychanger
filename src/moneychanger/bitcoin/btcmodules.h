#ifndef BTCMODULES_H
#define BTCMODULES_H

#include <tr1/memory>
#include "ibtcjson.h"
#include "ibtcrpc.h"
#include "imtbitcoin.h"
#include "btchelper.h"

class BtcModules
{
public:
    BtcModules();

    IMTBitcoinRef mtBitcoin; // interface to be used from OT/MC

    IBtcJsonRef btcJson;    // bitcon API json wrapper
    IBtcRpcRef btcRpc;      // RPC call wrapper
    BtcHelperRef btcHelper;    // helper class to do all the complicated stuff

};

typedef std::shared_ptr<BtcModules> BtcModulesRef;

#endif // BTCMODULES_H
