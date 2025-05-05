# GardenIrrigationControl

> **Note:** This project is still under construction.

<img src="_assets/_images/underconstruction.png" alt="alt text" width="500" style="height: 80;">

## Table of Contents

- [GardenIrrigationControl](#gardenirrigationcontrol)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Requirements](#requirements)
  - [Hardware](#hardware)
    - [ESP-32 Dev Kit C V4](#esp-32-dev-kit-c-v4)
      - [Pinout](#pinout)
    - [Relais module](#relais-module)
    - [Ventil Hunter PGV-101](#ventil-hunter-pgv-101)
  - [Circuit Diagram](#circuit-diagram)
- [ESP32 GPIO Assignment for 9 Inputs with Internal Pull-down Resistors and 9 SSR Outputs](#esp32-gpio-assignment-for-9-inputs-with-internal-pull-down-resistors-and-9-ssr-outputs)
  - [Selected GPIOs for Your Application](#selected-gpios-for-your-application)
  - [Important Notes](#important-notes)

## Introduction

The idea of this project is to control the irrigation of a garden using ESP32 and a relay module.
The ESP32 will be connected to a Wi-Fi network and will be able to receive commands to turn on or off the relay module.
The relay module will be connected to ventils that will control the water flow.
Additionally, the ESP32 measure the level of the water in the tank and send it to a server.

## Requirements

The system should:

- be controlled via a web interface
- be able to receive commands and send data via mqtt
- be able to receive commands via physical buttons
- be able to work without WLAN connection (in case if irrigation is started via physical buttons)
- be able to measure the water level in the tank via capacitive sensor. This sensor has 4-20mA output.

## Hardware

### ESP-32 Dev Kit C V4

<img src="_assets/_images/esp32DevKitCV4.png" width="500" alt="ESP32 Dev Kit C V4">

- Product name: ESP32 Dev Kit C V4
- Seller: AZ-Delivery

#### Pinout

<img src="_assets/_images/esp32DevKitCV4PinOut.png" width="1200" alt="ESP32 Pinout">

Source: [ESP32 Pinout](https://www.cnx-software.com/wp-content/uploads/2022/09/ESP32-DevkitC-V4-pinout-diagram.jpg)

### Relais module

<img src="_assets/_images/8chSSRModule.png" width="500" alt="8 Channel SSR Module">

- Product name: 8 Kanal Solid State Relais 5V DC Low Level
- Seller: AZ-Delivery

### Ventil Hunter PGV-101




## Circuit Diagram


<img src="_assets/_images/IrrigationCircuit_Steckplatine.png" width="1000" alt="Circuit Diagram">



# ESP32 GPIO Assignment for 9 Inputs with Internal Pull-down Resistors and 9 SSR Outputs

## Selected GPIOs for Your Application

| Function | Description | GPIO | Board Label | Internal Pull-down | Notes |
|----------|-------------|------|------------|-----------|-------|
| **Inputs (with Internal Pull-down)** |
| Input 1 | Digital Input | GPIO39 | D39 | ✓ | Internal pull-down ~45kΩ |
| Input 2 | Digital Input | GPIO34 | D34 | ✓ | Internal pull-down ~45kΩ |
| Input 3 | Digital Input | GPIO35 | D35 | ✓ | Internal pull-down ~45kΩ |
| Input 4 | Digital Input | GPIO32 | D32 | ✓ | Internal pull-down ~45kΩ |
| Input 5 | Digital Input | GPIO33 | D33 | ✓ | Internal pull-down ~45kΩ |
| Input 6 | Digital Input | GPIO25 | D25 | ✓ | Internal pull-down ~45kΩ |
| Input 7 | Digital Input | GPIO26 | D26 | ✓ | Internal pull-down ~45kΩ |
| Input 8 | Digital Input | GPIO27 | D27 | ✓ | Internal pull-down ~45kΩ |
| Drainage Input | Digital Input | GPIO14 | D14 | ✓ | Internal pull-down ~45kΩ |
| **Outputs (for SSR modules with Low-Level-Trigger)** |
| Relay 1 | Low-Level-Trigger | GPIO23 | D23 | - | Activation by LOW signal. During booting, this GPIO set the output for < = 1 second to LOW. |
| Relay 2 | Low-Level-Trigger | GPIO25 | D25 | - | Activation by LOW signal |
| Relay 3 | Low-Level-Trigger | GPIO26 | D26 | - | Activation by LOW signal |
| Relay 4 | Low-Level-Trigger | GPIO27 | D27 | - | Activation by LOW signal |
| Relay 5 | Low-Level-Trigger | GPIO32 | D32 | - | Activation by LOW signal. During booting, this GPIO set the output for < = 1 second to LOW. |
| Relay 6 | Low-Level-Trigger | GPIO33 | D33 | - | Activation by LOW signal |
| Relay 7 | Low-Level-Trigger | GPIO21 | D21 | - | Activation by LOW signal |
| Relay 8 | Low-Level-Trigger | GPIO22 | D22 | - | Activation by LOW signal |
| Drainage Relay | Low-Level-Trigger | GPIO19 | D19 | - | Activation by LOW signal |


## Important Notes

1. **Avoided GPIOs:**
   - GPIO0, GPIO1, GPIO3: These were avoided as they are used for booting and serial communication (debugging).
   - GPIO6-11: These are connected to the SPI flash and not available.
   - GPIO12: Avoided due to potential boot issues.
   - GPIO34, GPIO35, GPIO36, GPIO39: These are input-only and are not support pull-up or pull-down resistors.

2. **Special Considerations:**
   - GPIO2 (SSR 9): Connected to the onboard LED. When activating the SSR (LOW signal), the LED will light up.
   - Internal Pull-down Resistors: The ESP32 has built-in programmable pull-down resistors (approximately 45kΩ) that can be enabled via software, eliminating the need for external pull-down resistors.

3. **Low-Level-Trigger SSRs:**
   - Since SSR modules with Low-Level-Trigger are used, a LOW signal is needed to activate and a HIGH signal to deactivate.
   - Initially, all SSR outputs should be set to HIGH to avoid unintended activation.

4. **External Circuitry:**
   - When connecting SSR modules to the GPIOs, it may be advisable to use external protection circuits (such as optocouplers) to protect the ESP32 from reverse currents.
   - Check the specifications of your SSR modules to ensure they are directly compatible with the 3.3V logic levels of the ESP32.