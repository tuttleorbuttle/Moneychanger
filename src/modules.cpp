#include "modules.h"


QScopedPointer<BitcoinRpc> Modules::bitcoinRpc;
QScopedPointer<Json> Modules::json;

Modules::Modules(QObject *parent) :
    QObject(parent)
{
    Modules::bitcoinRpc.reset(new BitcoinRpc());
    Modules::json.reset(new Json());
    json->Initialize();
}

Modules::~Modules()
{
    int a = 0;
}
