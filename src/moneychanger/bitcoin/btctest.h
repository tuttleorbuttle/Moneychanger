#ifndef BTCTEST_H
#define BTCTEST_H

#include "btcmodules.h"

// will be used as a sort of unit test
class BtcTest
{
public:
    BtcTest();

    static bool TestBitcoinFunctions();

private:
    static bool TestBtcRpc();

    static bool TestBtcJson();

    static bool TestMultiSig();

    static bool TestMultiSigDeposit();

    static bool TestMultiSigWithdrawal();


    static BtcModulesRef modules;

    static std::string multiSigAddress;
    static std::string depositTxId;

};

#endif // BTCTEST_H
