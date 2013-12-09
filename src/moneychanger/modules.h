#ifndef MODULES_H
#define MODULES_H

#include <QScopedPointer>
#include "bitcoin/btcjson.h"
#include "bitcoin/btcrpc.h"
#include "bitcoin/btcinterface.h"
#include "bitcoin/sampleescrowmanager.h"
#include "bitcoin/MTBitcoin.h"

// This class will hold pointers to various modules so they can access eachother.
// Hierarchic layout would be possible too: BtcInterface -> BtcJson -> BtcRpc
class Modules
{
public:
    Modules();
    ~Modules();

    // TODO: _maybe_ overload the :: operator to check if the pointer isn't NULL
    // and maybe use QSharedPointer?
    static QScopedPointer<BtcJson> btcJson;
    static QScopedPointer<BtcRpc> btcRpc;
    static QScopedPointer<BtcInterface> btcInterface;
    static QScopedPointer<SampleEscrowManager> sampleEscrowManager;
    static QScopedPointer<MTBitcoin> mtBitcoin;
};

#endif // MODULES_H
