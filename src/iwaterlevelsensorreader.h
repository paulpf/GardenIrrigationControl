#ifndef IWATERLEVELSENSORREADER_H
#define IWATERLEVELSENSORREADER_H

class IWaterLevelSensorReader
{
public:
  virtual ~IWaterLevelSensorReader() = default;
  virtual void setup() = 0;
  virtual int readRaw() = 0;
};

#endif // IWATERLEVELSENSORREADER_H
