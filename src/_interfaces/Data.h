// Data.h
#ifndef DATA_H
#define DATA_H

#pragma once

class Data
{
public:
  Data();
  ~Data();
  void setCurrentTime(unsigned long currentTime);
  unsigned long getCurrentTime();

private:
  unsigned long currentTime;
};

#endif // DATA_H