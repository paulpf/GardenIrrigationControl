# GardenIrrigationControl

## Table of Contents

1. [Introduction](#introduction)

## Introduction

The idea of this project is to control the irrigation of a garden using ESP32 and a relay module.
The ESP32 will be connected to a Wi-Fi network and will be able to receive commands to turn on or off the relay module.
The relay module will be connected to ventils that will control the water flow.
Additionally, the ESP32 measure the level of the water in the tank and send it to a server.

There should be various options for controlling the irrigation:

- via a web interface
- via mqtt
- via physical buttons.
