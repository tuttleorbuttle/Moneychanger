#include "modules.h"


QScopedPointer<BtcRpc> Modules::btcRpc;
QScopedPointer<BtcJson> Modules::btcJson;
QScopedPointer<BtcInterface> Modules::btcInterface;
QScopedPointer<SampleEscrowManager> Modules::sampleEscrowManager;
QScopedPointer<MTBitcoin> Modules::mtBitcoin;


Modules::Modules()
{
    Modules::btcRpc.reset(new BtcRpc());
    Modules::btcJson.reset(new BtcJson());
    Modules::btcInterface.reset(new BtcInterface());
    Modules::sampleEscrowManager.reset(new SampleEscrowManager());
    Modules::mtBitcoin.reset(new MTBitcoin());
    btcJson->Initialize();
}

Modules::~Modules()
{

}
