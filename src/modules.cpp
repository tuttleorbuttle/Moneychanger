#include "modules.h"


QScopedPointer<BtcRpc> Modules::bitcoinRpc;
QScopedPointer<BtcJson> Modules::json;

Modules::Modules(QObject *parent) :
    QObject(parent)
{
    Modules::bitcoinRpc.reset(new BtcRpc());
    Modules::json.reset(new BtcJson());
    json->Initialize();
}

Modules::~Modules()
{
    int a = 0;
}
