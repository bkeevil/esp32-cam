#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "sdkconfig.h"

#include "idf_version.h"
#if defined(__USE_TCP_ADAPTOR)
#include <tcpip_adapter.h>
#else
#include <esp_netif.h>
#endif
//#include "ssd1306.h"
#include "app_settings.h"
#include "scut.h"

static const char* TAG = "settings";
static const char* NVS_KEY = "settings";

static void log_settings() {
  ESP_LOGI(TAG, " hostname=%s", settings.hostname);
  ESP_LOGI(TAG, " wifi_ssid=%s", settings.wifi_ssid);
  ESP_LOGI(TAG, " wifi_password=%s", settings.wifi_password);
  #ifdef CONFIG_MDNS_ENABLED
  ESP_LOGI(TAG, " mdns_instance=%s", settings.mdns_instance);
  #endif
  #ifdef CONFIG_SNTP_ENABLED
  ESP_LOGI(TAG, " ntp_server=%s",settings.ntp_server);
  ESP_LOGI(TAG, " timezone=%s",settings.timezone);
  #endif
  ESP_LOGI(TAG, " dhcp=%u", settings.dhcp);
  ESP_LOGI(TAG, " ip="      IPSTR, IP2STR(&settings.ip));
  ESP_LOGI(TAG, " netmask=" IPSTR, IP2STR(&settings.netmask));
  ESP_LOGI(TAG, " gateway=" IPSTR, IP2STR(&settings.gateway));
  ESP_LOGI(TAG, " dns1="    IPSTR, IP2STR(&settings.dns1));
  ESP_LOGI(TAG, " dns2="    IPSTR, IP2STR(&settings.dns2));
  ESP_LOGI(TAG, " fps=%u", settings.fps);
  ESP_LOGI(TAG, " auth=%u", settings.http_auth);
  ESP_LOGI(TAG, " http_user=%s", settings.http_user);
  ESP_LOGI(TAG, " http_password=%s", settings.http_password);
}

void app_settings_reset() {
  nvs_handle_t handle;

  ESP_LOGI(TAG,"Erasing all settings from NVS");
  if (nvs_open(NVS_KEY,NVS_READWRITE,&handle) == ESP_OK) {
    nvs_erase_all(handle);
    nvs_close(handle);
  }
  ESP_LOGI(TAG,"Restoring default settings");
  memset(&settings,0,sizeof(settings));
  scut(settings.wifi_ssid, CONFIG_ESP_WIFI_SSID, (((sizeof(settings.wifi_ssid)) >= (sizeof(CONFIG_ESP_WIFI_SSID))) ? (sizeof(CONFIG_ESP_WIFI_SSID)) : (sizeof(settings.wifi_ssid))));
  scut(settings.wifi_password, CONFIG_ESP_WIFI_PASSWORD, (((sizeof(settings.wifi_password)) >= (sizeof(CONFIG_ESP_WIFI_PASSWORD))) ? (sizeof(CONFIG_ESP_WIFI_PASSWORD)) : (sizeof(settings.wifi_password))));
  scut(settings.hostname, CONFIG_LWIP_LOCAL_HOSTNAME, (((sizeof(settings.hostname)) >= (sizeof(CONFIG_LWIP_LOCAL_HOSTNAME))) ? (sizeof(CONFIG_LWIP_LOCAL_HOSTNAME)) : (sizeof(settings.hostname))));
  #ifdef CONFIG_MDNS_ENABLED
  scut(settings.mdns_instance, CONFIG_MDNS_INSTANCE, (((sizeof(settings.mdns_instance)) >= (sizeof(CONFIG_MDNS_INSTANCE))) ? (sizeof(CONFIG_MDNS_INSTANCE)) : (sizeof(settings.mdns_instance))));
  #endif
  #ifdef CONFIG_SNTP_ENABLED
  scut(settings.ntp_server, CONFIG_NTP_SERVER, (((sizeof(settings.ntp_server)) >= (sizeof(CONFIG_NTP_SERVER))) ? (sizeof(CONFIG_NTP_SERVER)) : (sizeof(settings.ntp_server))));
  scut(settings.timezone, CONFIG_TIMEZONE, (((sizeof(settings.timezone)) >= (sizeof(CONFIG_TIMEZONE))) ? (sizeof(CONFIG_TIMEZONE)) : (sizeof(settings.timezone))));
  #endif
  settings.dhcp = true;
  settings.fps = 0;
  #ifdef CONFIG_DEF_HTTP_AUTH_ENABLED
  settings.http_auth = true;
  scut(settings.http_user, CONFIG_DEF_HTTP_USER, (((sizeof(settings.http_user)) >= (sizeof(CONFIG_DEF_HTTP_USER))) ? (sizeof(CONFIG_DEF_HTTP_USER)) : (sizeof(settings.http_user))));
  scut(settings.http_password, CONFIG_DEF_HTTP_PASSWORD, (((sizeof(settings.http_password)) >= (sizeof(CONFIG_DEF_HTTP_PASSWORD))) ? (sizeof(CONFIG_DEF_HTTP_PASSWORD)) : (sizeof(settings.http_password))));
  #else
  settings.http_auth = false;
  #endif
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
