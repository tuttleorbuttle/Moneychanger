#ifndef ESCROWPOOL_H
#define ESCROWPOOL_H

#include <QSharedPointer>
#include <QList>
#include "sampleescrowserver.h"

class EscrowPool
{
public:
    EscrowPool();

    // add server to pool
    void AddEscrowServer(SampleEscrowServerRef server);

    void RemoveEscrowServer(SampleEscrowServerRef server);

private:
    QList<SampleEscrowServerRef> escrowServers;     // servers that are part of this pool
};

typedef QSharedPointer<EscrowPool> EscrowPoolRef;

#endif // ESCROWPOOL_H
