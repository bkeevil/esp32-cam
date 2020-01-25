#ifdef CONFIG_SNTP_ENABLED
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "app_settings.h"
#include "app_wifi.h"
#include "esp_sntp.h"


static const char *TAG = "sntp";
extern EventGroupHandle_t event_group;

void time_sync_ notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void app_sntp_startup() {
  time_t now = 0
  struct tm timeinfo = { 0 };
  
  ESP_LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, settings.ntp_server);
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  sntp_init();
  // wait for time to be set
  int retry = 0;
  const int retry_count = 3;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  char strftime_buf[64];
  setenv("TZ", settings.timezone, 1);
  tzset();
  ESP_LOGI(TAG, "Timezone set to %s",settings.timezone);
  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
  return;
}

#endif