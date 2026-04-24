#include "gardencontrollerapp.h"

#include "_config/MqttConfig.h"
#include "_secrets/MqttSecret.h"
#include "_secrets/OtaSecret.h"
#include "_secrets/WifiSecret.h"

#include "config.h"
#include "global_defines.h"

#include "helper.h"
#include "storage_manager.h"
#include "trace.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

GardenControllerApp::GardenControllerApp(
    IrrigationConfig &irrigationConfig, SystemConfig &systemConfig,
    HardwareConfig &hardwareConfig, WifiManager &wifiManager,
    MqttManager &mqttManager, OtaManager &otaManager,
    IrrigationZone (&irrigationZones)[MAX_IRRIGATION_ZONES],
    WaterLevelManager &waterLevelManager,
    ConnectivityCoordinator &connectivityCoordinator,
    WifiConnectionAwaiter &wifiConnectionAwaiter, OtaLoopGuard &otaLoopGuard,
    LoopScheduler &loopScheduler)
    : _irrigationConfig(irrigationConfig),
      _systemConfig(systemConfig),
      _hardwareConfig(hardwareConfig),
      _wifiManager(wifiManager),
      _mqttManager(mqttManager),
      _otaManager(otaManager),
      _irrigationZones(irrigationZones),
      _waterLevelManager(waterLevelManager),
      _connectivityCoordinator(connectivityCoordinator),
      _wifiConnectionAwaiter(wifiConnectionAwaiter),
      _otaLoopGuard(otaLoopGuard),
      _loopScheduler(loopScheduler)
{
  _clientName[0] = '\0';
}

void GardenControllerApp::initIrrigationZones()
{
  Trace::log(TraceLevel::INFO, "Initializing irrigation zones...");

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    IrrigationZoneFactory::initializeZone(
        _irrigationZones[i], _irrigationConfig, _hardwareConfig.zones[i],
        _mqttManager, i, _clientName);
    _irrigationZones[i].loadSettingsFromStorage(i);
  }
}

void GardenControllerApp::updateClientNameFromMac()
{
  String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
  Helper::formatToBuffer(_clientName, CLIENT_NAME_MAX_SIZE,
                         "GardenController-%s", macFormatted.c_str());
  Trace::log(TraceLevel::INFO, "Client name set: " + String(_clientName));
}

void GardenControllerApp::setupOta()
{
#if ENABLE_OTA
  Trace::log(TraceLevel::INFO, "Setting up OTA...");
  _otaManager.setup(_clientName, OTA_PASSWORD);
#else
  Trace::log(TraceLevel::INFO, "OTA disabled in configuration");
  _otaManager.setEnabled(false);
#endif
}

void GardenControllerApp::initWatchdog()
{
  esp_task_wdt_init(_systemConfig.watchdogTimeoutMs / 1000, true);
  esp_task_wdt_add(NULL);
}

void GardenControllerApp::waitForWifiConnection()
{
  Trace::log(TraceLevel::INFO, "Waiting for WiFi connection...");
  if (!_wifiConnectionAwaiter.waitForConnection(WIFI_CONNECTION_TIMEOUT))
  {
    Trace::log(TraceLevel::ERROR, "WiFi connection timeout");
  }
}

void GardenControllerApp::initializeStorage()
{
  StorageManager::getInstance().begin();
  Trace::log(TraceLevel::INFO, "StorageManager initialized");
}

void GardenControllerApp::initializeIdentity()
{
  strncpy(_clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
  _clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';
}

void GardenControllerApp::initializeConnectivity()
{
  _wifiManager.setup(WIFI_SSID, WIFI_PWD, _clientName);
  waitForWifiConnection();
  updateClientNameFromMac();
}

void GardenControllerApp::initializeSubsystems()
{
  _waterLevelManager.setup(_clientName);

  _mqttManager.configure(_irrigationConfig);
  _mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD,
                     _clientName);
  _connectivityCoordinator.ensureMqttConnected();

  setupOta();
  initIrrigationZones();
}

void GardenControllerApp::initializeRuntimeSafety()
{
  initWatchdog();
}

void GardenControllerApp::setup()
{
  Serial.begin(115200);
  Trace::log(TraceLevel::INFO, "Setup begin");

  initializeStorage();
  initializeIdentity();
  initializeConnectivity();
  initializeSubsystems();
  initializeRuntimeSafety();

  Trace::log(TraceLevel::INFO, "Setup end");
}

void GardenControllerApp::plotZoneStates(unsigned long currentTime)
{
  static unsigned long lastPlotTime = 0;
  if ((unsigned long)(currentTime - lastPlotTime) >= TELEPLOT_INTERVAL)
  {
    lastPlotTime = currentTime;
    char bufferName[20];
    for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
    {
      snprintf(bufferName, sizeof(bufferName), "Btn%d", i + 1);
      Trace::plotBoolState(bufferName, _irrigationZones[i].getBtnState(), 1);

      snprintf(bufferName, sizeof(bufferName), "Relay%d", i + 1);
      Trace::plotBoolState(bufferName, _irrigationZones[i].getRelayState(), -1);
    }
  }
}

void GardenControllerApp::handleShortIntervalTasks()
{
  _connectivityCoordinator.handleEvents();

  _mqttManager.publishAllIrrigationZones();
  _mqttManager.loop();

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    _irrigationZones[i].loop();

    if (i % 3 == 0)
    {
      yield();
    }
  }

  _waterLevelManager.loop(_currentMillis);
}

void GardenControllerApp::updateLoopTimingPlot()
{
#ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("Mainloop", 0, _currentMillis - _mainLoopStartTime);
  _mainLoopStartTime = _currentMillis;
#endif
}

void GardenControllerApp::handleMiddleIntervalEvents()
{
  _mqttManager.publishSystemStatus();
}

void GardenControllerApp::handleLongIntervalTasks()
{
  if (!_wifiManager.loop())
  {
    Trace::log(TraceLevel::ERROR, "WiFi connection issue detected");
  }

  _connectivityCoordinator.handleEvents();
}

void GardenControllerApp::loop()
{
  esp_task_wdt_reset();
  _currentMillis = millis();

  if (_otaLoopGuard.process())
  {
    return;
  }

  updateLoopTimingPlot();

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisShortLoop,
                               _systemConfig.shortIntervalMs))
  {
    handleShortIntervalTasks();
  }

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisMiddleLoop,
                               _systemConfig.middleIntervalMs))
  {
    handleMiddleIntervalEvents();
    yield();
  }

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisLongLoop,
                               _systemConfig.longIntervalMs))
  {
    handleLongIntervalTasks();
  }

#ifdef ENABLE_ZONE_PLOTTING
  plotZoneStates(_currentMillis);
#endif
}
