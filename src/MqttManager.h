#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "Trace.h"

class MqttManager {
public:
    // Enum for MQTT connection state
    enum MqttState {
        _MQTT_DISCONNECTED,
        _MQTT_CONNECTING,
        _MQTT_CONNECTED
    };
    
    // Constructor
    MqttManager(const char* serverIp, int serverPort, const char* username, 
                const char* password, const String& clientName);
    
    // Setup method to initialize MQTT
    void setup();
    
    // Main connection management methods
    void manageMqttConnection(bool wifiIsConnected, bool dnsResolutionOk);
    void nonBlockingMqttManagement(bool wifiIsConnected, bool dnsResolutionOk);
    
    // Publish and subscribe methods
    bool publish(const String& topic, const String& payload);
    bool subscribe(const String& topic);
    
    // Connection status
    bool isConnected() const;
    MqttState getState() const { return mqttState; }
    
    // Set callback function for incoming messages
    void setCallback(MQTT_CALLBACK_SIGNATURE);

private:
    // MQTT client
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    // Connection parameters
    const char* serverIp;
    int serverPort;
    const char* username;
    const char* password;
    String clientName;
    
    // State management
    MqttState mqttState;
    unsigned long lastAttemptMillis;
    int reconnectAttempts;
    
    // Constants
    const unsigned long mqttRetryInterval = 5000; // Wait 5 seconds between connection attempts
    const int maxMqttReconnectAttempts = 5;
    
    // Private methods
    void reconnectMqtt();
};

#endif // MQTT_MANAGER_H