#include "scut.h"

static const char TAG[] = "scut";
esp_err_t scut (char * dst, char * src, size_t n) {
    //sizeof (dst) must be at least n
    //clears n bytes in destination using null-termination character
    //truncate src string to (n - 1) characters by copying (n- 1) characters from src to dst
    //then there is no need to add null term char because we cleared n and copied 1 char less than n
    memset(dst, '\0', n);    //clear n bytes with nullterms
    
    if (n < 2) {
        //thats an error
        ESP_LOGI(TAG, "dst size < 2 makes no sense");
        return ESP_FAIL; 
    }
    
    memcpy(dst, src, n - 1);
    return ESP_OK;
}
