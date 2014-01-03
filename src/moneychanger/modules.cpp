#include "modules.h"


QScopedPointer<BtcRpc> Modules::btcRpc;
QScopedPointer<BtcJson> Modules::btcJson;
QScopedPointer<BtcInterface> Modules::btcInterface;
QScopedPointer<SampleEscrowManager> Modules::sampleEscrowManager;
QScopedPointer<PoolManager> Modules::poolManager;
QScopedPointer<TransactionManager> Modules::transactionManager;
QScopedPointer<MTBitcoin> Modules::mtBitcoin;


Modules::Modules()
{
    Modules::btcRpc.reset(new BtcRpc());
    Modules::btcJson.reset(new BtcJson());
    Modules::btcInterface.reset(new BtcInterface());
    Modules::sampleEscrowManager.reset(new SampleEscrowManager());
    Modules::poolManager.reset(new PoolManager());
    Modules::transactionManager.reset(new TransactionManager());
    Modules::mtBitcoin.reset(new MTBitcoin());
    btcJson->Initialize();
}

Modules::~Modules()
{

}