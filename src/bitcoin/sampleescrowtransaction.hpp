#ifndef SAMPLEESCROWTRANSACTION_HPP
#define SAMPLEESCROWTRANSACTION_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>
#include <opentxs/TR1_Wrapper.hpp>

#include _CINTTYPES
#include _MEMORY

#include <bitcoin-api/btcmodules.hpp>

#include <string>

//#include "sampleescrowclient.h"
//#include "sampleescrowserver.h"


// this class holds some information about pending transactions
// to save/restore pending transactions we need txId, targetAddr and amountToSend
class SampleEscrowTransaction
{
public:
    SampleEscrowTransaction(int64_t amountToSend, BtcModulesPtr modules);

    bool SendToTarget();

    // checks if transaction is confirmed and sends the correct value to the correct address
    void CheckTransaction(int minConfirms);

    int64_t amountToSend;
    std::string targetAddr;             // address to which funds are sent
    std::string txId;                   // id of the pending transaction
    int64_t confirmations;              // confirmations of pending transaction

    enum SUCCESS
    {
        NotStarted,
        Pending,
        Successfull,
        Conflicted,
        Failed,
        Spent
    }status;

    //SampleEscrowClientPtr client;

private:
    BtcModulesPtr modules;
    //QList<SampleEscrowServerPtr> servers;
};

typedef _SharedPtr<SampleEscrowTransaction> SampleEscrowTransactionPtr;
typedef std::list<SampleEscrowTransactionPtr> SampleEscrowTransactions;

#endif // SAMPLEESCROWTRANSACTION_HPP
