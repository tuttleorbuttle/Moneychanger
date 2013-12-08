#ifndef UTILS_H
#define UTILS_H

#ifdef Q_OS_WIN
#include <windows.h>    // for Sleep
#else
#include <time.h>       // for nanosleep
#endif

#include <QMutex>
#include <QWaitCondition>

// code from http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt

// utilities
namespace utils
{
    // converts a double bitcoin (as received through json api) to int64 satoshis
    int64_t CoinsToSatoshis(double value);

    // converts int64 satoshis to double bitcoin
    double SatoshisToCoins(int64_t value);

    // copy of QTest::qSleep() so we don't have to include all of QtTest module.
    // time: time to wait in ms
    void qSleep(int time);

    // this class might be better as it supposedly doesn't block GUI events.
    class SleepSimulator
    {
         QMutex localMutex;
         QWaitCondition sleepSimulator;
    public:
        SleepSimulator()
        {
            localMutex.lock();
        }
        void sleep(unsigned long sleepMS)
        {
            sleepSimulator.wait(&localMutex, sleepMS);
        }
        void CancelSleep()
        {
            sleepSimulator.wakeAll();
        }
    };
}

#endif // UTILS_H
