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
    // TODO: _maybe_ free memory again, would break { Modules modules; } initialization but does that even matter?
    int a = 0;
}
