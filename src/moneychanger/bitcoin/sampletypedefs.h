#ifndef SAMPLETYPEDEFS_H
#define SAMPLETYPEDEFS_H

#include <tr1/memory>

class SampleEscrowServer;
typedef std::shared_ptr<SampleEscrowServer> SampleEscrowServerRef;

class SampleEscrowClient;
typedef std::shared_ptr<SampleEscrowClient> SampleEscrowClientRef;

class SampleEscrowTransaction;
typedef std::shared_ptr<SampleEscrowTransaction> SampleEscrowTransactionRef;

class EscrowPool;
typedef std::shared_ptr<EscrowPool> EscrowPoolRef;


#endif // SAMPLETYPEDEFS_H
