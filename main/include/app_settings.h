#ifndef APP_SETTINGS_H_
#define APP_SETTINGS_H_

#include "lwip/ip4_addr.h"

#define LEN_WIFI_SSID     32
#define LEN_WIFI_PASSWORD 64
#define LEN_HOSTNAME      32
#ifdef CONFIG_MDNS_ENABLED
#define LEN_MDNS_INSTANCE 32
#endif
#ifdef CONFIG_SNTP_ENABLED
#define LEN_NTP_SERVER    32
#define LEN_TIMEZONE      32
#endif

struct app_settings_t {
  int size;
  char wifi_ssid[LEN_WIFI_SSID];
  char wifi_password[LEN_WIFI_PASSWORD];
  char hostname[LEN_HOSTNAME];
  #ifdef CONFIG_MDNS_ENABLED
  char mdns_instance[LEN_MDNS_INSTANCE];
  #endif
  #ifdef CONFIG_SNTP_ENABLED
  char ntp_server[LEN_NTP_SERVER];
  char timezone[LEN_TIMEZONE];
  #endif
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