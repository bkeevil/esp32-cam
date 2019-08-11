#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "fonts/fonts.h"

//#define CONFIG_STORE_FONT_PARTITION

#ifdef CONFIG_STORE_FONT_PARTITION
#include "fonts/FreeMono12pt7b.h"
#include "fonts/FreeMono18pt7b.h"
#include "fonts/FreeMono24pt7b.h"
#include "fonts/FreeMono9pt7b.h"
#include "fonts/FreeMonoBold12pt7b.h"
#include "fonts/FreeMonoBold18pt7b.h"
#include "fonts/FreeMonoBold24pt7b.h"
#include "fonts/FreeMonoBold9pt7b.h"
#include "fonts/FreeMonoBoldOblique12pt7b.h"
#include "fonts/FreeMonoBoldOblique18pt7b.h"
#include "fonts/FreeMonoBoldOblique24pt7b.h"
#include "fonts/FreeMonoBoldOblique9pt7b.h"
#include "fonts/FreeMonoOblique12pt7b.h"
#include "fonts/FreeMonoOblique18pt7b.h"
#include "fonts/FreeMonoOblique24pt7b.h"
#include "fonts/FreeMonoOblique9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans24pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold12pt7b.h"
#include "fonts/FreeSansBold18pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"
#include "fonts/FreeSansBold9pt7b.h"
#include "fonts/FreeSansBoldOblique12pt7b.h"
#include "fonts/FreeSansBoldOblique18pt7b.h"
#include "fonts/FreeSansBoldOblique24pt7b.h"
#include "fonts/FreeSansBoldOblique9pt7b.h"
#include "fonts/FreeSansOblique12pt7b.h"
#include "fonts/FreeSansOblique18pt7b.h"
#include "fonts/FreeSansOblique24pt7b.h"
#include "fonts/FreeSansOblique9pt7b.h"
#include "fonts/FreeSerif12pt7b.h"
#include "fonts/FreeSerif18pt7b.h"
#include "fonts/FreeSerif24pt7b.h"
#include "fonts/FreeSerif9pt7b.h"
#include "fonts/FreeSerifBold12pt7b.h"
#include "fonts/FreeSerifBold18pt7b.h"
#include "fonts/FreeSerifBold24pt7b.h"
#include "fonts/FreeSerifBold9pt7b.h"
#include "fonts/FreeSerifBoldItalic12pt7b.h"
#include "fonts/FreeSerifBoldItalic18pt7b.h"
#include "fonts/FreeSerifBoldItalic24pt7b.h"
#include "fonts/FreeSerifBoldItalic9pt7b.h"
#include "fonts/FreeSerifItalic12pt7b.h"
#include "fonts/FreeSerifItalic18pt7b.h"
#include "fonts/FreeSerifItalic24pt7b.h"
#include "fonts/FreeSerifItalic9pt7b.h"
#include "fonts/Org_01.h"
#include "fonts/Picopixel.h"
#include "fonts/Tiny3x3a2pt7b.h"
#include "fonts/TomThumb.h"
#endif

static const char *TAG = "LCD";

#define STORAGE_PARTITION "fonts"
#define STORAGE_NAMESPACE "fonts"

#ifdef CONFIG_STORE_FONT_PARTITION

static int totalSize = 0;

void write_font_partition_record(nvs_handle_t handle, char *name, const uint8_t *data, const size_t size) {
  ESP_ERROR_CHECK(nvs_set_blob(handle,name,data,size));
  ESP_LOGI(TAG,"Key: %s Size: %d",name,size);
  totalSize += size;
}

