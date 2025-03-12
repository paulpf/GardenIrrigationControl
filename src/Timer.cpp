// Timer.cpp

#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::setup(IMqttClient* mqttClient, int tmrid)
{
  _mqttClient = mqttClient;
  _mqttClient->registerTimer(this, tmrid);
  _tmrid = tmrid;
  _startTime = 0;
  _duration = 0;
  _running = false;
}

void Timer::start()
{
  _startTime = millis();
  _running = true;
}

void Timer::stop()
{
  _running = false;
}

void Timer::reset()
{
  _startTime = millis();
}

bool Timer::isRunning()
{
  return _running;
}

unsigned long Timer::getDuration()
{
  return _duration;
}

void Timer::setDuration(unsigned long duration)
{
  this->_duration = duration;
}

void Timer::loop()
{
  if (_running)
  {
    unsigned long currentTime = millis();
    if (currentTime - _startTime >= _duration)
    {
      _running = false;
    }
  }
}