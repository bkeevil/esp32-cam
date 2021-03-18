#include "app_httpd.h"
#include "mbedtls/base64.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
//#include "img_converters.h"
//#include "fb_gfx.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "app_settings.h"
#include "scut.h"
#ifdef CONFIG_SNTP_ENABLED
#include <time.h>
#include <sys/time.h>
#include "app_sntp.h"
#endif
#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
#include "app_illuminator.h"
//#include <ctype.h>
#endif

static const char* TAG = "camera_httpd";

float avg_fps = 0;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// static const char* HTTP_AUTH_STRING = "Basic YnVnZXI6d2FubmFTRUU=";	
//buger:wannaSEE - echo -n "buger:wannaSEE" | openssl enc -base64
static const char* HTTP_AUTH_STRING_PREFIX = "Basic ";
static const char* HTTP_UNAUTH_RESP = "Unauthorized!";
static const char* HTTP_AUTH_HDR = "Authorization";
static const char* HTTP_401 = "401 Authorization Required";
static const char* HTTP_REQ_AUTH_HDR = "WWW-Authenticate";
static const char* HTTP_REQ_AUTH_REALM = "Basic realm=\"Secure Area\"";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

extern EventGroupHandle_t event_group;

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
static int led_duty = 0;
#endif

bool isStreaming = false;
char * http_auth_b64 = NULL;

