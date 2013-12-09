#ifndef SAMPLEESCROWMANAGER_H
#define SAMPLEESCROWMANAGER_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "btcrpc.h"
#include "sampletypedefs.h"
#include "Widgets/btcguitest.h"

class SampleEscrowManager : public QObject
{
public:
    SampleEscrowManager();

    void OnSimulateEscrowServers();

    void OnInitializeEscrow(BtcGuiTest* btcGuiTest);

    void OnRequestWithdrawal(BtcGuiTest* BtcGuiTest);

private:    
    SampleEscrowClientRef client;
    std::list<SampleEscrowServerRef> servers;

//    Q_OBJECT
//public slots:
};

#endif // SAMPLEESCROWMANAGER_H
