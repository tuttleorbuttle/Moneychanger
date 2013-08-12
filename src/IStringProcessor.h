#ifndef ISTRINGPROCESSOR_H
#define ISTRINGPROCESSOR_H

#include <QByteArray>
#include <QSharedPointer>

class IStringProcessor
{
public:
    virtual ~IStringProcessor() {}
    virtual void ProcessString(QSharedDataPointer<QByteArray>) = 0;
};


#endif // ISTRINGPROCESSOR_H
