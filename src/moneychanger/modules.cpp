#include "modules.h"


QScopedPointer<BtcRpc> Modules::btcRpcQt;
QScopedPointer<BtcJsonQt> Modules::btcJsonQt;
QScopedPointer<BtcInterface> Modules::btcInterface;
QScopedPointer<SampleEscrowManager> Modules::sampleEscrowManager;
QScopedPointer<PoolManager> Modules::poolManager;
QScopedPointer<TransactionManager> Modules::transactionManager;
//QScopedPointer<MTBitcoin> Modules::mtBitcoin;


Modules::Modules()
{
    Modules::btcRpcQt.reset(new BtcRpc());
    Modules::btcJsonQt.reset(new BtcJsonQt());
    Modules::btcInterface.reset(new BtcInterface());
    Modules::sampleEscrowManager.reset(new SampleEscrowManager());
    Modules::poolManager.reset(new PoolManager());
    Modules::transactionManager.reset(new TransactionManager());
    //Modules::mtBitcoin.reset(new MTBitcoin());
    btcJsonQt->Initialize();
}

Modules::~Modules()
{

}