typedef struct {
        size_t size;  //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

static ra_filter_t ra_filter;

static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int *)malloc(sample_size * sizeof(int));
    if(!filter->values){
        return NULL;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

static int ra_filter_run(ra_filter_t * filter, int value){
    if(!filter->values){
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len) {
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

//static esp_err_t send_crossorigin_hdr (httpd_req_t *req) {
//    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "600");
//    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET");
//    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
//    return ESP_OK;  //improve cheking
//}


static esp_err_t auth_check(httpd_req_t *req) {
	//returns ESP_OK is auth OK, ESP_FAIL if not
	char*  auth_buf;
    size_t auth_buf_len;

	//check if auth is on
	if (http_auth_b64 == NULL) {
		//auth is off => auth_check passed :-)
		return ESP_OK;
	}

	//check if there is auth header present
	auth_buf_len = httpd_req_get_hdr_value_len(req, HTTP_AUTH_HDR) + 1; //+1 for null term at the end of the string

    if (auth_buf_len == (strlen(http_auth_b64) + 1)) {
		//yes, there is auth header and have same length as our HTTP_AUTH_STRING, lets compare them
        auth_buf = malloc(auth_buf_len); 
        /* Copy null terminated value string into buffer - the auth string */
        if (httpd_req_get_hdr_value_str(req, HTTP_AUTH_HDR, auth_buf, auth_buf_len) == ESP_OK) {
            //ESP_LOGI(TAG, "Found header => Authorization: %s", auth_buf);
			if (strcmp (auth_buf, http_auth_b64) == 0) {
				//auth OK!
				ESP_LOGI(TAG, "Auth OK!");
				free(auth_buf);
				return ESP_OK;
			}
        }
        free(auth_buf);
    } 

	//no header present or bad login
	return ESP_FAIL;
}

static esp_err_t auth_req(httpd_req_t *req) {
	//ruquest the auth if failed
	    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
		esp_err_t res = httpd_resp_set_status(req, HTTP_401);
		if(res == ESP_OK) res = httpd_resp_set_hdr(req, HTTP_REQ_AUTH_HDR, HTTP_REQ_AUTH_REALM);
		//send_crossorigin_hdr (req); //temporary
		if(res == ESP_OK) res = httpd_resp_send(req, HTTP_UNAUTH_RESP, strlen(HTTP_UNAUTH_RESP));
		if(res == ESP_OK) {
			ESP_LOGI(TAG, "Auth request sent!");
		} else {
			ESP_LOGI(TAG, "Auth request failed!");
		}
		return res;
}

static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    
    int64_t fr_start = esp_timer_get_time();
    

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    app_illuminator_set_led_intensity(led_duty);
    vTaskDelay(150 / portTICK_PERIOD_MS); // The LED requires ~150ms to "warm up"
#endif
    fb = esp_camera_fb_get();
#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    app_illuminator_set_led_intensity(0);
#endif
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t fb_len = 0;
    if(fb->format == PIXFORMAT_JPEG){
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %uB %ums", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
    return res;

}

static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    //check auth
//	if (auth_check(req) != ESP_OK) {
//		//not authorized, ask auth and quit
//		return auth_req(req);
//	}
	//do this when auth OK

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    

    isStreaming = true;

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    app_illuminator_set_led_intensity(led_duty);
#endif

    //fps limiter init
    TickType_t xFrequency = 1000 / (portTICK_PERIOD_MS);    //initialize with 1fps
	TickType_t xLastWakeTime = 0;   // = xTaskGetTickCount ();;
	uint8_t prevfps = 0;   // settings.fps;

    while(true){

        fb = esp_camera_fb_get();       //get frame
        
        //fps limiter
		if (settings.fps != 0) {		//fps = 0 means limiter off
			if (prevfps != settings.fps) {
				//fps changed
				prevfps = settings.fps;
				xLastWakeTime = xTaskGetTickCount ();	//reinit to take effect immediately
				xFrequency = 1000 / (portTICK_PERIOD_MS * settings.fps);
				ESP_LOGI(TAG, "FPS changed");
			}
			vTaskDelayUntil( &xLastWakeTime, xFrequency );      //actually wait
		}     
        
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
                if(fb->format != PIXFORMAT_JPEG){
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if(!jpeg_converted){
                        ESP_LOGE(TAG, "JPEG compression failed");
                        res = ESP_FAIL;
                    }
                } else {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();

        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        avg_fps = 1000.0 / avg_frame_time;
        ESP_LOGI(TAG, "MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)"
            ,(uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, avg_fps
        );
    }

    avg_fps = 0;
    isStreaming = false;

#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    app_illuminator_set_led_intensity(0);
#endif

    last_frame = 0;
    return res;
}

static void urldecode2(char *dst, const char *src)      //https://stackoverflow.com/questions/2673207/c-c-url-decode-library/2766963
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}


static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[sizeof(settings.wifi_password) * 3];  //sizeof(settings.wifi_password) is the biggest one, *3 because special characters take 3 chard like " " = %20
    memset(variable, 0, sizeof(variable));
    memset(value, 0, sizeof(value));
    
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    urldecode2 (value, value);  //we need to decode (%20 to space etc) https://www.w3schools.com/jsref/jsref_encodeuricomponent.asp 

    int val = atoi(value);
    ESP_LOGI(TAG, "%s = %s (%d)", variable, value, val);
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;

    
    if(!strcmp(variable, "framesize")) {
      if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
    }
    else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);
    else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);
    else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);
    else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, val);
    else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);
    else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, val);
    else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, val);
    else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, val);
    else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, val);
    else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);
    else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);
    else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, val);
    else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, val);
    else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, val);
    else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, val);
    else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, val);
    else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, val);
    else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, val);
    else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, val);
    else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, val);
    else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, val);
    else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, val);
    else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, val);
    #ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    else if(!strcmp(variable, "led_intensity")) { led_duty = val; if (isStreaming) app_illuminator_set_led_intensity(led_duty); }
    #endif
    else if(!strcmp(variable, "hostname")) scut(settings.hostname, value, sizeof(settings.hostname));      //strncpy(settings.hostname, value, sizeof(settings.hostname));
    else if(!strcmp(variable, "wifi_ssid")) scut(settings.wifi_ssid, value, sizeof(settings.wifi_ssid));
    else if(!strcmp(variable, "wifi_password")) scut(settings.wifi_password, value, sizeof(settings.wifi_password));
    #ifdef CONFIG_MDNS_ENABLED
    else if(!strcmp(variable, "mdns_instance")) scut(settings.mdns_instance, value, sizeof(settings.mdns_instance));
    #endif
    else if(!strcmp(variable, "dhcp")) settings.dhcp = val;
    #ifdef CONFIG_SNTP_ENABLED
    else if(!strcmp(variable, "ntp_server")) scut(settings.ntp_server, value, sizeof(settings.ntp_server));
    else if(!strcmp(variable, "timezone")) { scut(settings.timezone, value, sizeof(settings.timezone)); setenv("TZ", settings.timezone, 1); tzset(); }
    #endif
    else if(!strcmp(variable, "ip")) settings.ip.addr = ipaddr_addr(value);
    else if(!strcmp(variable, "netmask")) settings.netmask.addr = ipaddr_addr(value);
    else if(!strcmp(variable, "gateway")) settings.gateway.addr = ipaddr_addr(value);
    else if(!strcmp(variable, "dns1")) settings.dns1.addr = ipaddr_addr(value);
    else if(!strcmp(variable, "dns2")) settings.dns2.addr = ipaddr_addr(value);
    else if(!strcmp(variable, "fps")) settings.fps = val;
    else if(!strcmp(variable, "http_user")) scut(settings.http_user, value, sizeof(settings.http_user));
	else if(!strcmp(variable, "http_password")) scut(settings.http_password, value, sizeof(settings.http_user));
	else if(!strcmp(variable, "http_auth")) settings.http_auth = val;
    else {
      res = -1;
    }

    if(res){
      return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t store_handler(httpd_req_t *req) {
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    app_settings_save();
    esp_camera_save_to_nvs("camera");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t reboot_handler(httpd_req_t *req) {
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    esp_err_t ret = httpd_resp_send(req, NULL, 0);
    vTaskDelay(250 / portTICK_PERIOD_MS); // Short delay to ensure the http response is sent
    esp_restart();
    return ret;
}

static esp_err_t reset_handler(httpd_req_t *req) {
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    app_settings_reset();
    esp_camera_load_from_nvs("camera");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t status_handler(httpd_req_t *req){
	//check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
	
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    p+=sprintf(p, "\"hostname\":\"%s\",", settings.hostname);
    p+=sprintf(p, "\"wifi_ssid\":\"%s\",", settings.wifi_ssid);
    //p+=sprintf(p, "\"wifi_password\":\"%s\",", settings.wifi_password);
    #ifdef CONFIG_MDNS_ENABLED
    p+=sprintf(p, "\"mdns_instance\":\"%s\",", settings.mdns_instance);
    #endif
    #ifdef CONFIG_SNTP_ENABLED
    p+=sprintf(p, "\"ntp_server\":\"%s\",", settings.ntp_server);
    p+=sprintf(p, "\"timezone\":\"%s\",", settings.timezone);
    #endif
    p+=sprintf(p, "\"dhcp\":%u,", settings.dhcp);
    p+=sprintf(p, "\"ip\":\"" IPSTR "\",", IP2STR(&settings.ip));
    p+=sprintf(p, "\"netmask\":\"" IPSTR "\",", IP2STR(&settings.netmask));
    p+=sprintf(p, "\"gateway\":\"" IPSTR "\",", IP2STR(&settings.gateway));
    p+=sprintf(p, "\"dns1\":\"" IPSTR "\",", IP2STR(&settings.dns1));
    p+=sprintf(p, "\"dns2\":\"" IPSTR "\",", IP2STR(&settings.dns2));
    p+=sprintf(p, "\"fps\":%u,", settings.fps);
    p+=sprintf(p, "\"http_auth\":%u,", settings.http_auth);
	p+=sprintf(p, "\"http_user\":\"%s\",", settings.http_user);
	p+=sprintf(p, "\"http_password\":\"%s\"", settings.http_password);
#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    p+= sprintf(p, ",\"led_intensity\":%u", led_duty);
    p+= sprintf(p, ",\"led_max_intensity\":%u", CONFIG_LED_MAX_INTENSITY);
#else
    p+= sprintf(p, ",\"led_intensity\":%d", -1);
#endif
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t stylesheet_handler(httpd_req_t *req){
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK

    extern const unsigned char style_css_gz_start[] asm("_binary_style_css_gz_start");
    extern const unsigned char style_css_gz_end[]   asm("_binary_style_css_gz_end");
    size_t style_css_gz_len = style_css_gz_end - style_css_gz_start;

    httpd_resp_set_type(req, "text/css");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)style_css_gz_start, style_css_gz_len);
}

static esp_err_t script_handler(httpd_req_t *req){
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    extern const unsigned char script_js_gz_start[] asm("_binary_script_js_gz_start");
    extern const unsigned char script_js_gz_end[]   asm("_binary_script_js_gz_end");
    size_t script_js_gz_len = script_js_gz_end - script_js_gz_start;

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)script_js_gz_start, script_js_gz_len);
}

