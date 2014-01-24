#ifndef BTCMODULES_H
#define BTCMODULES_H

#include <tr1/memory>
#include "ibtcjson.h"
#include "ibtcrpc.h"
#include "imtbitcoin.h"
#include "btchelper.h"


/*
*
* * MTBitcoin provides a clean interface for all important bitcoin functions
*   it is mostly a wrapper for BtcHelper
*
* * BtcHelper automates abstract tasks like verifying multisig transactions
*   by calling BtcJson functions and processing the results
*
* * BtcJson provides Bitcoin API functions
*   it creates Json strings, sends them to bitcoin's http interface via BtcRpcCurl and parses the results
*   it returns strings, int64 or smart pointers to structures if Json objects are returned
*
* * BtcRpcCurl uses libcurl to connect to bitcoin's http interface
*   it only has a few functions so should be easy to replace with a class using zmq
*
* * BtcModules is a class holding pointers to the instances of the other classes
*   it isn't really good for anything but makes things a bit easier imho
*
* * btcobjects.h is a collection of various structs
*   most of them imitate the objects returned in bitcoind's replies
*
*/


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
