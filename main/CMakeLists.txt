set(COMPONENT_SRCS 
    "app_main.c" 
    "app_settings.c" 
    "app_illuminator.c"
    "app_camera.c" 
    "app_wifi.c"  
    "app_fonts.c" 
    "app_httpd.c" 
    "app_lcd.c" 
    "app_sntp.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "include" "fonts")

set(COMPONENT_REQUIRES
    esp32-camera
    esp-idf-ssd1306
    mdns
    nvs_flash
    esp_http_server
)

set(COMPONENT_EMBED_FILES
    "www/index.html.gz"
    "www/style.css.gz"
    "www/script.js.gz"
)

register_component()
