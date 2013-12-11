#include "transactionmanager.h"

TransactionManager::TransactionManager()
{
    this->transactions = QList<SampleEscrowTransactionRef>();
}

void TransactionManager::AddTransaction(SampleEscrowTransactionRef transaction)
{
    if(transaction == NULL || this->transactions.contains(transaction))
        return;

    this->transactions.append(transaction);
}

void TransactionManager::RemoveTransaction(SampleEscrowTransactionRef transaction)
{
    this->transactions.removeAll(transaction);
}
