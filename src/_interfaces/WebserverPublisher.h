#ifndef WEBSERVER_PUBLISHER_H
#define WEBSERVER_PUBLISHER_H

#include "IPublisher.h"
#include "Data.h"
#include "ESPAsyncWebServer.h"

class WebserverPublisher : public IPublisher
{
public:
    WebserverPublisher();
    ~WebserverPublisher();
    void setup();
    void publish(Data &data);
    void handleRequest(AsyncWebServerRequest *request);

private:
    AsyncWebServer webServer;
    AsyncWebServerRequest *request;
    String sseData; // Add this line
    static WebserverPublisher *instance;
};

#endif // WEBSERVER_PUBLISHER_H