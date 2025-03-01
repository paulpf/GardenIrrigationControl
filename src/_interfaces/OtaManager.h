// OtaManager.h

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>

class OtaManager
{
public:
    OtaManager();
    void setup();
    void handle();
private:
    void setupOta();
};

#endif // OTA_MANAGER_H