#ifndef AUTOMATICIRRIGATIONCONTROLLER_H
#define AUTOMATICIRRIGATIONCONTROLLER_H

class AutomaticIrrigationController
{
public:
  void setup(bool enabledByDefault = false);
  void setEnabled(bool enabled);
  bool isEnabled() const;
  bool canRunSchedules() const;

private:
  bool _enabled = false;
};

#endif // AUTOMATICIRRIGATIONCONTROLLER_H
