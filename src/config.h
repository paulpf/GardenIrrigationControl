#ifndef CONFIG_H
#define CONFIG_H

// Pins configuration for ESP32

// GPIO 0 - usually used for boot mode selection, should not be used for other purposes
// GPIO 2 - usually used for boot mode selection, but can be used for other purposes
// GPIO 5 - usually used for SPI flash, but can be used for other purposes
// GPIO 34, 35, 36, 39 - input only pins, but they will not be used in this project
// because they are not have internal pull-down resistors


// Hardware configuration for 8 irrigation zones
// Pins for Zone 1
#define ZONE1_BUTTON_PIN 14
#define ZONE1_RELAY_PIN 27

// Pins for Zone 2
#define ZONE2_BUTTON_PIN 13
#define ZONE2_RELAY_PIN 26

// Pins for Zone 3
#define ZONE3_BUTTON_PIN 15
#define ZONE3_RELAY_PIN 25

// Pins for Zone 4
#define ZONE4_BUTTON_PIN 2
#define ZONE4_RELAY_PIN 33

// Pins for Zone 5
#define ZONE5_BUTTON_PIN 4
#define ZONE5_RELAY_PIN 32  

// Pins for Zone 6
#define ZONE6_BUTTON_PIN 16
#define ZONE6_RELAY_PIN 23

// Pins for Zone 7
#define ZONE7_BUTTON_PIN 17
#define ZONE7_RELAY_PIN 22

// Pins for Zone 8
#define ZONE8_BUTTON_PIN 5
#define ZONE8_RELAY_PIN 21

// Hardware configuration for special pins
// Pin for drainage
#define DRAINAGE_BUTTON_PIN 18
#define DRAINAGE_RELAY_PIN 19  


// Irrigation timing defaults
#define DEFAULT_DURATION_TIME 5 * 60 * 1000 // Default duration time in milliseconds (5 minutes)
#define MAX_DURATION_TIME 60 * 60 * 1000    // Maximum duration time in milliseconds (1 hour)

// System configuration
#define WATCHDOG_TIMEOUT 60000   // Watchdog timeout in milliseconds
#define LONG_INTERVAL 60 * 1000        // Main loop interval in milliseconds
#define SHORT_INTERVAL 100        // Main loop interval in milliseconds
#define BUTTON_DEBOUNCE_TIME 500 // Button debounce time in milliseconds

#define DEBUG_MODE true          // Enable/disable verbose logging
#define TRACE_LEVEL TraceLevel::ERROR       // Enable/disable trace logging

// Maximum number of irrigation zones supported
#define MAX_IRRIGATION_ZONES 9

// Serial plotting configuration
#define ENABLE_ZONE_PLOTTING
#define ENABLE_LOOP_TIME_PLOTTING // Enable loop time plotting for debugging
#define TELEPLOT_INTERVAL 1000  // Plotting-Intervall in ms

#endif // CONFIG_H