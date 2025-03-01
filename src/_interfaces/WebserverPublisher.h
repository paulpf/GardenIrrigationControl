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
    AsyncWebServer _webServer;
    AsyncWebServerRequest *_request;
    String _sseData; // Add this line
    static WebserverPublisher *_instance;
};

#endif // WEBSERVER_PUBLISHER_H