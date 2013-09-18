#include "modules.h"


QScopedPointer<BtcRpc> Modules::bitcoinRpc;
QScopedPointer<BtcJson> Modules::json;
QScopedPointer<BtcInterface> Modules::btcInterface;


Modules::Modules(QObject *parent) :
    QObject(parent)
{
    Modules::bitcoinRpc.reset(new BtcRpc());
    Modules::json.reset(new BtcJson());
    Modules::btcInterface.reset(new BtcInterface());
    json->Initialize();
}

Modules::~Modules()
{

}
