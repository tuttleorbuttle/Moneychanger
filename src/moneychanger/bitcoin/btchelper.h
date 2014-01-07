#ifndef BTCHELPER_H
#define BTCHELPER_H

#include "sys/types.h"

class BtcHelper
{
public:
    // converts a double bitcoin (as received through json api) to int64 satoshis
    static int64_t CoinsToSatoshis(double value);

    // converts int64 satoshis to double bitcoin
    static double SatoshisToCoins(int64_t value);

private:
};

#endif // BTCHELPER_H
