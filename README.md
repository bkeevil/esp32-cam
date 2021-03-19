# ESP32-CAM-BGR
... based on [bkeevil/esp32-cam](https://github.com/bkeevil/esp32-cam)

## Features fixed / added

* added basic http authentication
  * you can set deafault username / password using menuconfig
  * it is possible to disable auth completely
  * stream url is <ip-of-your-camera>:81/stream-<password> when auth is on, there is no auth on the stream itself
* added fps limiter (default off)
  * can be used to reduce CPU load / heat
* fixed resolution switcher - new driver added new resolutions and there was mistmach
* "Reboot Camera" button delay the refresh of the page by 10 secponds), this gives ESP some time to reboot - otherwise the page can't refresh correctly

## Notes

* to enable PSRAM correctly according to current implementation of the camera driver go to Component config -> ESP32 specific -> SPI RAM config -> SPI RAM ACCESS METHOD -> using heap_caps_malloc

# ORIGINAL README of ESP32-CAM by bkeevil

The goal of this project is to create a functional webcam firmware for ESP32 based camera boards with OV2640 modules using the ESP-IDF framework. Such modules often come with optional components like high power LED "flash", an OLED display, or a motion detector. This project will incorporate compile time support for optional libraries to support these components.

![Screenshot Image](/images/screenshot.png "Screenshot")

This project was originally forked from one of the example programs for the ESP-WHO face recognition framework by Espressif. The face detection and recognition code has been removed to provide more resources for other features and to eliminate a dependency on external SPI RAM. In its place I have added:

* An improved web interface
* The ability to modify wifi and network settings from the web interface
* Persistent storage of camera and other settings in NVS
* An SSD1306 display driver and code to outputs pertinent information about the wifi connection and frame rate
* An LED Illuminator driver to control the intensity of an LED flash
* An mDNS server to announce camera services to the local network
* Implement the NTP client and RTC for date/time

The current roadmap involves finishing testing of the features above and creating an initial release before moving on to some planned new features:

* A basic font library for the lcd and image overlays
* Playback of image on LCD screen
* A basic motion detection library [See this](https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/) [and this](https://github.com/alanesq/CameraWifiMotion)
* Record and playback from an SD card
* Basic ONVIF support MJPEG streams only.

The resources available on the ESP32 to support cameras are very limited and users should not expect to see anything like the full resolutions and frame rates the OV2640 is capable of.

## License

Espressif originally released their source code under the MIT license. I will release my contributions to this source code under the same license:

 ESPRESSIF MIT License
 
 Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 
 Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 it is free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Installation

The source code requires the [installation of the ESP-IDF toolchain and development framework](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)

**The source code works with the release/v4.0 branch.**

Once that is done, create a folder for your projects and set the path and environment variables of your shell:

```
cd /home/username/esp/esp-idf
. ./export.sh
cd ..
mkdir myprojects
cd myprojects
```

clone the repository using the recursive option:

```
git clone --recursive https://github.com/bkeevil/esp32-cam.git
cd esp32-cam
```  

Compress the stylesheet, html and javascript pages in main/www

```
cd main/www
./compress_pages.sh
cd ../..
```

To configure the source code you first need to run `idf.py menuconfig` from the root of the source tree. This will bring up an ncurses based repository editor.

Important settings are:

- Select a USB port for uploading the firmware under *Serial Flasher Config -> Default serial port*
- Select what board you are using under *Camera Web Server -> Camera Pins*
- If your board has an LED flash, enable it under *Camera Web Server -> LED Illuminator*
- You can select default wifi settings under *Camera Web Server -> Wifi Settings*
- If your board has an SSD1306 based OLED display, enable it under *Component Config -> SSD1306 Configuration* and select pins for SDA and SCL
- To use additional external SPI RAM that may be on your board, enable it using *Component Config -> ESP32 Specific -> Support for external, SPI-connected RAM*
- To enable font overlays, under *Font Configuration* check *Store Font Partition* (Uses approximately 240K of flash)
- Enable NTP and select a default NTP server and timezone under *NTP configuration*

When your settings are complete, save them and exit.

If needed, clean your build directory with `idf.py fullclean`

Compile the source code by executing `idf.py build`

Flash the binary file to your ESP32 module using `idf.py flash` 

Monitor the debug serial output by running `idf.py monitor`

### Upgrade Notes

I moved the offset of the partition table in flash from 8000 to 18000. If you get the error message "Detected overlap at address: 0x8000 for file: partition_table/partition-table.bin" then check the following line in `sdkconfig` and `sdkconfig.defaults`

```
CONFIG_PARTITION_TABLE_OFFSET=0x18000
```

### Windows 10 Installation Notes

Instead of using make xxxx commands, you have to use idf.py xxxx commands. For example:

```
idf.py build
idf.py menuconfig
idf.py app
idf.py -p Portname app-flash
```

Start ESP-IDF Command Prompt from Windows Start Menu to have idf.py available automatically.

To further simplify the build process, create a build-windows-defaultcomport file (without any extension) with your devices COM port, e.g. COM15, then from within the ESP-IFD prompt run the build script.
```
echo COM15 > build-windows-defaultcomport
powershell.exe .\build-windows.ps1
```

## First Run

When the firmware first starts up, it will try to establish a wifi connection in the following order:

1. Establish a connection using the SSID and password stored in non-volatile storage (nvs)
2. Establish a connection using the default STA SSID and password provided by `make menuconfig`
3. After five failed attempts at starting in station (STA) mode, it will fall back to SoftAP mode and act as a wifi access point

To connect to the access point, point a browser to 192.168.4.1. There is no username or password.

From the web interface, select a Wifi SSID and password and optionally specify a static IP address and other network settings. Reboot the device for the settings to take effect.

## Web Camera URLS

Picture: http://IP/capture

Stream: http://IP:81/stream

Control LED: http://IP/control?var=led_intensity&val=[0...255]

## Notes

- Although the ESP32-Camera driver provides support for the OV3660 sensor, it is not supported by this firmware due to the unavailability of these sensors on the market for testing purposes.
- The esp_restart() function used by the web interface for a software reboot does not work on my boards. You might have to unplug the device to force a hardware restart.

## Contributions 

Developer contributions that further the goals of this project are most welcome.
