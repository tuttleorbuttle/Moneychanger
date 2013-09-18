#include <OTLog.h>
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

bool BtcInterface::TestBtcJson()
{
    //Modules::bitcoinRpc->ConnectToBitcoin("admin1", "123", "http://127.0.0.1", 19001);
    Modules::bitcoinRpc->ConnectToBitcoin("moneychanger", "money1234");
    Modules::json->GetInfo();
    double balance = Modules::json->GetBalance();
    QStringList accounts = Modules::json->ListAccounts();
    QString address = Modules::json->GetNewAddress();
    QStringList keys;
    keys.append(Modules::json->GetNewAddress());    // key can be an address or public key
    keys.append(Modules::json->GetNewAddress());
    QString multiSigAddr = Modules::json->AddMultiSigAddress(2, QJsonArray::fromStringList(keys));  // maybe add them to account moneychanger-multisig or w/e to keep an overview
    Modules::json->GetInfo();

    OTLog::vOutput(0, "Balance: %f\n", balance);
    if(address != NULL)
        OTLog::vOutput(0, "New address created: \"%s\"\n", address.toStdString().c_str());
    if(multiSigAddr != NULL)
        OTLog::vOutput(0, "Multisig address created: \"%s\"\n", multiSigAddr.toStdString().c_str());
}
