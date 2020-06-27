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


static int dump_weather_location(const struct weather_location *location)
{
    ESP_LOGV(TAG, "location:");
    ESP_LOGV(TAG, "\t id: %s", location->id);
    ESP_LOGV(TAG, "\t name: %s", location->name);
    ESP_LOGV(TAG, "\t country: %s", location->country);
    ESP_LOGV(TAG, "\t path: %s", location->path);
    ESP_LOGV(TAG, "\t timezone: %s", location->timezone);
    ESP_LOGV(TAG, "\t timezone_offset: %s", location->timezone_offset);
    return 0;
}

static int dump_weather_now(const struct weather_now *now)
{
    dump_weather_location(&(now->common.location));
    ESP_LOGV(TAG, "weather now: %s", now->common.last_update);
    ESP_LOGV(TAG, "\t summary: %s", now->items->summary);
    ESP_LOGV(TAG, "\t wind_direction: %s", now->items->wind_direction);
    ESP_LOGV(TAG, "\t temperature: %f", now->items->temperature);
    ESP_LOGV(TAG, "\t feels_like: %f", now->items->feels_like);
    ESP_LOGV(TAG, "\t pressure: %f", now->items->pressure);
    ESP_LOGV(TAG, "\t visibility: %f", now->items->visibility);
    ESP_LOGV(TAG, "\t wind_speed: %f", now->items->wind_speed);
    ESP_LOGV(TAG, "\t wind_scale: %f", now->items->wind_scale);
    ESP_LOGV(TAG, "\t dew_point: %f", now->items->dew_point);
    ESP_LOGV(TAG, "\t code: %d", now->items->code);
    ESP_LOGV(TAG, "\t wind_direction_degree: %f", now->items->wind_direction_degree);
    ESP_LOGV(TAG, "\t clouds: %f", now->items->clouds);
    ESP_LOGV(TAG, "\t humidity: %f", now->items->humidity);
    return 0;
}


static int dump_weather_daily_item(const struct weather_daily_item *daily_item)
{
    ESP_LOGV(TAG, "\t date: %s", daily_item->date);
    ESP_LOGV(TAG, "\t text_day: %s", daily_item->text_day);
    ESP_LOGV(TAG, "\t code_day: %d", daily_item->code_day);
    ESP_LOGV(TAG, "\t text_night: %s", daily_item->text_night);
    ESP_LOGV(TAG, "\t code_night: %d", daily_item->code_night);

    ESP_LOGV(TAG, "\t wind_direction: %s", daily_item->wind_direction);
    ESP_LOGV(TAG, "\t wind_direction_degree: %f", daily_item->wind_direction_degree);
    ESP_LOGV(TAG, "\t wind_speed: %f", daily_item->wind_speed);
    ESP_LOGV(TAG, "\t wind_scale: %f", daily_item->wind_scale);
    ESP_LOGV(TAG, "\t high_temp: %f", daily_item->high_temp);
    ESP_LOGV(TAG, "\t low_temp: %f", daily_item->low_temp);
    ESP_LOGV(TAG, "\t precip: %f", daily_item->precip);
    ESP_LOGV(TAG, "\t rainfall: %f", daily_item->rainfall);
    ESP_LOGV(TAG, "\t humidity: %f", daily_item->humidity);
    return 0;
}

static int dump_weather_daily(const struct weather_daily *daily)
{
    ESP_LOGV(TAG, "Weather daily: %s", daily->common.last_update);
    for (int index = 0; index < daily->common.count; index++) {
        ESP_LOGV(TAG, "index %d ===========================", index);
        dump_weather_daily_item(&daily->items[index]);
    }
    return 0;
}


static int dump_weather_hourly_item(const struct weather_hourly_item *hourly_item)
{
    ESP_LOGV(TAG, "\t time: %s", hourly_item->time);
    ESP_LOGV(TAG, "\t text: %s", hourly_item->text);
    ESP_LOGV(TAG, "\t code: %d", hourly_item->code);
    ESP_LOGV(TAG, "\t wind_direction: %s", hourly_item->wind_direction);
    ESP_LOGV(TAG, "\t wind_speed: %f", hourly_item->wind_speed);
    ESP_LOGV(TAG, "\t temperature: %f", hourly_item->temperature);
    ESP_LOGV(TAG, "\t humidity: %f", hourly_item->humidity);
    return 0;
}

static int dump_weather_hourly(const struct weather_hourly *hourly)
{
    ESP_LOGV(TAG, "Weather hourly: %s", hourly->common.last_update);
    for (int index = 0; index < hourly->common.count; index++) {
        ESP_LOGV(TAG, "index %d ===========================", index);
        dump_weather_hourly_item(&hourly->items[index]);
    }
    return 0;
}

void get_current_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *now = creat_weather_data(SENIVERSE_WEATHER_NOW, 1);
    int count = 0;

    vTaskDelay(10000 / portTICK_PERIOD_MS);
    seniverse_get_weather(SENIVERSE_WEATHER_NOW, "ip",now, &count, 0, 0);
    dump_weather_now(&now->now);
    destroy_weather_data(now);
    vTaskDelete(NULL);
}

void get_daily_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *daily = creat_weather_data(SENIVERSE_WEATHER_DAILY, 7);
    int count = 0;

    vTaskDelay(20000 / portTICK_PERIOD_MS);
    seniverse_get_weather(SENIVERSE_WEATHER_DAILY, "ip", daily, &count, 0, 7);
    dump_weather_daily(&daily->daily);
    destroy_weather_data(daily);
    vTaskDelete(NULL);
}

void get_hourly_weather_task(void* pvParameters)
{
    struct seniverse_weather_obj *hourly = creat_weather_data(SENIVERSE_WEATHER_HOURLY, 12);
    int count = 0;

    vTaskDelay(30000 / portTICK_PERIOD_MS);
    seniverse_get_weather(SENIVERSE_WEATHER_HOURLY, "ip", hourly, &count, -1, 11);
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
