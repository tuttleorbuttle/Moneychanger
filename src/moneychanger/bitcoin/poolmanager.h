#ifndef POOLMANAGER_H
#define POOLMANAGER_H

#include "escrowpool.h"
#include<QList>

class PoolManager
{
public:
    PoolManager();

    void AddPool(EscrowPoolRef pool);

    void RemovePool(EscrowPoolRef poolToRemove);

private:
    QList<EscrowPoolRef> escrowPools;   // available pools
};

#endif // POOLMANAGER_H
