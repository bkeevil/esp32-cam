# ESP32-CAM

The goal of this project is to create a functional webcam firmware for ESP32 based camera boards using the ESP-IDF framework. This project was originally forked from an ESP-WHO face recognition example but the face recognition code has been removed. In its place I have added:

* An SSD1306 display driver
* A font library for the lcd and image overlays
* Persistent storage of camera and network settings in NVS
* An improved web interface
* The ability to store Wifi and network settings from the web interface
* An NTP server for time overlays
