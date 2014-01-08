#ifndef SAMPLEESCROWMANAGER_H
#define SAMPLEESCROWMANAGER_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "btcrpcqt.h"
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
    EscrowPoolRef escrowPool;

//    Q_OBJECT
//public slots:
};

#endif // SAMPLEESCROWMANAGER_H
