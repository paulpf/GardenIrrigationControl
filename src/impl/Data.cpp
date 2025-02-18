#include <Arduino.h>
#include "_interfaces/Data.h"

Data::Data()
{
    this->currentTime = 0;
}

Data::~Data()
{
}

void Data::setCurrentTime(unsigned long currentTime)
{
    this->currentTime = currentTime;
}

unsigned long Data::getCurrentTime()
{
    return this->currentTime;
}