#ifndef _APP_MDNS_H_
#define _APP_MDNS_H_

#ifdef __cplusplus
extern "C" {
#endif

void app_mdns_startup(EventGroupHandle_t event_group);
void app_mdns_shutdown();

#ifdef __cplusplus
}
#endif

#endif