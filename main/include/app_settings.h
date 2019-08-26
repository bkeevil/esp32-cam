#ifndef APP_SETTINGS_H_
#define APP_SETTINGS_H_

#include "esp_camera.h"
#include "sensor.h"
#include "lwip/ip4_addr.h"

#define LEN_WIFI_SSID     32
#define LEN_WIFI_PASSWORD 64
#define LEN_HOSTNAME      32
#define LEN_MDNS_INSTANCE 32
#define LEN_NTP_SERVER   32
#define LEN_TIMEZONE      32

struct app_settings_t {
  char wifi_ssid[LEN_WIFI_SSID];
  char wifi_password[LEN_WIFI_PASSWORD];
  char hostname[LEN_HOSTNAME];
  bool mdns;
  char mdns_instance[LEN_MDNS_INSTANCE];
  bool ntp;
  char ntp_server[LEN_NTP_SERVER];
  char timezone[LEN_TIMEZONE];
  bool dhcp;
  ip4_addr_t ip;
  ip4_addr_t netmask;
  ip4_addr_t gateway;
  ip4_addr_t dns1;
  ip4_addr_t dns2;
} settings;

void app_settings_save();
void app_settings_reset();
void app_settings_startup();
void app_settings_shutdown();

#endif