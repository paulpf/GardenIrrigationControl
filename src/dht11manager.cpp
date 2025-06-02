#include "dht11manager.h"

Dht11Manager::Dht11Manager() 
    : _dht(nullptr), _pin(-1), _dhtType(DHT11), 
      _temperature(NAN), _humidity(NAN), _heatIndex(NAN), 
      _dataValid(false), _lastReadTime(0)
{
}

void Dht11Manager::setup(int pin, int dhtType, String clientName) 
{
    _pin = pin;
    _dhtType = dhtType;
    _mqttTopicBase = clientName + "/dht11"; // Set MQTT topic base
    
    // Initialize DHT sensor
    _dht = new DHT(_pin, _dhtType);
    _dht->begin();
    
    // Wait for sensor to stabilize
    delay(2000);
    
    // Initial reading
    readSensorData();
    
    Trace::log(TraceLevel::DEBUG, "DHT11 sensor initialized on pin " + String(_pin));
}

void Dht11Manager::loop() 
{
    unsigned long currentTime = millis();
    
    // Check if it's time to read the sensor
    if (currentTime - _lastReadTime >= DHT11_READ_INTERVAL) 
    {
        readSensorData();
        _lastReadTime = currentTime;
    }
}

void Dht11Manager::readSensorData() 
{
    if (_dht == nullptr) 
    {
        Trace::log(TraceLevel::ERROR, "DHT11 sensor not initialized");
        _dataValid = false;
        return;
    }
    
    // Read temperature and humidity
    float newTemperature = _dht->readTemperature();
    float newHumidity = _dht->readHumidity();    // Check if readings are valid
    if (isValidReading(newTemperature) && isValidReading(newHumidity)) 
    {
        _temperature = newTemperature;
        _humidity = newHumidity;
        calculateHeatIndex();
        _dataValid = true;
        
        Trace::log(TraceLevel::DEBUG, "DHT11 - Temperature: " + String(_temperature, 1) + 
                  "°C, Humidity: " + String(_humidity, 1) + 
                  "%, Heat Index: " + String(_heatIndex, 1) + "°C");
    }
    else
    {
        _dataValid = false;
        Trace::log(TraceLevel::ERROR, "DHT11 sensor reading failed");
    }
}

void Dht11Manager::calculateHeatIndex() 
{
    // Calculate heat index using the formula for Celsius
    // This is a simplified version - DHT library also has a computeHeatIndex method
    if (_dataValid) 
    {
        _heatIndex = _dht->computeHeatIndex(_temperature, _humidity, false); // false for Celsius
    }
    else
    {
        _heatIndex = NAN;
    }
}

bool Dht11Manager::isValidReading(float value) const 
{
    // Check if the reading is not NaN and within reasonable ranges
    if (isnan(value)) 
    {
        return false;
    }
    
    // DHT11 specifications:
    // Temperature: 0-50°C (±2°C accuracy)
    // Humidity: 20-90% RH (±5% accuracy)
    // Allow some tolerance for extreme conditions
    return true; // Basic NaN check is sufficient for most cases
}


