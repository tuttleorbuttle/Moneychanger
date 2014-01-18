#ifndef BTCMODULES_H
#define BTCMODULES_H

#include <tr1/memory>
#include "ibtcjson.h"
#include "ibtcrpc.h"
#include "imtbitcoin.h"
#include "btchelper.h"


class BtcModules;
typedef std::shared_ptr<BtcModules> BtcModulesRef;

class BtcModules
{
public:
    BtcModules();
    ~BtcModules();

    IMTBitcoinRef mtBitcoin; // interface to be used from OT/MC

    IBtcJsonRef btcJson;    // bitcon API json wrapper
    IBtcRpcRef btcRpc;      // RPC call wrapper
    BtcHelperRef btcHelper;    // helper class to do all the complicated stuff

    static BtcModules* staticInstance;
};

#endif // BTCMODULES_H
