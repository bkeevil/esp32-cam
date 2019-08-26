#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "app_settings.h"
#include "app_wifi.h"
#include "app_camera.h"
#include "app_httpd.h"
#include "app_lcd.h"
#ifdef CONFIG_MDNS_ENABLED
#include "app_mdns.h"
#endif

EventGroupHandle_t event_group; // Shared event group

void app_shutdown() {
  app_settings_shutdown();
  #ifdef CONFIG_USE_SSD1306_LCD_DRIVER
  app_lcd_shutdown();
  #endif
  #ifdef CONFIG_LED_ILLUMINATOR_ENABLED
  app_illuminator_shutdown();
  #endif
  #ifdef CONFIG_MDNS_ENABLED
  if (settings.mdns) {
    app_mdns_shutdown();
  }
  #endif
  app_httpd_shutdown();
  app_wifi_shutdown();
  app_camera_shutdown();
}

void app_main()
{
  ESP_ERROR_CHECK(esp_event_loop_create_default());   
  event_group = xEventGroupCreate();
  
  app_settings_startup(); 
  app_settings_reset();
  app_settings_save();    
  app_camera_startup();
  app_wifi_startup(event_group);
  app_httpd_startup(event_group);
  #ifdef CONFIG_MDNS_ENABLED
  if (settings.mdns) {
    app_mdns_startup(event_group);
  }
  #endif
  #ifdef CONFIG_USE_SSD1306_LCD_DRIVER
  app_lcd_startup(event_group);
  #endif
  #ifdef CONFIG_LED_ILLUMINATOR_ENABLED
  app_illuminator_startup();
  #endif
  esp_register_shutdown_handler(&app_shutdown);
}