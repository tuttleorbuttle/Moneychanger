#ifndef POOLMANAGER_H
#define POOLMANAGER_H

#include <QObject>
#include<QList>
#include "escrowpool.h"

class PoolManager : QObject
{
public:
    PoolManager();

    void AddPool(EscrowPoolRef pool);

    void RemovePool(EscrowPoolRef poolToRemove);

    QList<EscrowPoolRef> escrowPools;   // available pools

private:
};

typedef QSharedPointer<PoolManager> PoolManagerRef;

#endif // POOLMANAGER_H
