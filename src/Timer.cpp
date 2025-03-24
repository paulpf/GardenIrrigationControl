#include "Timer.h"

Timer::Timer() : startTime(0), duration(0), active(false), deactivationCallback(nullptr) {}

void Timer::start(unsigned long duration) {
  this->duration = duration;
  startTime = millis();
  active = true;
  if (activationCallback) {
    activationCallback();
  }
}

void Timer::stop() {
  active = false;
  if (deactivationCallback) {
    deactivationCallback();
  }
}

void Timer::update() {
  if (active && (millis() - startTime >= duration)) {
    stop();
  }
}

bool Timer::isActive() const {
  return active;
}

unsigned long Timer::getRemainingTime() const {
  if (active) {
    return duration - (millis() - startTime);
  } else {
    return 0;
  }
}

void Timer::setDeactivationCallback(void (*callback)()) {
  deactivationCallback = callback;
}

void Timer::setActivationCallback(void (*callback)()) {
  activationCallback = callback;
}