#include "mqttmanager.h"
#include "dht11manager.h"

// Initialize the static instance pointer
MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager()
{
    // Store the instance pointer
    _instance = this;
    _mqttState = MQTT_DISCONNECTED_STATE;
    _mqttReconnectAttempts = 0;
    _pubSubClient.setClient(_wifiClient);
    _dht11Manager = nullptr; // Initialize DHT11 manager pointer
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
    reconnect(); // Attempt to connect to MQTT broker
    Trace::log(TraceLevel::DEBUG, "MqttManager setup complete.");
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
    Trace::log(TraceLevel::DEBUG, "Message arrived [" + String(topic) + "]");
    String message = "";
    for (int i = 0; i < length; i++) 
    {
        message += (char)payload[i];
    }
    Trace::log(TraceLevel::DEBUG, "Message: " + message);

    // Check if the message is for the software button of any irrigation zone
    for (int i = 0; i < _numIrrigationZones; i++) 
    {
        if (String(topic).startsWith(_irrigationZones[i]->getMqttTopicForSwButton())) 
        {
            _irrigationZones[i]->synchronizeButtonStates(message == "true");
            break; // Exit the loop after processing the message
        }

        if(String(topic).startsWith(_irrigationZones[i]->getMqttTopicForDurationTime())) 
        {
            _blockPublish = true; // Set block to true to prevent further processing
            int durationTime = message.toInt();
            if (durationTime > 0 && durationTime <= MAX_DURATION_TIME) 
            {
                // Update to use new method with zone index for storage
                _irrigationZones[i]->setDurationTime(durationTime, i);
                Trace::log(TraceLevel::DEBUG, "Updated duration time for zone " + String(i) + ": " + String(durationTime));
            } 
            else 
            {
                // Invalid duration time, reset to default
                _irrigationZones[i]->setDurationTime(DEFAULT_DURATION_TIME, i);
                Trace::log(TraceLevel::DEBUG, "Invalid duration time received for zone " + String(i) + ": " + String(durationTime));
            }
            _blockPublish = false; // Reset block to false after processing
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
                Trace::log(TraceLevel::DEBUG, "MQTT connection lost");
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
    Trace::log(TraceLevel::DEBUG, "Attempting MQTT connection...");
    if (_pubSubClient.connect(_clientName, _mqttUser, _mqttPassword)) 
    {
        Trace::log(TraceLevel::DEBUG, "MQTT connected");
        _mqttState = MQTT_CONNECTED_STATE;
        _mqttReconnectAttempts = 0;
        
        // Resubscribe to all topics for irrigation zones
        subscribeIrrigationZones();
        // Publish initial state of all irrigation zones
        initPublish();
    } 
    else 
    {
        _mqttReconnectAttempts++;
        Trace::log(TraceLevel::DEBUG, "MQTT connection failed, rc=" + String(_pubSubClient.state()) + 
                ", attempts: " + String(_mqttReconnectAttempts));
        _mqttState = MQTT_DISCONNECTED_STATE;
        
        if (_mqttReconnectAttempts >= _maxMqttReconnectAttempts) 
        {
            Trace::log(TraceLevel::DEBUG, "Maximum MQTT reconnection attempts reached, will try again later");
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
        Trace::log(TraceLevel::DEBUG, "Cannot publish to MQTT - not connected");
    }
}

void MqttManager::subscribeIrrigationZones() 
{
    // Subscribe to all irrigation zones
    for (int i = 0; i < _numIrrigationZones; i++) 
    {
        subscribe(_irrigationZones[i]->getMqttTopicForRelay().c_str());
        subscribe(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str());
        subscribe(_irrigationZones[i]->getMqttTopicForSwButton().c_str());
        subscribe(_irrigationZones[i]->getMqttTopicForDurationTime().c_str());
    }
}

void MqttManager::subscribe(const char* topic) 
{
    if (_mqttState == MQTT_CONNECTED_STATE) 
    {
        _pubSubClient.subscribe(topic);
        Trace::log(TraceLevel::DEBUG, "Subscribed to: " + String(topic));
    }
    else 
    {
        Trace::log(TraceLevel::DEBUG, "Cannot subscribe to MQTT - not connected");
    }
}

void MqttManager::addIrrigationZone(IrrigationZone* zone) 
{
    if (_numIrrigationZones < MAX_IRRIGATION_ZONES) 
    {
        _irrigationZones[_numIrrigationZones] = zone;
        _numIrrigationZones++;
        Trace::log(TraceLevel::DEBUG, "Added irrigation zone: " + zone->getMqttTopicForZone());
    } 
    else 
    {
        Trace::log(TraceLevel::DEBUG, "Maximum number of irrigation zones reached.");
    }
}

bool MqttManager::isConnected() 
{
    return _mqttState == MQTT_CONNECTED_STATE;
}

bool MqttManager::publishAllIrrigationZones() 
{
    if(isConnected()) 
    {
        for (int i = 0; i < _numIrrigationZones; i++) 
        {
            publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(), 
                _irrigationZones[i]->getRelayState() ? "true" : "false");
            publish(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str(), 
                String(_irrigationZones[i]->getRemainingTimeAsString()).c_str());
            publish(_irrigationZones[i]->getMqttTopicForSwButton().c_str(),
                _irrigationZones[i]->getBtnState() ? "true" : "false");
        }
        return true; // Successfully published all zones
    } 
    else 
    {
        Trace::log(TraceLevel::DEBUG, "Cannot publish to MQTT - not connected");
        return false; // Failed to publish because not connected
    }
}

void MqttManager::initPublish()
{
    for (int i = 0; i < _numIrrigationZones; i++) 
    {
        publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
            String(_irrigationZones[i]->getDurationTime()).c_str());
    }
}

void MqttManager::setDht11Manager(Dht11Manager* dht11Manager) 
{
    _dht11Manager = dht11Manager;
    Trace::log(TraceLevel::DEBUG, "DHT11 manager assigned to MQTT manager");
}

bool MqttManager::publishDht11Data() 
{
    if (!isConnected() || _dht11Manager == nullptr) {
        return false;
    }
    
    if (_dht11Manager->isDataValid()) {
        // Publish temperature
        String tempStr = String(_dht11Manager->getTemperature(), 1);
        publish(_dht11Manager->getMqttTopicForTemperature().c_str(), tempStr.c_str());
        
        // Publish humidity
        String humStr = String(_dht11Manager->getHumidity(), 1);
        publish(_dht11Manager->getMqttTopicForHumidity().c_str(), humStr.c_str());
        
        // Publish heat index
        String heatIndexStr = String(_dht11Manager->getHeatIndex(), 1);
        publish(_dht11Manager->getMqttTopicForHeatIndex().c_str(), heatIndexStr.c_str());
          // Publish sensor status
        publish(_dht11Manager->getMqttTopicForStatus().c_str(), "online");
        
        return true;
    } else {
        // Publish offline status if sensor data is invalid
        publish(_dht11Manager->getMqttTopicForStatus().c_str(), "offline");
        return false;
    }
}