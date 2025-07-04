#ifndef DHT11MANAGER_H
#define DHT11MANAGER_H

#include "globaldefines.h"
#include <DHT.h>

class Dht11Manager {
public:
    Dht11Manager();
    void setup(int pin, int dhtType, String clientName);
    void loop();
    
    // Getters for sensor data
    float getTemperature() const { return _temperature; }
    float getHumidity() const { return _humidity; }    float getHeatIndex() const { return _heatIndex; }
    
    // Status methods
    bool isDataValid() const { return _dataValid; }
    unsigned long getLastReadTime() const { return _lastReadTime; }
    
    // MQTT topic getters
    String getMqttTopicForTemperature() const { return _mqttTopicBase + "/temperature"; }
    String getMqttTopicForHumidity() const { return _mqttTopicBase + "/humidity"; }
    String getMqttTopicForHeatIndex() const { return _mqttTopicBase + "/heatIndex"; }
    String getMqttTopicForStatus() const { return _mqttTopicBase + "/status"; }
    String getTimeStamp() const { return String(_lastReadTime); }

private:
    DHT* _dht;
    int _pin;
    int _dhtType;
      // Sensor readings
    float _temperature;
    float _humidity;
    float _heatIndex;
    bool _dataValid;
    
    // Timing
    unsigned long _lastReadTime;
    
    // MQTT
    String _mqttTopicBase;
    
    // Helper methods
    void readSensorData();
    void calculateHeatIndex();
    bool isValidReading(float value) const;
};

#endif // DHT11MANAGER_H
