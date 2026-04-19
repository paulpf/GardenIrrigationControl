#ifndef CONFIG_H
#define CONFIG_H

// Pins configuration for ESP32

// GPIO 0 - usually used for boot mode selection, should not be used for other
// purposes GPIO 2 - usually used for boot mode selection, but can be used for
// other purposes GPIO 5 - usually used for SPI flash, but can be used for other
// purposes GPIO 34, 35, 36, 39 - input only pins, but they will not be used in
// this project because they are not have internal pull-down resistors

// Hardware configuration for 8 irrigation zones
// Pins for Zone 1
constexpr int ZONE1_BUTTON_PIN = 14;
constexpr int ZONE1_RELAY_PIN = 27;

// Pins for Zone 2
constexpr int ZONE2_BUTTON_PIN = 13;
constexpr int ZONE2_RELAY_PIN = 26;

// Pins for Zone 3
constexpr int ZONE3_BUTTON_PIN = 15;
constexpr int ZONE3_RELAY_PIN = 25;

// Pins for Zone 4
constexpr int ZONE4_BUTTON_PIN = 2;
constexpr int ZONE4_RELAY_PIN = 33;

// Pins for Zone 5
constexpr int ZONE5_BUTTON_PIN = 4;
constexpr int ZONE5_RELAY_PIN = 32;

// Pins for Zone 6
constexpr int ZONE6_BUTTON_PIN = 16;
constexpr int ZONE6_RELAY_PIN = 23;

// Pins for Zone 7
constexpr int ZONE7_BUTTON_PIN = 17;
constexpr int ZONE7_RELAY_PIN = 22;

// Pins for Zone 8
constexpr int ZONE8_BUTTON_PIN = 5;
constexpr int ZONE8_RELAY_PIN = 21;

// Hardware configuration for special pins
// Pin for drainage
constexpr int ZONE9_BUTTON_PIN = 18;
constexpr int ZONE9_RELAY_PIN = 19;

// Water level sensor (4-20mA via shunt resistor to ADC voltage)
constexpr int WATER_LEVEL_SENSOR_PIN = 36; // GPIO36 / VP (ADC1)

// ADC conversion defaults for ESP32 (12-bit range)
constexpr int WATER_LEVEL_ADC_MIN = 820;  // ~0.66V (4mA on 165 Ohm)
constexpr int WATER_LEVEL_ADC_MAX = 4095; // ~3.3V (20mA on 165 Ohm)
constexpr float WATER_LEVEL_CRITICAL_PERCENT = 10.0f;
constexpr float WATER_LEVEL_LOCKOUT_RELEASE_PERCENT = 12.0f;

// Irrigation timing defaults
constexpr int DEFAULT_DURATION_TIME =
    5 * 60 * 1000; // Default duration time in milliseconds (5 minutes)
constexpr int MAX_DURATION_TIME =
    60 * 60 * 1000; // Maximum duration time in milliseconds (1 hour)

// System configuration
constexpr int WATCHDOG_TIMEOUT = 60000; // Watchdog timeout in milliseconds
constexpr unsigned int WDT_TIMEOUT_SEC = WATCHDOG_TIMEOUT / 1000;
constexpr int LONG_INTERVAL = 60 * 1000;  // Main loop interval in milliseconds
constexpr int MIDDLE_INTERVAL = 1 * 1000; // Main loop interval in milliseconds
constexpr int SHORT_INTERVAL = 50;        // Main loop interval in milliseconds
constexpr int WATER_LEVEL_READ_INTERVAL =
    30 * 1000; // Water level reading interval in milliseconds (30 seconds)
constexpr int BUTTON_DEBOUNCE_TIME =
    500; // Button debounce time in milliseconds

#define TRACE_LEVEL TraceLevel::INFO // Enable/disable trace logging

// Maximum number of irrigation zones supported
constexpr int MAX_IRRIGATION_ZONES = 9;

// OTA (Over-The-Air) Update configuration
#define ENABLE_OTA true        // Enable/disable OTA functionality
constexpr int OTA_PORT = 3232; // OTA port (default: 3232)

// Serial plotting configuration
// #define ENABLE_ZONE_PLOTTING
// #define ENABLE_LOOP_TIME_PLOTTING // Enable loop time plotting for debugging
constexpr int TELEPLOT_INTERVAL = 1000; // Plotting-Intervall in ms

// WiFi configuration
constexpr int WIFI_CONNECTION_TIMEOUT =
    10000; // WiFi connection timeout in milliseconds (10 seconds)

#endif // CONFIG_H