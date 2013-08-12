#ifndef MODULES_H
#define MODULES_H

#include <QObject>
#include <QScopedPointer>
#include "json.h"
#include "bitcoinrpc.h"


//!
//! \brief The Modules class will hold pointers to various modules so they can access eachother.
//!
class Modules : public QObject
{
    Q_OBJECT
public:
    explicit Modules(QObject *parent = 0);
    ~Modules();

    static QScopedPointer<Json> json;
    static QScopedPointer<BitcoinRpc> bitcoinRpc;

private:

    
signals:
    
public slots:
    
};

#endif // MODULES_H
