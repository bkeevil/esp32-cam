#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_CONNECTED_BIT  BIT0  // Event group bit is set high while connected
#define WIFI_SOFTAP_BIT     BIT1

int wifi_connection_count();
int wifi_get_rssi();
float wifi_get_tx_power();

void app_wifi_startup();
void app_wifi_shutdown();

#ifdef __cplusplus
}
#endif

#endif