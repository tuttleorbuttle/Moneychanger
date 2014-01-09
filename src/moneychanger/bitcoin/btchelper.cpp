#include "btchelper.h"
#include "btcmodules.h"


BtcHelper::BtcHelper(BtcModules *modules)
{
    MinConfirms = 1;

    this->modules = modules;
}

// converts a double bitcoin (as received through json api) to int64 satoshis
int64_t BtcHelper::CoinsToSatoshis(double value)
{
    // copied from the wiki
    return (int64_t)(value * 1e8 + (value < 0.0 ? -.5 : .5));
}

// converts int64 satoshis to double bitcoin
double BtcHelper::SatoshisToCoins(int64_t value)
{
    return (double)value / 1e8;
}
