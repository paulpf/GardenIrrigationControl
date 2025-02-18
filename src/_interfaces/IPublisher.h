// IPublisher.h
#ifndef I_PUBLISHER_H
#define I_PUBLISHER_H

#include "Data.h"

class IPublisher
{
public:
    virtual void publish(Data &data);
    ~IPublisher() = default;
};

#endif // I_PUBLISHER_H