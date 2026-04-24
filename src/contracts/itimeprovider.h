#ifndef ITIMEPROVIDER_H
#define ITIMEPROVIDER_H

class ITimeProvider
{
public:
  virtual ~ITimeProvider() = default;
  virtual unsigned long millis() const = 0;
  virtual void delayMs(unsigned long ms) = 0;
};

#endif // ITIMEPROVIDER_H
