#include "btcinterface.h"
#include "btcjson.h"
#include "modules.h"

BtcInterface::BtcInterface(QObject *parent) :
    QObject(parent)
{
}

// order of keys doesn't really matter
QString BtcInterface::CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey)
{
    QJsonArray keys;
    keys.append(myKey); keys.append(hisKey);
    return Modules::json->AddMultiSigAddress(2, keys, "moneychanger-twooftwo");
}
