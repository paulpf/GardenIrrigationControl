#include "MqttManager.h"

MqttManager::MqttManager(const char* serverIp, int serverPort, const char* username, 
                        const char* password, const String& clientName) :
    mqttClient(wifiClient),
    serverIp(serverIp),
    serverPort(serverPort),
    username(username),
    password(password),
    clientName(clientName),
    mqttState(_MQTT_DISCONNECTED),
    lastAttemptMillis(0),
    reconnectAttempts(0)
{
}

void MqttManager::setup() {
    mqttClient.setServer(serverIp, serverPort);
}

void MqttManager::reconnectMqtt() {
    // Single connection attempt instead of blocking loop
    Trace::log("Attempting MQTT connection...");
    if (mqttClient.connect(clientName.c_str(), username, password)) {
        Trace::log("MQTT connected");
        mqttState = _MQTT_CONNECTED;
        reconnectAttempts = 0;
    } 
    else {
        reconnectAttempts++;
        Trace::log("MQTT connection failed, rc=" + String(mqttClient.state()) + 
                ", attempts: " + String(reconnectAttempts));
        mqttState = _MQTT_DISCONNECTED;
        
        if (reconnectAttempts >= maxMqttReconnectAttempts) {
            Trace::log("Maximum MQTT reconnection attempts reached, will try again later");
            reconnectAttempts = 0;
        }
    }
}

void MqttManager::manageMqttConnection(bool wifiIsConnected, bool dnsResolutionOk) {
    Trace::log("Managing MQTT connection");
    
    if (!wifiIsConnected) {
        Trace::log("Cannot connect to MQTT - WiFi is disconnected");
        mqttState = _MQTT_DISCONNECTED;
        return; // Can't connect to MQTT without WiFi
    }

    // DNS check before attempting connection
    if (mqttState == _MQTT_DISCONNECTED && !dnsResolutionOk) {
        return;  // Skip connection attempt if DNS resolution fails
    }
    
    switch (mqttState) {
        case _MQTT_DISCONNECTED:
            Trace::log("Mqtt is disconnected");
            reconnectMqtt();
            lastAttemptMillis = millis();
            break;
            
        case _MQTT_CONNECTED:
            if (!mqttClient.connected()) {
                Trace::log("MQTT connection lost");
                mqttState = _MQTT_DISCONNECTED;
            } 
            else {
                Trace::log("MQTT loop");
                mqttClient.loop(); // Process incoming messages and maintain connection
            }
            break;
    }
}

void MqttManager::nonBlockingMqttManagement(bool wifiIsConnected, bool dnsResolutionOk) {
    unsigned long currentMillis = millis();
    if (mqttState == _MQTT_DISCONNECTED && 
        currentMillis - lastAttemptMillis >= mqttRetryInterval) {
        manageMqttConnection(wifiIsConnected, dnsResolutionOk);
    }
    else if (mqttState == _MQTT_CONNECTED) {
        mqttClient.loop(); // Process incoming messages
    }
}

bool MqttManager::publish(const String& topic, const String& payload) {
    if (mqttState == _MQTT_CONNECTED) {
        return mqttClient.publish(topic.c_str(), payload.c_str());
    }
    else {
        Trace::log("Cannot publish to MQTT - not connected");
        return false;
    }
}

bool MqttManager::subscribe(const String& topic) {
    if (mqttState == _MQTT_CONNECTED) {
        return mqttClient.subscribe(topic.c_str());
    }
    else {
        Trace::log("Cannot subscribe to MQTT - not connected");
        return false;
    }
}

bool MqttManager::isConnected() const {
    return mqttState == _MQTT_CONNECTED;
}

void MqttManager::setCallback(MQTT_CALLBACK_SIGNATURE) {
    mqttClient.setCallback(callback);
}