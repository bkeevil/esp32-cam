#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/sys.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "app_settings.h"
#include "app_wifi.h"

static const char *TAG = "wifi station";

extern EventGroupHandle_t event_group;

static int s_retry_num = 0;

static char *wifi_authmode_to_str(wifi_auth_mode_t mode) {
	switch(mode) {
	case WIFI_AUTH_OPEN:
		return "Open";
	case WIFI_AUTH_WEP:
		return "WEP";
	case WIFI_AUTH_WPA_PSK:
		return "WPA";
	case WIFI_AUTH_WPA2_PSK:
		return "WPA2";
	case WIFI_AUTH_WPA_WPA2_PSK:
		return "WPA_WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2 Enterprise";
	default:
		return "Unknown";
	}
}

float wifi_get_tx_power() {
  int8_t tx_power;
  ESP_ERROR_CHECK(esp_wifi_get_max_tx_power(&tx_power));
  return tx_power * 0.25;
}

int wifi_get_rssi() {
  wifi_ap_record_t apinfo;

  ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&apinfo));
  return apinfo.rssi;
}

int wifi_connection_count() {
  wifi_sta_list_t list;
  if (esp_wifi_ap_get_sta_list(&list) == ESP_OK) {
      return list.num;
  } else {
      return 0;
  }
}

static void wifi_dump_ap_info() {
    wifi_ap_record_t apinfo;
    int8_t tx_power;

    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&apinfo));
    ESP_ERROR_CHECK(esp_wifi_get_max_tx_power(&tx_power));
    ESP_LOGI(TAG,"Primary Channel: %d",apinfo.primary);
    ESP_LOGI(TAG,"Secondary Channel: %d",apinfo.second);
    ESP_LOGI(TAG,"RSSI: %d",apinfo.rssi);
    ESP_LOGI(TAG,"Auth Mode: %s",wifi_authmode_to_str(apinfo.authmode));
    ESP_LOGI(TAG,"Country Code: %s",apinfo.country.cc);
    ESP_LOGI(TAG,"Transmit Power: %3.1f dBm",tx_power * 0.25);
    ESP_LOGI(TAG,"Phy Mode: 11%s%s%s %s",apinfo.phy_11b ? "B" : "",apinfo.phy_11g ? "G" : "", apinfo.phy_11n ? "N" : "", apinfo.phy_lr ? "LR" : "");
}

void wifi_init_softap()
{
    if (strcmp(CONFIG_SERVER_IP, "192.168.4.1"))
    {
        int a, b, c, d;
        sscanf(CONFIG_SERVER_IP, "%d.%d.%d.%d", &a, &b, &c, &d);
        tcpip_adapter_ip_info_t ip_info;
        IP4_ADDR(&ip_info.ip, a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(WIFI_IF_AP));
        ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(WIFI_IF_AP, &ip_info));
        ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(WIFI_IF_AP));
    }
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    snprintf((char*)wifi_config.ap.ssid, 32, "%s", CONFIG_ESP_WIFI_AP_SSID);
    wifi_config.ap.ssid_len = strlen((char*)wifi_config.ap.ssid);
    snprintf((char*)wifi_config.ap.password, 64, "%s", CONFIG_ESP_WIFI_AP_PASSWORD);
    wifi_config.ap.max_connection = 1;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    if (strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    xEventGroupSetBits(event_group, WIFI_SOFTAP_BIT);
    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             CONFIG_ESP_WIFI_AP_SSID, CONFIG_ESP_WIFI_AP_PASSWORD);
}

static void wifi_init_sta(void) {
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    snprintf((char*)wifi_config.sta.ssid, 32, "%s", settings.wifi_ssid);
    snprintf((char*)wifi_config.sta.password, 64, "%s", settings.wifi_password);
    ESP_LOGI(TAG, "Connecting to AP SSID:%s password:%s",
        wifi_config.sta.ssid, wifi_config.sta.password);

    wifi_country_t wifi_country = {
        .cc = "",
        .schan = 1,
        .nchan = 14,
        .max_tx_power = 127,
        .policy = WIFI_COUNTRY_POLICY_AUTO
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country));
 
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             wifi_config.sta.ssid, wifi_config.sta.password);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{      
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);         
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG,"Station disconnected (reason=%d)",event->reason);
        if (s_retry_num <CONFIG_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        } else {
            s_retry_num = 0;
            esp_wifi_stop();
            ESP_LOGI(TAG, "Restarting wifi in SoftAP mode.");
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
            wifi_init_softap();
            ESP_ERROR_CHECK(esp_wifi_start());
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        settings.ip = event->ip_info.ip;
        ESP_LOGI(TAG, "got ip:%s",ip4addr_ntoa(&settings.ip));
        s_retry_num = 0;
        xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_dump_ap_info();
    }
}

void app_wifi_startup() {
    tcpip_adapter_init();
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA,settings.hostname);
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_AP,settings.hostname);
    if (!settings.dhcp) {
      tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
      tcpip_adapter_ip_info_t info;
      info.ip.addr = settings.ip.addr;
      info.gw.addr = settings.gateway.addr;
      info.netmask.addr = settings.netmask.addr;
	  tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &info);
      dns_setserver(1, &settings.dns1);
      dns_setserver(2, &settings.dns2);
    }
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_init_sta();
    
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(127));
}

void app_wifi_shutdown() {
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
}