void write_font_partition(nvs_handle_t handle) {  
  totalSize = 0;

  write_font_partition_record(handle,"FM9-BM",&FreeMono9pt7bBitmaps,sizeof(FreeMono9pt7bBitmaps));
  write_font_partition_record(handle,"FM9-G",&FreeMono9pt7bGlyphs,sizeof(FreeMono9pt7bGlyphs));
  write_font_partition_record(handle,"FM9",&FreeMono9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM12-BM",&FreeMono12pt7bBitmaps,sizeof(FreeMono12pt7bBitmaps));
  write_font_partition_record(handle,"FM12-G",&FreeMono12pt7bGlyphs,sizeof(FreeMono12pt7bGlyphs));
  write_font_partition_record(handle,"FM12",&FreeMono12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM18-BM",&FreeMono18pt7bBitmaps,sizeof(FreeMono18pt7bBitmaps));
  write_font_partition_record(handle,"FM18-G",&FreeMono18pt7bGlyphs,sizeof(FreeMono18pt7bGlyphs));
  write_font_partition_record(handle,"FM18",&FreeMono18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM24-BM",&FreeMono24pt7bBitmaps,sizeof(FreeMono24pt7bBitmaps));
  write_font_partition_record(handle,"FM24-G",&FreeMono24pt7bGlyphs,sizeof(FreeMono24pt7bGlyphs));
  write_font_partition_record(handle,"FM24",&FreeMono24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));   

  write_font_partition_record(handle,"FM9B-BM",&FreeMonoBold9pt7bBitmaps,sizeof(FreeMonoBold9pt7bBitmaps));
  write_font_partition_record(handle,"FM9B-G",&FreeMonoBold9pt7bGlyphs,sizeof(FreeMonoBold9pt7bGlyphs));
  write_font_partition_record(handle,"FM9B",&FreeMonoBold9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM12B-BM",&FreeMonoBold12pt7bBitmaps,sizeof(FreeMonoBold12pt7bBitmaps));
  write_font_partition_record(handle,"FM12B-G",&FreeMonoBold12pt7bGlyphs,sizeof(FreeMonoBold12pt7bGlyphs));
  write_font_partition_record(handle,"FM12B",&FreeMonoBold12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM18B-BM",&FreeMonoBold18pt7bBitmaps,sizeof(FreeMonoBold18pt7bBitmaps));
  write_font_partition_record(handle,"FM18B-G",&FreeMonoBold18pt7bGlyphs,sizeof(FreeMonoBold18pt7bGlyphs));
  write_font_partition_record(handle,"FM18B",&FreeMonoBold18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM24B-BM",&FreeMonoBold24pt7bBitmaps,sizeof(FreeMonoBold24pt7bBitmaps));
  write_font_partition_record(handle,"FM24B-G",&FreeMonoBold24pt7bGlyphs,sizeof(FreeMonoBold24pt7bGlyphs));
  write_font_partition_record(handle,"FM24B",&FreeMonoBold24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        

  write_font_partition_record(handle,"FM9BO-BM",&FreeMonoBoldOblique9pt7bBitmaps,sizeof(FreeMonoBoldOblique9pt7bBitmaps));
  write_font_partition_record(handle,"FM9BO-G",&FreeMonoBoldOblique9pt7bGlyphs,sizeof(FreeMonoBoldOblique9pt7bGlyphs));
  write_font_partition_record(handle,"FM9BO",&FreeMonoBoldOblique9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM12BO-BM",&FreeMonoBoldOblique12pt7bBitmaps,sizeof(FreeMonoBoldOblique12pt7bBitmaps));
  write_font_partition_record(handle,"FM12BO-G",&FreeMonoBoldOblique12pt7bGlyphs,sizeof(FreeMonoBoldOblique12pt7bGlyphs));
  write_font_partition_record(handle,"FM12BO",&FreeMonoBoldOblique12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM18BO-BM",&FreeMonoBoldOblique18pt7bBitmaps,sizeof(FreeMonoBoldOblique18pt7bBitmaps));
  write_font_partition_record(handle,"FM18BO-G",&FreeMonoBoldOblique18pt7bGlyphs,sizeof(FreeMonoBoldOblique18pt7bGlyphs));
  write_font_partition_record(handle,"FM18BO",&FreeMonoBoldOblique18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM24BO-BM",&FreeMonoBoldOblique24pt7bBitmaps,sizeof(FreeMonoBoldOblique24pt7bBitmaps));
  write_font_partition_record(handle,"FM24BO-G",&FreeMonoBoldOblique24pt7bGlyphs,sizeof(FreeMonoBoldOblique24pt7bGlyphs));
  write_font_partition_record(handle,"FM24BO",&FreeMonoBoldOblique24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        

  write_font_partition_record(handle,"FM9O-BM",&FreeMonoOblique9pt7bBitmaps,sizeof(FreeMonoOblique9pt7bBitmaps));
  write_font_partition_record(handle,"FM9O-G",&FreeMonoOblique9pt7bGlyphs,sizeof(FreeMonoOblique9pt7bGlyphs));
  write_font_partition_record(handle,"FM9O",&FreeMonoOblique9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM12O-BM",&FreeMonoOblique12pt7bBitmaps,sizeof(FreeMonoOblique12pt7bBitmaps));
  write_font_partition_record(handle,"FM12O-G",&FreeMonoOblique12pt7bGlyphs,sizeof(FreeMonoOblique12pt7bGlyphs));
  write_font_partition_record(handle,"FM12O",&FreeMonoOblique12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM18O-BM",&FreeMonoOblique18pt7bBitmaps,sizeof(FreeMonoOblique18pt7bBitmaps));
  write_font_partition_record(handle,"FM18O-G",&FreeMonoOblique18pt7bGlyphs,sizeof(FreeMonoOblique18pt7bGlyphs));
  write_font_partition_record(handle,"FM18O",&FreeMonoOblique18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FM24O-BM",&FreeMonoOblique24pt7bBitmaps,sizeof(FreeMonoOblique24pt7bBitmaps));
  write_font_partition_record(handle,"FM24O-G",&FreeMonoOblique24pt7bGlyphs,sizeof(FreeMonoOblique24pt7bGlyphs));
  write_font_partition_record(handle,"FM24O",&FreeMonoOblique24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        

  write_font_partition_record(handle,"FS99-BM",&FreeSans9pt7bBitmaps,sizeof(FreeSans9pt7bBitmaps));
  write_font_partition_record(handle,"FS99-G",&FreeSans9pt7bGlyphs,sizeof(FreeSans9pt7bGlyphs));
  write_font_partition_record(handle,"FS99",&FreeSans9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS912-BM",&FreeSans12pt7bBitmaps,sizeof(FreeSans12pt7bBitmaps));
  write_font_partition_record(handle,"FS912-G",&FreeSans12pt7bGlyphs,sizeof(FreeSans12pt7bGlyphs));
  write_font_partition_record(handle,"FS912",&FreeSans12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS918-BM",&FreeSans18pt7bBitmaps,sizeof(FreeSans18pt7bBitmaps));
  write_font_partition_record(handle,"FS918-G",&FreeSans18pt7bGlyphs,sizeof(FreeSans18pt7bGlyphs));
  write_font_partition_record(handle,"FS918",&FreeSans18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FS924-BM",&FreeSans24pt7bBitmaps,sizeof(FreeSans24pt7bBitmaps));
  write_font_partition_record(handle,"FS924-G",&FreeSans24pt7bGlyphs,sizeof(FreeSans24pt7bGlyphs));
  write_font_partition_record(handle,"FS924",&FreeSans24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));   

  write_font_partition_record(handle,"FS99B-BM",&FreeSansBold9pt7bBitmaps,sizeof(FreeSansBold9pt7bBitmaps));
  write_font_partition_record(handle,"FS99B-G",&FreeSansBold9pt7bGlyphs,sizeof(FreeSansBold9pt7bGlyphs));
  write_font_partition_record(handle,"FS99B",&FreeSansBold9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS912B-BM",&FreeSansBold12pt7bBitmaps,sizeof(FreeSansBold12pt7bBitmaps));
  write_font_partition_record(handle,"FS912B-G",&FreeSansBold12pt7bGlyphs,sizeof(FreeSansBold12pt7bGlyphs));
  write_font_partition_record(handle,"FS912B",&FreeSansBold12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS918B-BM",&FreeSansBold18pt7bBitmaps,sizeof(FreeSansBold18pt7bBitmaps));
  write_font_partition_record(handle,"FS918B-G",&FreeSansBold18pt7bGlyphs,sizeof(FreeSansBold18pt7bGlyphs));
  write_font_partition_record(handle,"FS918B",&FreeSansBold18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS924B-BM",&FreeSansBold24pt7bBitmaps,sizeof(FreeSansBold24pt7bBitmaps));
  write_font_partition_record(handle,"FS924B-G",&FreeSansBold24pt7bGlyphs,sizeof(FreeSansBold24pt7bGlyphs));
  write_font_partition_record(handle,"FS924B",&FreeSansBold24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        

  write_font_partition_record(handle,"FS99BO-BM",&FreeSansBoldOblique9pt7bBitmaps,sizeof(FreeSansBoldOblique9pt7bBitmaps));
  write_font_partition_record(handle,"FS99BO-G",&FreeSansBoldOblique9pt7bGlyphs,sizeof(FreeSansBoldOblique9pt7bGlyphs));
  write_font_partition_record(handle,"FS99BO",&FreeSansBoldOblique9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS912BO-BM",&FreeSansBoldOblique12pt7bBitmaps,sizeof(FreeSansBoldOblique12pt7bBitmaps));
  write_font_partition_record(handle,"FS912BO-G",&FreeSansBoldOblique12pt7bGlyphs,sizeof(FreeSansBoldOblique12pt7bGlyphs));
  write_font_partition_record(handle,"FS912BO",&FreeSansBoldOblique12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS918BO-BM",&FreeSansBoldOblique18pt7bBitmaps,sizeof(FreeSansBoldOblique18pt7bBitmaps));
  write_font_partition_record(handle,"FS918BO-G",&FreeSansBoldOblique18pt7bGlyphs,sizeof(FreeSansBoldOblique18pt7bGlyphs));
  write_font_partition_record(handle,"FS918BO",&FreeSansBoldOblique18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS924BO-BM",&FreeSansBoldOblique24pt7bBitmaps,sizeof(FreeSansBoldOblique24pt7bBitmaps));
  write_font_partition_record(handle,"FS924BO-G",&FreeSansBoldOblique24pt7bGlyphs,sizeof(FreeSansBoldOblique24pt7bGlyphs));
  write_font_partition_record(handle,"FS924BO",&FreeSansBoldOblique24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        

  write_font_partition_record(handle,"FS99O-BM",&FreeSansOblique9pt7bBitmaps,sizeof(FreeSansOblique9pt7bBitmaps));
  write_font_partition_record(handle,"FS99O-G",&FreeSansOblique9pt7bGlyphs,sizeof(FreeSansOblique9pt7bGlyphs));
  write_font_partition_record(handle,"FS99O",&FreeSansOblique9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS912O-BM",&FreeSansOblique12pt7bBitmaps,sizeof(FreeSansOblique12pt7bBitmaps));
  write_font_partition_record(handle,"FS912O-G",&FreeSansOblique12pt7bGlyphs,sizeof(FreeSansOblique12pt7bGlyphs));
  write_font_partition_record(handle,"FS912O",&FreeSansOblique12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS918O-BM",&FreeSansOblique18pt7bBitmaps,sizeof(FreeSansOblique18pt7bBitmaps));
  write_font_partition_record(handle,"FS918O-G",&FreeSansOblique18pt7bGlyphs,sizeof(FreeSansOblique18pt7bGlyphs));
  write_font_partition_record(handle,"FS918O",&FreeSansOblique18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FS924O-BM",&FreeSansOblique24pt7bBitmaps,sizeof(FreeSansOblique24pt7bBitmaps));
  write_font_partition_record(handle,"FS924O-G",&FreeSansOblique24pt7bGlyphs,sizeof(FreeSansOblique24pt7bGlyphs));
  write_font_partition_record(handle,"FS924O",&FreeSansOblique24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 

  write_font_partition_record(handle,"FR9-BM",&FreeSerif9pt7bBitmaps,sizeof(FreeSerif9pt7bBitmaps));
  write_font_partition_record(handle,"FR9-G",&FreeSerif9pt7bGlyphs,sizeof(FreeSerif9pt7bGlyphs));
  write_font_partition_record(handle,"FR9",&FreeSerif9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FR12-BM",&FreeSerif12pt7bBitmaps,sizeof(FreeSerif12pt7bBitmaps));
  write_font_partition_record(handle,"FR12-G",&FreeSerif12pt7bGlyphs,sizeof(FreeSerif12pt7bGlyphs));
  write_font_partition_record(handle,"FR12",&FreeSerif12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FR18-BM",&FreeSerif18pt7bBitmaps,sizeof(FreeSerif18pt7bBitmaps));
  write_font_partition_record(handle,"FR18-G",&FreeSerif18pt7bGlyphs,sizeof(FreeSerif18pt7bGlyphs));
  write_font_partition_record(handle,"FR18",&FreeSerif18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FR24-BM",&FreeSerif24pt7bBitmaps,sizeof(FreeSerif24pt7bBitmaps));
  write_font_partition_record(handle,"FR24-G",&FreeSerif24pt7bGlyphs,sizeof(FreeSerif24pt7bGlyphs));
  write_font_partition_record(handle,"FR24",&FreeSerif24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));   

  write_font_partition_record(handle,"FR9B-BM",&FreeSerifBold9pt7bBitmaps,sizeof(FreeSerifBold9pt7bBitmaps));
  write_font_partition_record(handle,"FR9B-G",&FreeSerifBold9pt7bGlyphs,sizeof(FreeSerifBold9pt7bGlyphs));
  write_font_partition_record(handle,"FR9B",&FreeSerifBold9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FR12B-BM",&FreeSerifBold12pt7bBitmaps,sizeof(FreeSerifBold12pt7bBitmaps));
  write_font_partition_record(handle,"FR12B-G",&FreeSerifBold12pt7bGlyphs,sizeof(FreeSerifBold12pt7bGlyphs));
  write_font_partition_record(handle,"FR12B",&FreeSerifBold12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR18B-BM",&FreeSerifBold18pt7bBitmaps,sizeof(FreeSerifBold18pt7bBitmaps));
  write_font_partition_record(handle,"FR18B-G",&FreeSerifBold18pt7bGlyphs,sizeof(FreeSerifBold18pt7bGlyphs));
  write_font_partition_record(handle,"FR18B",&FreeSerifBold18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR24B-BM",&FreeSerifBold24pt7bBitmaps,sizeof(FreeSerifBold24pt7bBitmaps));
  write_font_partition_record(handle,"FR24B-G",&FreeSerifBold24pt7bGlyphs,sizeof(FreeSerifBold24pt7bGlyphs));
  write_font_partition_record(handle,"FR24B",&FreeSerifBold24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        
  
  write_font_partition_record(handle,"FR9BO-BM",&FreeSerifBoldItalic9pt7bBitmaps,sizeof(FreeSerifBoldItalic9pt7bBitmaps));
  write_font_partition_record(handle,"FR9BO-G",&FreeSerifBoldItalic9pt7bGlyphs,sizeof(FreeSerifBoldItalic9pt7bGlyphs));
  write_font_partition_record(handle,"FR9BO",&FreeSerifBoldItalic9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));

  write_font_partition_record(handle,"FR12BO-BM",&FreeSerifBoldItalic12pt7bBitmaps,sizeof(FreeSerifBoldItalic12pt7bBitmaps));
  write_font_partition_record(handle,"FR12BO-G",&FreeSerifBoldItalic12pt7bGlyphs,sizeof(FreeSerifBoldItalic12pt7bGlyphs));
  write_font_partition_record(handle,"FR12BO",&FreeSerifBoldItalic12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR18BO-BM",&FreeSerifBoldItalic18pt7bBitmaps,sizeof(FreeSerifBoldItalic18pt7bBitmaps));
  write_font_partition_record(handle,"FR18BO-G",&FreeSerifBoldItalic18pt7bGlyphs,sizeof(FreeSerifBoldItalic18pt7bGlyphs));
  write_font_partition_record(handle,"FR18BO",&FreeSerifBoldItalic18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR24BO-BM",&FreeSerifBoldItalic24pt7bBitmaps,sizeof(FreeSerifBoldItalic24pt7bBitmaps));
  write_font_partition_record(handle,"FR24BO-G",&FreeSerifBoldItalic24pt7bGlyphs,sizeof(FreeSerifBoldItalic24pt7bGlyphs));
  write_font_partition_record(handle,"FR24BO",&FreeSerifBoldItalic24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));        
  
  write_font_partition_record(handle,"FR9O-BM",&FreeSerifItalic9pt7bBitmaps,sizeof(FreeSerifItalic9pt7bBitmaps));
  write_font_partition_record(handle,"FR9O-G",&FreeSerifItalic9pt7bGlyphs,sizeof(FreeSerifItalic9pt7bGlyphs));
  write_font_partition_record(handle,"FR9O",&FreeSerifItalic9pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR12O-BM",&FreeSerifItalic12pt7bBitmaps,sizeof(FreeSerifItalic12pt7bBitmaps));
  write_font_partition_record(handle,"FR12O-G",&FreeSerifItalic12pt7bGlyphs,sizeof(FreeSerifItalic12pt7bGlyphs));
  write_font_partition_record(handle,"FR12O",&FreeSerifItalic12pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR18O-BM",&FreeSerifItalic18pt7bBitmaps,sizeof(FreeSerifItalic18pt7bBitmaps));
  write_font_partition_record(handle,"FR18O-G",&FreeSerifItalic18pt7bGlyphs,sizeof(FreeSerifItalic18pt7bGlyphs));
  write_font_partition_record(handle,"FR18O",&FreeSerifItalic18pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle));
  
  write_font_partition_record(handle,"FR24O-BM",&FreeSerifItalic24pt7bBitmaps,sizeof(FreeSerifItalic24pt7bBitmaps));
  write_font_partition_record(handle,"FR24O-G",&FreeSerifItalic24pt7bGlyphs,sizeof(FreeSerifItalic24pt7bGlyphs));
  write_font_partition_record(handle,"FR24O",&FreeSerifItalic24pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 
  
  write_font_partition_record(handle,"ORG-BM",&Org_01Bitmaps,sizeof(Org_01Bitmaps));
  write_font_partition_record(handle,"ORG-G",&Org_01Glyphs,sizeof(Org_01Glyphs));
  write_font_partition_record(handle,"ORG",&Org_01.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 
  
  write_font_partition_record(handle,"Pico-BM",&PicopixelBitmaps,sizeof(PicopixelBitmaps));
  write_font_partition_record(handle,"Pico-G",&PicopixelGlyphs,sizeof(PicopixelGlyphs));
  write_font_partition_record(handle,"Pico",&Picopixel.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 
  
  write_font_partition_record(handle,"TINY-BM",&Tiny3x3a2pt7bBitmaps,sizeof(Tiny3x3a2pt7bBitmaps));
  write_font_partition_record(handle,"TINY-G",&Tiny3x3a2pt7bGlyphs,sizeof(Tiny3x3a2pt7bGlyphs));
  write_font_partition_record(handle,"TINY",&Tiny3x3a2pt7b.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 
  
  write_font_partition_record(handle,"TOM-BM",&TomThumbBitmaps,sizeof(TomThumbBitmaps));
  write_font_partition_record(handle,"TOM-G",&TomThumbGlyphs,sizeof(TomThumbGlyphs));
  write_font_partition_record(handle,"TOM",&TomThumb.first,3);
  ESP_ERROR_CHECK(nvs_commit(handle)); 
  
  ESP_LOGI(TAG,"Total font data size=%d",totalSize);
}
#endif


void app_fonts_main() {
  nvs_handle_t handle;
  nvs_stats_t stats;
  ESP_ERROR_CHECK(nvs_flash_init_partition(STORAGE_PARTITION));
  #ifdef CONFIG_STORE_FONT_PARTITION
  ESP_ERROR_CHECK(nvs_flash_erase_partition(STORAGE_PARTITION));
  #endif
  ESP_ERROR_CHECK(nvs_open_from_partition(STORAGE_PARTITION,STORAGE_NAMESPACE,NVS_READWRITE,&handle));
  #ifdef CONFIG_STORE_FONT_PARTITION
  write_font_partition(handle);
  #endif
  ESP_ERROR_CHECK(nvs_get_stats(STORAGE_PARTITION,&stats));
  ESP_LOGI(TAG,"Font Table Used Entries: %d",stats.used_entries);
  ESP_LOGI(TAG,"Font Table Free Entries: %d",stats.free_entries);
  ESP_LOGI(TAG,"Font Table Total Entries: %d",stats.total_entries);
  nvs_close(handle);
  ESP_ERROR_CHECK(nvs_flash_deinit_partition("fonts"));
}