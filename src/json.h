#ifndef JSON_H
#define JSON_H

#include <QByteArray>
#include "IStringProcessor.h"
#include "bitcoinrpc.h"

//!
//! \brief This class will create/process json queries.
//!
class Json //: IStringProcessor
{
public:
    ///!
    //! \brief Currently just doing some testing to figure out how QJson works
    //!
    Json();
    ~Json();

    void Initialize();       // should make this part of all modules

    //void ProcessString(QSharedPointer<QByteArray>);
    void ProcessRPCString(QSharedPointer<QByteArray> jsonString);
};

#endif // JSON_H
