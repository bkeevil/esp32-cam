#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "lwip/ip4_addr.h"
#include "mdns.h"
#include "app_settings.h"
#include "app_wifi.h"
#include "app_httpd.h"
#include "app_mdns.h"

static EventGroupHandle_t _event_group = 0;

void app_mdns_startup(EventGroupHandle_t event_group) {
  _event_group = event_group;
	EventBits_t uxBits;
  
  for (;;) {
	  uxBits = xEventGroupWaitBits(_event_group,WIFI_CONNECTED_BIT | WIFI_SOFTAP_BIT,pdFALSE,pdFALSE,500 / portTICK_PERIOD_MS);
	  if ((uxBits & WIFI_CONNECTED_BIT) == WIFI_CONNECTED_BIT) {	  
      //initialize mDNS
      ESP_ERROR_CHECK( mdns_init() );
      //set mDNS hostname (required if you want to advertise services)
      ESP_ERROR_CHECK(mdns_hostname_set(settings.hostname));
      //set default mDNS instance name
      ESP_ERROR_CHECK(mdns_instance_name_set(settings.mdns_instance));

      //structure with TXT records
      mdns_txt_item_t serviceTxtData[1] = {
        {"path","/"}
      };

      //initialize service
      ESP_ERROR_CHECK( mdns_service_add(settings.mdns_instance, "_http", "_tcp", 80, serviceTxtData, 1) );
      return;
    }
  }
}

void app_mdns_shutdown() {
  mdns_free();
}