static esp_err_t index_handler(httpd_req_t *req){
    //check auth
	if (auth_check(req) != ESP_OK) {
		//not authorized, ask auth and quit
		return auth_req(req);
	}

	//do this when auth OK
    extern const unsigned char index_html_gz_start[] asm("_binary_index_html_gz_start");
    extern const unsigned char index_html_gz_end[]   asm("_binary_index_html_gz_end");
    size_t index_html_gz_len = index_html_gz_end - index_html_gz_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)index_html_gz_start, index_html_gz_len);
}

void app_httpd_startup(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = 10;
    
    if (settings.http_auth) {
		//auth turned on
		char buf[sizeof(settings.http_user) + sizeof(settings.http_password) + 2];
		memset(buf, 0, sizeof(buf));
		strcpy(buf, settings.http_user);
		strcat(buf, ":");
		strcat(buf, settings.http_password);

		ESP_LOGI(TAG, " creds=%s", buf);

		#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */

		size_t b64_len = 0;
		int err = 0;
		//check how big buffer we need for base64 encoded username:password
		mbedtls_base64_encode( NULL, 0, &b64_len, (unsigned char*) buf, strlen(buf));
		if (b64_len != BASE64_SIZE_T_MAX) {
			//yes we know the length
			ESP_LOGI(TAG, " ok, we need %u bytes to encode creds", b64_len);
			char* b64_buf;
			b64_buf = malloc(b64_len);
			err = mbedtls_base64_encode((unsigned char*)b64_buf, b64_len, &b64_len, (unsigned char*) buf, strlen(buf));
			if (err == 0) {
				//conversion to base64 OK
				//now add the prefix
				http_auth_b64 = malloc(b64_len + strlen(HTTP_AUTH_STRING_PREFIX));
				strncpy(http_auth_b64, HTTP_AUTH_STRING_PREFIX, b64_len + strlen(HTTP_AUTH_STRING_PREFIX));
				strncat(http_auth_b64, b64_buf, b64_len);
				ESP_LOGI(TAG, " http_auth_b64=%s", http_auth_b64);
			} else {
				//conversion to base64 failed
				ESP_LOGI(TAG, " b64 encoder err - conversion failed");
				if (err == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
					ESP_LOGI(TAG, " b64 encoder err - buffer too small");
				} else if (err == MBEDTLS_ERR_BASE64_INVALID_CHARACTER) {
					ESP_LOGI(TAG, " b64 encoder err - invalid character");
				} else {
					ESP_LOGI(TAG, " b64 encoder err - unknown");
				}
			}
			free(b64_buf);
		} else {
			ESP_LOGI(TAG, " b64 encoder err - cant get buffer length");
		}
	} else {
		//auth is off
		http_auth_b64 = NULL;
	}

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t stylesheet_uri = {
        .uri       = "/style.css",
        .method    = HTTP_GET,
        .handler   = stylesheet_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t script_uri = {
        .uri       = "/script.js",
        .method    = HTTP_GET,
        .handler   = script_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t status_uri = {
        .uri       = "/status",
        .method    = HTTP_GET,
        .handler   = status_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t cmd_uri = {
        .uri       = "/control",
        .method    = HTTP_GET,
        .handler   = cmd_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t store_uri = {
        .uri       = "/store",
        .method    = HTTP_GET,
        .handler   = store_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t capture_uri = {
        .uri       = "/capture",
        .method    = HTTP_GET,
        .handler   = capture_handler,
        .user_ctx  = NULL
    };


   char surl[sizeof(settings.http_password) + 1 + 8];    // +8 for "/stream-"; 
   memset(surl, 0, sizeof(surl));
   strcpy(surl, "/stream");
   if (settings.http_auth) {
        //add -password to the stream url if auth enabled
        strcat(surl, "-");
        strcat(surl, settings.http_password);
   }
   
   httpd_uri_t stream_uri = {
        .uri       = surl,
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

   httpd_uri_t reboot_uri = {
        .uri       = "/reboot",
        .method    = HTTP_GET,
        .handler   = reboot_handler,
        .user_ctx  = NULL
    };

   httpd_uri_t reset_uri = {
        .uri       = "/reset",
        .method    = HTTP_GET,
        .handler   = reset_handler,
        .user_ctx  = NULL
    };

    ra_filter_init(&ra_filter, 20);
    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);
    ESP_ERROR_CHECK(httpd_start(&camera_httpd, &config));
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &store_uri);
    httpd_register_uri_handler(camera_httpd, &status_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &reboot_uri);
    httpd_register_uri_handler(camera_httpd, &reset_uri);
    httpd_register_uri_handler(camera_httpd, &stylesheet_uri);
    httpd_register_uri_handler(camera_httpd, &script_uri);

    config.server_port += 1;
    config.ctrl_port += 1;
    ESP_LOGI(TAG, "Starting stream server on port: '%d'", config.server_port);
    ESP_ERROR_CHECK(httpd_start(&stream_httpd, &config));
    ESP_ERROR_CHECK(httpd_register_uri_handler(stream_httpd, &stream_uri));
}

void app_httpd_shutdown() {
  if (stream_httpd) httpd_stop(stream_httpd);
  if (camera_httpd) httpd_stop(camera_httpd);
}
