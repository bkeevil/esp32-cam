#include "scut.h"

static const char TAG[] = "scut";
esp_err_t scut (char * dst, char * src, size_t dst_size) {
    //clears destination, so its full of null-termination characters
    //truncate src string to (dst_size - 1) characters by copying (dst_size - 1) characters from src to dst
    //then there is no need to add null term char because we cleared 1st and copied 1 char less than dst_size
    memset(dst, '\0', dst_size);    //clear dst with nullterms
    
    if (dst_size < 2) {
        //thats an error
        ESP_LOGI(TAG, "[strtrunc] dst size < 2 makes no sense");
        return ESP_FAIL; 
    }
    
    memcpy(dst, src, dst_size - 1);
    return ESP_OK;
}
