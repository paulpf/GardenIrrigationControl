#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include "globaldefines.h"
#include <ArduinoOTA.h>

class OtaManager 
{
public:
    OtaManager();
    
    void setup(const char* hostname, const char* password = nullptr);
    void loop();
    bool isUpdating() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
private:
    bool _enabled;
    bool _isUpdating;
    unsigned long _lastProgressUpdate;
    
    void onStart();
    void onEnd();
    void onProgress(unsigned int progress, unsigned int total);
    void onError(ota_error_t error);
    
    const char* getErrorString(ota_error_t error);
};

#endif // OTAMANAGER_H
