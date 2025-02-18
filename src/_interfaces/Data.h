// Data.h
#ifndef DATA_H
#define DATA_H

#pragma once

class Data
{
private:
    unsigned long currentTime;
public:
  Data();
  ~Data();
  void setCurrentTime(unsigned long currentTime);
  unsigned long getCurrentTime();
};

#endif // DATA_H