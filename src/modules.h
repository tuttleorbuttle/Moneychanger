#ifndef MODULES_H
#define MODULES_H

#include <QObject>
#include <QScopedPointer>
#include "btcjson.h"
#include "btcrpc.h"


//!
//! \brief The Modules class will hold pointers to various modules so they can access eachother.
//!
class Modules : public QObject
{
    Q_OBJECT
public:
    explicit Modules(QObject *parent = 0);
    ~Modules();

    static QScopedPointer<BtcJson> json;
    static QScopedPointer<BtcRpc> bitcoinRpc;

private:

    
signals:
    
public slots:
    
};

#endif // MODULES_H
