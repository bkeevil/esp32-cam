#if !defined(__IDF_VERSION__H__)
#define __IDF_VERSION__H__

#include <esp_idf_version.h>

#if ESP_IDF_VERSION_MAJOR < 4
#error esp-idf version must be 4.x
#endif

#if ESP_IDF_VERSION_MAJOR == 4 && ESP_IDF_VERSION_MINOR < 1
#define __USE_LWIP_IP_4_ADDR
#define __USE_TCP_ADAPTOR
#endif

#endif
