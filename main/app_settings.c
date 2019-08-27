#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sdkconfig.h"
#include "lwip/ip4_addr.h"
//#include "ssd1306.h"
#include "app_settings.h"

static const char* TAG = "settings";
static const char* NVS_KEY = "settings";

static void log_settings() {
  ESP_LOGI(TAG," hostname=%s",settings.hostname);
  ESP_LOGI(TAG," wifi_ssid=%s",settings.wifi_ssid);
  ESP_LOGI(TAG," wifi_password=%s",settings.wifi_password);
  #ifdef CONFIG_MDNS_ENABLED
  ESP_LOGI(TAG," mdns_instance=%s",settings.mdns_instance);
  #endif
  #ifdef CONFIG_SNTP_ENAblED
  ESP_LOGI(TAG," ntp_server=%s",settings.ntp_server);
  ESP_LOGI(TAG," timezone=%s",settings.timezone);
  #endif
  ESP_LOGI(TAG," dhcp=%u",settings.dhcp);
  ESP_LOGI(TAG," ip=%s",ip4addr_ntoa(&settings.ip));
  ESP_LOGI(TAG," netmask=%s",ip4addr_ntoa(&settings.netmask));
  ESP_LOGI(TAG," gateway=%s",ip4addr_ntoa(&settings.gateway));
  ESP_LOGI(TAG," dns1=%s",ip4addr_ntoa(&settings.dns1));
  ESP_LOGI(TAG," dns2=%s",ip4addr_ntoa(&settings.dns2));
}

void app_settings_reset() {
  nvs_handle_t handle;

  ESP_LOGI(TAG,"Erasing settings from NVS");
  if (nvs_open(NVS_KEY,NVS_READWRITE,&handle) == ESP_OK) {
    nvs_erase_all(handle);
    nvs_close(handle);
  }
  ESP_LOGI(TAG,"Restoring default settings");
  memset(&settings,0,sizeof(settings));
  strncpy(settings.wifi_ssid,CONFIG_ESP_WIFI_SSID,LEN_WIFI_SSID );
  strncpy(settings.wifi_password,CONFIG_ESP_WIFI_PASSWORD,LEN_WIFI_PASSWORD);
  strncpy(settings.hostname,CONFIG_LWIP_LOCAL_HOSTNAME,LEN_HOSTNAME);
  #ifdef CONFIG_MDNS_ENABLED
  strncpy(settings.mdns_instance,CONFIG_MDNS_INSTANCE,LEN_MDNS_INSTANCE);
  #endif
  #ifdef CONFIG_SNTP_ENABLED
  strncpy(settings.ntp_server,CONFIG_NTP_SERVER,LEN_NTP_SERVER);
  strncpy(settings.timezone,CONFIG_TIMEZONE,LEN_TIMEZONE);
  #endif
  settings.dhcp = true;  
}

void app_settings_save() {
  nvs_handle_t handle;
  esp_err_t ret;

  ret = nvs_open(NVS_KEY,NVS_READWRITE,&handle);
  if (ret == ESP_OK) {
    settings.size = sizeof(settings);
    ret = nvs_set_blob(handle,"settings",&settings,sizeof(settings));
    if (ret == ESP_OK) {
      nvs_commit(handle);
      ESP_LOGI(TAG,"Saved settings to NVS");
      log_settings();
    } else {
      ESP_LOGE(TAG,"Error (%d) saving settings to NVS",ret);
    }
    nvs_close(handle);
  } else {
    ESP_LOGE(TAG,"Error (%d) opening settings",ret);
  }
}

void app_settings_startup() {
  nvs_handle_t handle;

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);  

  ESP_LOGI(TAG,"NVS Flash Init");
  
  ret = nvs_open(NVS_KEY,NVS_READONLY,&handle);
  if (ret == ESP_OK) {
    size_t size = sizeof(settings);
    ret = nvs_get_blob(handle,"settings",&settings,&size);
    if (ret == ESP_OK) { 
      if (settings.size == sizeof(settings)) {
        ESP_LOGI(TAG,"Settings loaded from NVS");
        log_settings();
      } else {
        app_settings_reset();
        app_settings_save();
      }
    } else {
      app_settings_reset();
      app_settings_save();
    }
    nvs_close(handle);
  } else {
    app_settings_reset();
    app_settings_save(); 
  } 
}

void app_settings_shutdown() {
  app_settings_save();
  nvs_flash_deinit();
}