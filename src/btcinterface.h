#ifndef BTCINTEGRATION_H
#define BTCINTEGRATION_H

#include <QObject>

class BtcInterface : public QObject
{
    Q_OBJECT
public:
    explicit BtcInterface(QObject *parent = 0);

    QString CreateTwoOfTwoEscrowAddress(QString myKey, QString hisKey);

private:
    
signals:
    
public slots:
    
};

#endif // BTCINTEGRATION_H
