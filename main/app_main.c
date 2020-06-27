#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "app_settings.h"
#include "app_wifi.h"
#ifdef CONFIG_CAMERA_ENABLED
#include "app_camera.h"
#include "app_camera_httpd.h"
#endif
#include "app_lcd.h"
#ifdef CONFIG_MDNS_ENABLED
#include "mdns.h"
#endif
#ifdef CONFIG_SNTP_ENABLED
#include "app_sntp.h"
#endif
#include "app_weather.h"

EventGroupHandle_t event_group;

void app_shutdown() {
  app_settings_shutdown();
  #ifdef CONFIG_USE_SSD1306_LCD_DRIVER
  app_lcd_shutdown();
  #endif
  #ifdef CONFIG_LED_ILLUMINATOR_ENABLED
  app_illuminator_shutdown();
  #endif
  #ifdef CONFIG_MDNS_ENABLED
  mdns_free();
  #endif
  #ifdef CONFIG_CAMERA_ENABLED
  app_camera_httpd_shutdown();
  app_camera_shutdown();
  #endif
  app_wifi_shutdown();

}

void app_main()
{
  EventBits_t uxBits;

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  event_group = xEventGroupCreate();

  app_settings_startup();

  #ifdef CONFIG_CAMERA_ENABLED
  app_camera_startup();
  #endif
  #ifdef CONFIG_LED_ILLUMINATOR_ENABLED
  app_illuminator_startup();
  #endif
  app_wifi_startup();

  for (;;) {
	  uxBits = xEventGroupWaitBits(event_group,WIFI_CONNECTED_BIT | WIFI_SOFTAP_BIT,pdFALSE,pdFALSE,500 / portTICK_PERIOD_MS);
	  if (uxBits > 0) {
      #ifdef CONFIG_SNTP_ENABLED
      app_sntp_startup();
      #endif
      app_weather_startup();
      #ifdef CONFIG_MDNS_ENABLED
      ESP_ERROR_CHECK(mdns_init());
      ESP_ERROR_CHECK(mdns_hostname_set(settings.hostname));
      ESP_ERROR_CHECK(mdns_instance_name_set(settings.mdns_instance));
      mdns_txt_item_t serviceTxtData[1] = { {"path","/"} };
      ESP_ERROR_CHECK( mdns_service_add(settings.mdns_instance, "_http", "_tcp", 80, serviceTxtData, 1) );
      #endif
      #ifdef CONFIG_CAMERA_ENABLED
      app_camera_httpd_startup();
      #endif
      #ifdef CONFIG_USE_SSD1306_LCD_DRIVER
      app_lcd_startup();
      #endif
      return;
    }
  }
  esp_register_shutdown_handler(&app_shutdown);
}