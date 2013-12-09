#ifndef SAMPLEESCROWSERVER_H
#define SAMPLEESCROWSERVER_H

#include <QSharedPointer>
#include "bitcoin/btcrpc.h"
#include "bitcoin/sampleescrowclient.h"
#include "bitcoin/btcjsonobjects.h"


class SampleEscrowServer
{
public:
    SampleEscrowServer(BitcoinServerRef rpcServer);

    // called when someone wants to make a deposit
    void OnRequestEscrowDeposit(SampleEscrowClient* client);

    // called by other servers in the pool so they get eachothers' public keys for multi-sig
    std::string GetMultiSigPubKey();

    // called when the client tells the server the tx id of the incoming transaction
    void OnIncomingDeposit(std::string txId);

    bool CheckIncomingTransaction();

    void OnRequestEscrowWithdrawal(SampleEscrowClient* client);

    std::list<SampleEscrowServerRef> serverPool;

    std::string addressForMultiSig;     // this server's address used to create the multi-sig

    std::string pubKeyForMultiSig;      // public key of the address used to create the multi-sig

    std::list<std::string> publicKeys;         // public keys of this and all servers to create multi-sig

    std::string multiSigAddress;        // the actual multisig address

    BtcMultiSigAddressRef multiSigAddrInfo; // info required to withdraw from the address

    SampleEscrowTransactionRef transactionDeposit;      // info about deposit
    SampleEscrowTransactionRef transactionWithdrawal;   // info about withdrawal


private:
    BitcoinServerRef rpcServer;     // login info for bitcoin-qt rpc

    int minSignatures = 2;          // minimum required signatures

    int minConfirms = 2;            // minimum required confirmations
};


#endif // SAMPLEESCROWSERVER_H
