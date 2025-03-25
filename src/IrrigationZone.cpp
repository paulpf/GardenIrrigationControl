#include "IrrigationZone.h"

IrrigationZone::IrrigationZone(int buttonPin, int relayPin, unsigned long debounceDelay, const String& clientName, MqttManager& mqttManager): 
    hwButton(buttonPin, debounceDelay), 
    relay(relayPin), 
    clientName(clientName), 
    mqttManager(mqttManager),
    synchronizedState(false), 
    hwButtonState(false), 
    swButtonState(false) {}

void IrrigationZone::setup() {
    hwButton.setup();
    hwButton.setOnPressedCallback(std::bind(&IrrigationZone::onHwButtonPressed, this));

    relay.setup();

    timer.setTickCallback(std::bind(&IrrigationZone::onTimerTick, this));
    timer.setActivationCallback(std::bind(&IrrigationZone::onTimerActivated, this));
    timer.setDeactivationCallback(std::bind(&IrrigationZone::onTimerDeactivated, this));

    mqttManager.registerTopicHandler(clientName + "/swBtn", std::bind(&IrrigationZone::handleSwBtnMessage, this, std::placeholders::_1));
    mqttManager.subscribeToTopic(clientName + "/swBtn");
}

void IrrigationZone::onHwButtonPressed() {
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
  Trace::log("Timer tick"); 
  mqttManager.publish(clientName + "/remainingTime", String(timer.getRemainingTime()));
}

void IrrigationZone::updateTimerState() {
  Trace::log("Updating timer state");  
  if (synchronizedState && !timer.isActive()) {
        timer.start(10000); // Start timer for 10 seconds
    } else if (!synchronizedState && timer.isActive()) {
        timer.stop();
    }
}

void IrrigationZone::loop() {
    timer.update();
    timer.handleTick();
    updateTimerState();    
}