#pragma once

using ota_error_t = int;

constexpr ota_error_t OTA_AUTH_ERROR = 0;
constexpr ota_error_t OTA_BEGIN_ERROR = 1;
constexpr ota_error_t OTA_CONNECT_ERROR = 2;
constexpr ota_error_t OTA_RECEIVE_ERROR = 3;
constexpr ota_error_t OTA_END_ERROR = 4;

class ArduinoOTAClass
{
public:
  template <typename T> ArduinoOTAClass &onStart(T)
  {
    return *this;
  }

  template <typename T> ArduinoOTAClass &onEnd(T)
  {
    return *this;
  }

  template <typename T> ArduinoOTAClass &onProgress(T)
  {
    return *this;
  }

  template <typename T> ArduinoOTAClass &onError(T)
  {
    return *this;
  }

  void setHostname(const char *) {}
  void setPassword(const char *) {}
  void begin() {}
  void handle() {}
};

static ArduinoOTAClass ArduinoOTA;
