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
#include "seniverse.h"

static const char *TAG = "app_weather";
extern EventGroupHandle_t event_group;
TaskHandle_t get_current_weather_task_handler;
TaskHandle_t get_daily_weather_task_handler;
TaskHandle_t get_hourly_weather_task_handler;

void get_current_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *now = creat_weather_data(SENIVERSE_WEATHER_NOW, 1);
    int count = 0;
    int ret = 0;

    ret = seniverse_get_weather(SENIVERSE_WEATHER_NOW, "ip", now, &count, 0, 0);
    if (!ret && count > 0)
        dump_weather_now(&now->now);
    destroy_weather_data(now);
    vTaskDelete(NULL);
}

void get_daily_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *daily = creat_weather_data(SENIVERSE_WEATHER_DAILY, 8);
    int count = 0;
    int ret = 0;

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    seniverse_get_weather(SENIVERSE_WEATHER_DAILY, "ip", daily, &count, -1, 8);
    if (!ret && count > 0)
        dump_weather_daily(&daily->daily);
    destroy_weather_data(daily);
    vTaskDelete(NULL);
}

void get_hourly_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *hourly = creat_weather_data(SENIVERSE_WEATHER_HOURLY, 12);
    int count = 0;
    int ret = 0;

    vTaskDelay(20000 / portTICK_PERIOD_MS);
    seniverse_get_weather(SENIVERSE_WEATHER_HOURLY, "ip", hourly, &count, 0, 12);
    if (!ret && count > 0)
        dump_weather_hourly(&hourly->hourly);
    destroy_weather_data(hourly);
    vTaskDelete(NULL);
}

void app_weather_startup()
{
    ESP_LOGI(TAG, "Initializing Weather Task.");
    xTaskCreate(&get_current_weather_task, "get_current_weather_task", 1024 * 20, NULL, 5, &get_current_weather_task_handler);
    xTaskCreate(&get_daily_weather_task, "get_daily_weather_task", 1024 * 20, NULL, 5, &get_daily_weather_task_handler);
    xTaskCreate(&get_hourly_weather_task, "get_hourly_weather_task", 1024 * 20, NULL, 5, &get_hourly_weather_task_handler);

    return;
}
