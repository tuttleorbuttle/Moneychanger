#include "modules.h"


QScopedPointer<BtcRpc> Modules::btcRpc;
QScopedPointer<BtcJson> Modules::btcJson;
QScopedPointer<BtcInterface> Modules::btcInterface;


Modules::Modules(QObject *parent) :
    QObject(parent)
{
    Modules::btcRpc.reset(new BtcRpc());
    Modules::btcJson.reset(new BtcJson());
    Modules::btcInterface.reset(new BtcInterface());
    btcJson->Initialize();
}

Modules::~Modules()
{

}
