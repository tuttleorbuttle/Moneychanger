#include "utils.h"


namespace utils
{
    void qSleep(int ms)
    {
        if(ms < 0) return;

    #ifdef Q_OS_WIN
        Sleep(uint(ms));
    #else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
    #endif
    }

    // converts a double bitcoin (as received through json api) to int64 satoshis
    int64_t CoinsToSatoshis(double value)
    {
        // copied from the wiki
        return (int64_t)(value * 1e8 + (value < 0.0 ? -.5 : .5));
    }

    // converts int64 satoshis to double bitcoin
    double SatoshisToCoins(int64_t value)
    {
        return (double)value / 1e8;
    }
}
