#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "app_settings.h"
#include "app_wifi.h"
#include "app_httpd.h"
#include "lwip/ip4_addr.h"
#include "ssd1306.h"

/*#define OLED_ADDRESS 0x3c
#define I2C_SDA 14
#define I2C_SCL 13*/

static const char *TAG = "LCD";

static TaskHandle_t xUpdateLCDTaskHandle = NULL;

SSD1306_t lcd;

char lineChar[24];

void app_update_lcd_task (void * pvParameters) {
	int rssi;

	for (;;) {
		ssd1306_clear_screen(&lcd, false);
		ssd1306_contrast(&lcd, 0xff);
		ip4addr_ntoa_r(&settings.ip,lineChar,24);
		ssd1306_display_text(&lcd,0, lineChar, strlen(lineChar), false);
		ssd1306_display_text(&lcd,1, settings.wifi_ssid, strlen(settings.wifi_ssid), false);
		rssi = 2 * (wifi_get_rssi() + 100);
		sprintf(lineChar,"RSSI: %d%%",rssi);
		ssd1306_display_text(&lcd, 2, lineChar, strlen(lineChar), false);		 
		sprintf(lineChar,"FPS: %.1f",avg_fps);
		ssd1306_display_text(&lcd,3, lineChar, strlen(lineChar), false);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void app_lcd_startup() {
  #if CONFIG_I2C_INTERFACE
	ESP_LOGI(TAG, "INTERFACE is i2c");
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
  #if CONFIG_SSD1306_128x64
	ESP_LOGI(TAG, "Panel is 128x64");
	i2c_init(&lcd, 128, 64, 0x3C);
  #endif // CONFIG_SSD1306_128x64
  #if CONFIG_SSD1306_128x32
	ESP_LOGI(TAG, "Panel is 128x32");
	i2c_init(&lcd, 128, 32, 0x3C);
  #endif // CONFIG_SSD1306_128x32
  #endif // CONFIG_I2C_INTERFACE

  #if CONFIG_SPI_INTERFACE
	ESP_LOGI(TAG, "INTERFACE is SPI");
	ESP_LOGI(TAG, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
	ESP_LOGI(TAG, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	spi_master_init(&lcd, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
	spi_init(&lcd, 128, 64);
  #endif // CONFIG_SPI_INTERFACE

	ssd1306_clear_screen(&lcd, false);
	ssd1306_contrast(&lcd, 0xff);
  
  xTaskCreate(app_update_lcd_task, "LCD", 2048, NULL, tskIDLE_PRIORITY, &xUpdateLCDTaskHandle ); 
}

void app_lcd_shutdown() {
	ssd1306_clear_screen(&lcd, false);
	ssd1306_contrast(&lcd, 0xff);	
	vTaskDelete(xUpdateLCDTaskHandle);
}