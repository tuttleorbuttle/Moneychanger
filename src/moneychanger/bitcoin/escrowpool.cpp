#include "escrowpool.h"

EscrowPool::EscrowPool()
{
    this->escrowServers = QList<SampleEscrowServerRef>();

    this->poolName = "unnamed";
}

void EscrowPool::AddEscrowServer(SampleEscrowServerRef server)
{
    if(server == NULL || this->escrowServers.contains(server))
        return;

    this->escrowServers.append(server);
}

void EscrowPool::RemoveEscrowServer(SampleEscrowServerRef server)
{
    this->escrowServers.removeAll(server);
}
