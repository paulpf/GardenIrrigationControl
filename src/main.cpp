#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#include "./_config/MqttConfig.h"
#include "./_secrets/MqttSecret.h"
#endif

#include "globaldefines.h"
#include "config.h"

#include "wifimanager.h"
#include "irrigationZone.h"
#include "mqttmanager.h"

#include "esp_task_wdt.h"
#include "esp_system.h"

// ================ Variables ================
// Name is used for the hostname. This will be updated after WiFi init with actual MAC
const int CLIENT_NAME_MAX_SIZE = 50;  // Max Größe für den Client-Namen
char clientName[CLIENT_NAME_MAX_SIZE]; // Buffer für clientName

// ================ WiFi ================
WifiManager wifiManager;

// ================ MQTT ================
MqttManager mqttManager;

// ================ Irrigation zones ================
// Verwendung eines Arrays für bessere Skalierbarkeit mit 8 Zonen
IrrigationZone irrigationZones[MAX_IRRIGATION_ZONES];
int activeZones = 0; // Wird im Setup erhöht

// Extra-Hilfsfunktion zur Konfiguration zusätzlicher Zonen
// bool addIrrigationZone(int buttonPin, int relayPin, const char* zoneName) 
// {
//   if (activeZones < MAX_IRRIGATION_ZONES) 
//   {
//     char topicBuffer[100];
//     snprintf(topicBuffer, sizeof(topicBuffer), "%s/irrigationZone%d", clientNameBuffer, activeZones + 1);
    
//     irrigationZones[activeZones].setup(buttonPin, relayPin, topicBuffer);
//     mqttManager.addIrrigationZone(&irrigationZones[activeZones]);
//     activeZones++;
//     Trace::log("Neue Bewässerungszone hinzugefügt: " + String(zoneName) + " (Zone " + String(activeZones) + ")");
//     return true;
//   } 
//   else 
//   {
//     Trace::log("Maximale Anzahl an Bewässerungszonen erreicht!");
//     return false;
//   }
// }

// ================ Timing ================
unsigned long previousMillis = 0;
unsigned long previousReconnectMillis = 0;
const unsigned long reconnectInterval = 30000; // Reconnect every 30 seconds

// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Initialen Client-Namen setzen (wird später aktualisiert)
  strncpy(clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
  clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';

  // Setup WiFi
  wifiManager.setup(WIFI_SSID, WIFI_PWD, clientName);

  // Aktualisiere den Client-Namen mit MAC-Adresse für eindeutige Identifizierung
  String macFormatted = Tools::replaceChars(WiFi.macAddress(), ':', '-');
  Tools::formatToBuffer(clientName, CLIENT_NAME_MAX_SIZE, "GardenController-%s", macFormatted.c_str());
  Trace::log("Client-Name gesetzt: " + String(clientName));

  // Setup MQTT
  mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName);

  // Setup aller 8 Bewässerungszonen
  Trace::log("Initialisiere 8 Bewässerungszonen...");
  
  // Zone 1
  Tools::addIrrigationZone(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, "Rasen vorne", irrigationZones, &mqttManager, activeZones, clientName);
  
  Trace::log("Bewässerungszonen initialisiert: " + String(activeZones) + " Zonen");

  // Initialize the watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL); // Add current thread to WDT watch

  Trace::log("Setup end");
}

void loop() 
{
  unsigned long currentMillis = millis();
  
  // Reset the watchdog timer in each loop iteration
  esp_task_wdt_reset();
  
  // Haupt-Timer-basierte Ereignisse
  if (currentMillis - previousMillis >= LOOP_INTERVAL) 
  {
    previousMillis = currentMillis;

    // Periodische Aufgaben ausführen (mit reduziertem Logging)
    #if DEBUG_MODE
    Trace::log("Loop: " + String(millis()));
    #endif

    // MQTT-Daten veröffentlichen (nur periodisch notwendig)
    mqttManager.publishAllIrrigationZones();
  }

  // Diese Funktionen sollten in jedem Durchlauf aufgerufen werden (ohne Verzögerung)
  
  // WiFi-Status überprüfen und verwalten
  wifiManager.loop();
  
  // MQTT-Nachrichten verarbeiten
  mqttManager.loop();
  
  // Bewässerungszonen aktualisieren
  for (int i = 0; i < activeZones; i++) 
  {
    irrigationZones[i].loop();
  }
}