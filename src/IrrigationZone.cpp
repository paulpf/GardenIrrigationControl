#include "IrrigationZone.h"

IrrigationZone::IrrigationZone(int buttonPin, int relayPin, unsigned long debounceDelay, const String& clientName, MqttManager& mqttManager)
    : button(buttonPin, debounceDelay), relay(relayPin), clientName(clientName), mqttManager(mqttManager),
      synchronizedState(false), hwButtonState(false), swButtonState(false) {}

void IrrigationZone::setup() {
    button.setup();
    button.setOnPressedCallback(std::bind(&IrrigationZone::handleButtonPress, this));

    relay.setup();

    timer.setTickCallback([this]() { onTimerTick(); });
    timer.setActivationCallback([this]() { onTimerActivated(); });
    timer.setDeactivationCallback([this]() { onTimerDeactivated(); });
}

void IrrigationZone::handleButtonPress() {
    Trace::log("Hardware button pressed");
    hwButtonState = !hwButtonState;
    synchronizedState = swButtonState = hwButtonState;
}

void IrrigationZone::handleSwBtnMessage(const String& message) {
    Trace::log("Received message on topic " + clientName + "/swBtn: " + message);
    swButtonState = (message == "true");
    synchronizedState = hwButtonState = swButtonState;
}

void IrrigationZone::onTimerActivated() {
    Trace::log("Timer activated");
    relay.activate(true);
    synchronizedState = true;
    mqttManager.publish(clientName + "/relay", String(relay.isActive()));
    mqttManager.publish(clientName + "/swBtn", synchronizedState ? "true" : "false");
}

void IrrigationZone::onTimerDeactivated() {
    Trace::log("Timer deactivated");
    relay.activate(false);
    synchronizedState = false;
    mqttManager.publish(clientName + "/relay", String(relay.isActive()));
    mqttManager.publish(clientName + "/swBtn", synchronizedState ? "true" : "false");
    mqttManager.publish(clientName + "/remainingTime", String(0));
}

void IrrigationZone::onTimerTick() {
    mqttManager.publish(clientName + "/remainingTime", String(timer.getRemainingTime()));
}

void IrrigationZone::updateTimerState() {
    if (synchronizedState && !timer.isActive()) {
        timer.start(10000); // Start timer for 10 seconds
    } else if (!synchronizedState && timer.isActive()) {
        timer.stop();
    }
}