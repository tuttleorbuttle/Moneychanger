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

    // TODO: _maybe_ overload the :: operator to check
    // if the pointer isn't NULL
    static QScopedPointer<BtcJson> json;
    static QScopedPointer<BtcRpc> bitcoinRpc;

private:

    
signals:
    
public slots:
    
};

#endif // MODULES_H
