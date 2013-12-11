#ifndef SAMPLETYPEDEFS_H
#define SAMPLETYPEDEFS_H

#include <QSharedPointer>

class SampleEscrowServer;
typedef QSharedPointer<SampleEscrowServer> SampleEscrowServerRef;

class SampleEscrowClient;
typedef QSharedPointer<SampleEscrowClient> SampleEscrowClientRef;

class SampleEscrowTransaction;
typedef QSharedPointer<SampleEscrowTransaction> SampleEscrowTransactionRef;

class EscrowPool;
typedef QSharedPointer<EscrowPool> EscrowPoolRef;


#endif // SAMPLETYPEDEFS_H
