#include "mqttmanager.h"

// Initialize the static instance pointer
MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager()
{
    // Store the instance pointer
    _instance = this;
    _mqttState = MQTT_DISCONNECTED_STATE;
    _mqttReconnectAttempts = 0;
    _pubSubClient.setClient(_wifiClient);
}

void MqttManager::setup(const char* mqttServer, int mqttPort, const char* mqttUser, const char* mqttPassword, const char* clientName) 
{
    _mqttServer = mqttServer;
    _mqttPort = mqttPort;
    _mqttUser = mqttUser;
    _mqttPassword = mqttPassword;
    _clientName = clientName;
    _pubSubClient.setServer(_mqttServer, _mqttPort);
    _pubSubClient.setCallback(staticMqttCallback);
    Trace::log("MqttManager setup complete.");
}

// Static callback that will be called by PubSubClient
void MqttManager::staticMqttCallback(char* topic, byte* payload, unsigned int length) 
{
    // Forward to the instance method if instance exists
    if (_instance) 
    {
        _instance->instanceMqttCallback(topic, payload, length);
    }
}

// Instance callback that processes the actual MQTT message
void MqttManager::instanceMqttCallback(char* topic, byte* payload, unsigned int length)
{
    Trace::log("Message arrived [" + String(topic) + "]");
    String message = "";
    for (int i = 0; i < length; i++) 
    {
        message += (char)payload[i];
    }
    Trace::log("Message: " + message);

    // Check if the message is for the software button of any irrigation zone
    for (int i = 0; i < _numIrrigationZones; i++) 
    {
        if (String(topic).startsWith(_irrigationZones[i]->getMqttTopicForSwButton())) 
        {
            _irrigationZones[i]->synchronizeButtonStates(message == "true");
            break; // Exit the loop after processing the message
        }
    }
}

void MqttManager::loop() 
{
    if (WiFi.status() != WL_CONNECTED) 
    {
        // Can't connect to MQTT without WiFi
        _mqttState = MQTT_DISCONNECTED_STATE;
        return;
    }

    switch (_mqttState) 
    {
        case MQTT_DISCONNECTED_STATE:
            if (millis() - _lastMqttAttemptMillis >= _mqttRetryInterval) 
            {
                reconnect();
                _lastMqttAttemptMillis = millis();
            }
            break;
            
        case MQTT_CONNECTED_STATE:
            if (!_pubSubClient.connected()) 
            {
                Trace::log("MQTT connection lost");
                _mqttState = MQTT_DISCONNECTED_STATE;
            } 
            else 
            {
                _pubSubClient.loop(); // Process incoming messages and maintain connection
            }
            break;
    }
}

void MqttManager::reconnect() 
{
    // Single connection attempt
    Trace::log("Attempting MQTT connection...");
    if (_pubSubClient.connect(_clientName, _mqttUser, _mqttPassword)) 
    {
        Trace::log("MQTT connected");
        _mqttState = MQTT_CONNECTED_STATE;
        _mqttReconnectAttempts = 0;
        
        // Resubscribe to all topics for irrigation zones
        for (int i = 0; i < _numIrrigationZones; i++) 
        {
            _pubSubClient.subscribe(_irrigationZones[i]->getMqttTopicForSwButton().c_str());
        }
    } 
    else 
    {
        _mqttReconnectAttempts++;
        Trace::log("MQTT connection failed, rc=" + String(_pubSubClient.state()) + 
                ", attempts: " + String(_mqttReconnectAttempts));
        _mqttState = MQTT_DISCONNECTED_STATE;
        
        if (_mqttReconnectAttempts >= _maxMqttReconnectAttempts) 
        {
            Trace::log("Maximum MQTT reconnection attempts reached, will try again later");
            _mqttReconnectAttempts = 0;
        }
    }
}

void MqttManager::publish(const char* topic, const char* payload) 
{
    if (_mqttState == MQTT_CONNECTED_STATE) 
    {
        _pubSubClient.publish(topic, payload);
    }
    else 
    {
        Trace::log("Cannot publish to MQTT - not connected");
    }
}

void MqttManager::subscribe(const char* topic) 
{
    if (_mqttState == MQTT_CONNECTED_STATE) 
    {
        _pubSubClient.subscribe(topic);
        Trace::log("Subscribed to: " + String(topic));
    }
    else 
    {
        Trace::log("Cannot subscribe to MQTT - not connected");
    }
}

void MqttManager::addIrrigationZone(IrrigationZone* zone) 
{
    if (_numIrrigationZones < MAX_IRRIGATION_ZONES) 
    {
        _irrigationZones[_numIrrigationZones] = zone;
        _numIrrigationZones++;
        Trace::log("Added irrigation zone: " + zone->getMqttTopicForZone());
        
        // Subscribe to the zone's button topic if we're connected
        if (_mqttState == MQTT_CONNECTED_STATE) 
        {
            subscribe(zone->getMqttTopicForSwButton().c_str());
        }
    } 
    else 
    {
        Trace::log("Maximum number of irrigation zones reached.");
    }
}

bool MqttManager::isConnected() 
{
    return _mqttState == MQTT_CONNECTED_STATE;
}