#ifndef SAMPLEESCROWCLIENT_H
#define SAMPLEESCROWCLIENT_H

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QList>
#include "bitcoin/btcrpc.h"
#include "Widgets/btcguitest.h"
#include "sampletypedefs.h"
#include "sampleescrowserver.h"
#include "sampleescrowtransaction.h"



class SampleEscrowClient : public QObject
{
        Q_OBJECT
public:
    explicit SampleEscrowClient(QObject* parent = 0);
    ~SampleEscrowClient();


    /** Deposit **/
    // client asks servers for public keys
    // amountToSend: amount in satoshis
    void StartDeposit(int64_t amountToSend, std::list<SampleEscrowServerRef> escrowServerPool);

    // client receives public keys
    void OnReceivePubKey(const std::string& publicKey, int minSignatures);

    // returns true when the transaction finished
    bool CheckDepositFinished();

    /** Release **/
    // client asks server for release of escrow
    void StartWithdrawal();

    // client receives partially signed tx
    void OnReceiveSignedTx(const std::string &signedTx);

    bool CheckWithdrawalFinished();

    bool CheckTransactionFinished(SampleEscrowTransactionRef transaction);


    std::list<std::string> pubKeyList;
    int minSignatures;

    int minConfirms;  // minimum number of confirmations before tx is considered successfull

    SampleEscrowTransactionRef transactionDeposit;      // holds info about the transaction to the pool
    SampleEscrowTransactionRef transactionWithdrawal;   // holds info about the transaction from the pool

private:
    BitcoinServerRef rpcServer;
    std::list<SampleEscrowServerRef> escrowServers;

    // client sends bitcoin to escrow
    void SendToEscrow();

    signals:    // used to update the GUI
        // deposit
        void SetMultiSigAddress(const std::string& address);
        void SetTxIdDeposit(const std::string& txId);
        void SetConfirmationsDeposit(int confirms);
        void SetStatusDeposit(SampleEscrowTransaction::SUCCESS status);
        // withdrawal
        void SetWithdrawalAddress(const std::string& address);
        void SetTxIdWithdrawal(const std::string& txId);
        void SetConfirmationsWithdrawal(int confirms);
        void SetStatusWithdrawal(SampleEscrowTransaction::SUCCESS status);
};





#endif // SAMPLEESCROWCLIENT_H
