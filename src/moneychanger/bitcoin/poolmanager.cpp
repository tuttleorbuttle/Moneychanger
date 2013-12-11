#include "poolmanager.h"


PoolManager::PoolManager()
{

}

void PoolManager::AddPool(EscrowPoolRef pool)
{
    if(pool == NULL || this->escrowPools.contains(pool))
        return;

    this->escrowPools.append(pool);
}

void PoolManager::RemovePool(EscrowPoolRef poolToRemove)
{
    // better safe than sorry
    this->escrowPools.removeAll(poolToRemove);
}
