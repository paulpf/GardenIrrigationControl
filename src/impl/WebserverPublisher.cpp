#include "../_interfaces/WebserverPublisher.h"
#include "../_interfaces/Data.h"
#include <Arduino.h>
#include "config.h"
#include "ESPAsyncWebServer.h"


WebserverPublisher *WebserverPublisher::instance = nullptr;

WebserverPublisher::WebserverPublisher() : webServer(80), request(nullptr) // Initialize AsyncWebServer with port 80
{
    instance = this;
}

WebserverPublisher::~WebserverPublisher()
{
}

void WebserverPublisher::setup()
{
    // Route for root / web page
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        if (instance != nullptr) 
        {
            instance->handleRequest(request);
        }
    });

    // Route for SSE
    webServer.on("/events", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        AsyncWebServerResponse *response = request->beginChunkedResponse("text/event-stream", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t 
        {
            if (instance != nullptr && instance->sseData.length() > 0)
            {
                String response = "data: " + instance->sseData + "\n\n";
                instance->sseData = "";
                memcpy(buffer, response.c_str(), response.length());
                return response.length();
            }
            return 0;
        });
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
    });

    // Start server
    webServer.begin();
}

// Implement the interface method
void WebserverPublisher::publish(Data &data)
{
    // Store the data in sseData member variable
    sseData = String(data.getCurrentTime());
}

void WebserverPublisher::handleRequest(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", R"rawliteral(
        <html>
        <head>
            <title>Garden Irrigation Control System</title>
            <style>
                body { font-family: Arial, sans-serif; margin: 20px; }
                .timestamp { font-size: 1.2em; margin: 10px 0; }
            </style>
            <script>
                let systemTime = 0;
                
                function updateTime() {
                    if (systemTime > 0) {
                        const now = Date.now();
                        const currentTime = parseInt(systemTime) + (now - lastUpdate);
                        
                        // Convert milliseconds to readable format
                        const seconds = Math.floor(currentTime / 1000);
                        const minutes = Math.floor(seconds / 60);
                        const hours = Math.floor(minutes / 60);
                        
                        const timeString = 
                            String(hours % 24).padStart(2, '0') + ':' +
                            String(minutes % 60).padStart(2, '0') + ':' +
                            String(seconds % 60).padStart(2, '0');
                            
                        document.getElementById('systemTime').innerText = 'System Time: ' + timeString;
                    }
                }

                let lastUpdate = Date.now();
                
                if (!!window.EventSource) {
                    const source = new EventSource('/events');
                    source.onmessage = function(e) {
                        systemTime = e.data;
                        lastUpdate = Date.now();
                    };
                }

                // Update display every second
                setInterval(updateTime, 1000);
            </script>
        </head>
        <body>
            <h1>Garden Irrigation Control System</h1>
            <div id="systemTime" class="timestamp">Waiting for data...</div>
        </body>
        </html>
    )rawliteral");
}