
#ifndef _APP_HTTPD_H_
#define _APP_HTTPD_H_

#ifdef __cplusplus
extern "C" {
#endif

extern float avg_fps;

void app_httpd_startup();
void app_httpd_shutdown();

#ifdef __cplusplus
}
#endif

#endif