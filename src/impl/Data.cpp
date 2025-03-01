#include <Arduino.h>
#include "_interfaces/Data.h"

Data::Data()
{
    this->_currentTime = 0;
}

Data::~Data()
{
}

void Data::setCurrentTime(unsigned long currentTime)
{
    this->_currentTime = currentTime;
}

unsigned long Data::getCurrentTime()
{
    return this->_currentTime;
}