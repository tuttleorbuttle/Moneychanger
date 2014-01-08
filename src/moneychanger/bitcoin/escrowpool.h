#ifndef ESCROWPOOL_H
#define ESCROWPOOL_H

#include <tr1/memory>
#include <QList>
#include <QString>
//#include "sampleescrowserver.h"

class SampleEscrowServer;
typedef std::shared_ptr<SampleEscrowServer> SampleEscrowServerRef;

class EscrowPool
{
public:
    EscrowPool();

    // add server to pool
    void AddEscrowServer(SampleEscrowServerRef server);

    void RemoveEscrowServer(SampleEscrowServerRef server);

    QList<SampleEscrowServerRef> escrowServers;     // servers that are part of this pool

    QString poolName;

private:

};

typedef std::shared_ptr<EscrowPool> EscrowPoolRef;

#endif // ESCROWPOOL_H
