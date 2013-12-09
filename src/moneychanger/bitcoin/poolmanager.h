#ifndef POOLMANAGER_H
#define POOLMANAGER_H

#include "escrowpool.h"
#include<QList>

class PoolManager
{
public:
    PoolManager();

    EscrowPoolRef AddPool();

    void RemovePool(EscrowPoolRef poolToRemove);

private:
    QList<EscrowPoolRef> escrowPools;
};

#endif // POOLMANAGER_H
