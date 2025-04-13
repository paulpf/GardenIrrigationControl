#ifndef CONFIG_H
#define CONFIG_H

// Hardware configuration for 8 irrigation zones
// Pins for Zone 1
#define ZONE1_BUTTON_PIN 23
#define ZONE1_RELAY_PIN 22

// Pins for Zone 2
#define ZONE2_BUTTON_PIN 21
#define ZONE2_RELAY_PIN 19

// Pins for Zone 3
#define ZONE3_BUTTON_PIN 18
#define ZONE3_RELAY_PIN 5

// Pins for Zone 4
#define ZONE4_BUTTON_PIN 17
#define ZONE4_RELAY_PIN 16

// Pins for Zone 5
#define ZONE5_BUTTON_PIN 4
#define ZONE5_RELAY_PIN 2

// Pins for Zone 6
#define ZONE6_BUTTON_PIN 15
#define ZONE6_RELAY_PIN 13

// Pins for Zone 7
#define ZONE7_BUTTON_PIN 12
#define ZONE7_RELAY_PIN 14

// Pins for Zone 8
#define ZONE8_BUTTON_PIN 27
#define ZONE8_RELAY_PIN 26

// Irrigation timing defaults
#define DEFAULT_DURATION_TIME 300 // Default duration in seconds (5 minutes)
#define MAX_DURATION_TIME 1800    // Maximum duration in seconds (30 minutes)

// System configuration
#define WATCHDOG_TIMEOUT 60000   // Watchdog timeout in milliseconds
#define LOOP_INTERVAL 500        // Main loop interval in milliseconds
#define DEBUG_MODE true          // Enable/disable verbose logging

// Maximum number of irrigation zones supported
#define MAX_IRRIGATION_ZONES 8

#endif // CONFIG_H