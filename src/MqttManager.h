#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>
#include <map>
#include "Trace.h"

class MqttManager {

private:
    // MQTT client
    WiFiClient wifiClient;
    PubSubClient pubSubClient;

    // Add a handler type for topic-specific callbacks
    typedef std::function<void(const String&)> TopicHandler;
    std::map<String, TopicHandler> topicHandlers;
    
    // Internal callback that will dispatch to the right handler
    static void globalCallback(char* topic, byte* payload, unsigned int length);
    static MqttManager* instance; // Singleton instance for the static callback
    
    // Connection parameters
    const char* serverIp;
    int serverPort;
    const char* username;
    const char* password;
    String clientName;
    
    unsigned long lastAttemptMillis;
    int reconnectAttempts;
    
    // Constants
    const unsigned long mqttRetryInterval = 5000; // Wait 5 seconds between connection attempts
    const int maxMqttReconnectAttempts = 5;
    
    // Private methods
    void reconnectMqtt();

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

    MqttState mqttState;

    // New methods for handling specific topics
    void subscribeToTopic(const String& topic);
    void registerTopicHandler(const String& topicPrefix, TopicHandler handler);
    
    // Setup method to initialize MQTT
    void setup();
    
    // Main connection management methods
    void manageMqttConnection(bool wifiIsConnected, bool dnsResolutionOk);
    void loop(bool wifiIsConnected, bool dnsResolutionOk);
    
    // Publish and subscribe methods
    bool publish(const String& topic, const String& payload);
    bool subscribe(const String& topic);
    
    // Connection status
    bool isConnected() const;
    MqttState getState() const { return mqttState; }
    
    // Set callback function for incoming messages
    void setCallback(MQTT_CALLBACK_SIGNATURE);


};

#endif // MQTT_MANAGER_H