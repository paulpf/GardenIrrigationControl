#include "otamanager.h"
#include "trace.h"

OtaManager::OtaManager() : _enabled(true), _isUpdating(false), _lastProgressUpdate(0)
{
}

void OtaManager::setup(const char* hostname, const char* password)
{
    if (!_enabled) 
    {
        Trace::log(TraceLevel::INFO, "OTA is disabled");
        return;
    }

    Trace::log(TraceLevel::INFO, "Setting up OTA...");

    // Set hostname for OTA
    ArduinoOTA.setHostname(hostname);
    
    // Set password if provided
    if (password != nullptr && strlen(password) > 0) 
    {
        ArduinoOTA.setPassword(password);
        Trace::log(TraceLevel::DEBUG, "OTA password protection enabled");
    }

    // Set OTA port (default is 3232)
    ArduinoOTA.setPort(3232);

    // Setup event handlers
    ArduinoOTA.onStart([this]() 
    {
        this->onStart();
    });

    ArduinoOTA.onEnd([this]() 
    {
        this->onEnd();
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) 
    {
        this->onProgress(progress, total);
    });

    ArduinoOTA.onError([this](ota_error_t error) 
    {
        this->onError(error);
    });

    // Begin OTA service
    ArduinoOTA.begin();
    
    Trace::log(TraceLevel::INFO, "OTA initialized successfully");
    Trace::log(TraceLevel::DEBUG, "OTA Hostname: " + String(hostname));
    Trace::log(TraceLevel::DEBUG, "OTA Port: 3232");
}

void OtaManager::loop()
{
    if (!_enabled) 
    {
        return;
    }
    
    ArduinoOTA.handle();
}

bool OtaManager::isUpdating() const
{
    return _isUpdating;
}

void OtaManager::setEnabled(bool enabled)
{
    _enabled = enabled;
    if (!enabled) 
    {
        Trace::log(TraceLevel::INFO, "OTA disabled");
    } 
    else 
    {
        Trace::log(TraceLevel::INFO, "OTA enabled");
    }
}

bool OtaManager::isEnabled() const
{
    return _enabled;
}

void OtaManager::onStart()
{
    _isUpdating = true;
    _lastProgressUpdate = millis();
    
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) 
    {
        type = "sketch";
    } 
    else
    { // U_SPIFFS
        type = "filesystem";
    }
    
    Trace::log(TraceLevel::INFO, "OTA Update Started - Type: " + type);
    
    // Important: Stop all other operations during OTA
    // This will be checked in the main loop
}

void OtaManager::onEnd()
{
    _isUpdating = false;
    Trace::log(TraceLevel::INFO, "OTA Update Completed Successfully");
    Trace::log(TraceLevel::INFO, "Device will restart...");
}

void OtaManager::onProgress(unsigned int progress, unsigned int total)
{
    unsigned long currentTime = millis();
    
    // Limit progress updates to prevent flooding the log
    if (currentTime - _lastProgressUpdate >= 1000) 
    { // Update every second
        _lastProgressUpdate = currentTime;
        
        unsigned int percentage = (progress / (total / 100));
        String progressMsg = "OTA Progress: " + String(percentage) + "% (" + 
                           String(progress) + "/" + String(total) + " bytes)";
        Trace::log(TraceLevel::INFO, progressMsg);
        
        #ifdef ENABLE_LOOP_TIME_PLOTTING
        Trace::plotLoopTime("OTA_Progress", 0, percentage);
        #endif
    }
}

void OtaManager::onError(ota_error_t error)
{
    _isUpdating = false;
    
    String errorMsg = "OTA Update Error: " + String(getErrorString(error));
    Trace::log(TraceLevel::ERROR, errorMsg);
}

const char* OtaManager::getErrorString(ota_error_t error)
{
    switch (error) 
    {
        case OTA_AUTH_ERROR:
            return "Authentication Failed";
        case OTA_BEGIN_ERROR:
            return "Begin Failed";
        case OTA_CONNECT_ERROR:
            return "Connect Failed";
        case OTA_RECEIVE_ERROR:
            return "Receive Failed";
        case OTA_END_ERROR:
            return "End Failed";
        default:
            return "Unknown Error";
    }
}
