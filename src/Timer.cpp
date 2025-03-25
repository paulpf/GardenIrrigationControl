#include "Timer.h"

Timer::Timer() : startTime(0), duration(0), active(false), deactivationCallback(nullptr), lastTickTime(0) {}

void Timer::start(unsigned long duration) {
  Trace::log("Timer started with duration " + String(duration));
  this->duration = duration;
  startTime = millis();
  lastTickTime = startTime; // Initialize lastTickTime when the timer starts
  active = true;
  if (activationCallback) {
    activationCallback();
  }
}

void Timer::stop() {
  Trace::log("Timer stopped");
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

void Timer::handleTick() {
  if (active && tickCallback) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastTickTime >= 1000) { // Check if one second has passed
      lastTickTime = currentMillis;
      tickCallback();
    }
  }
}

void Timer::setDeactivationCallback(const std::function<void()>& callback) {
  deactivationCallback = callback;
}

void Timer::setActivationCallback(const std::function<void()>& callback) {
  activationCallback = callback;
}

void Timer::setTickCallback(const std::function<void()>& callback) {
  tickCallback = callback;
}