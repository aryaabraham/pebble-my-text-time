#include "simple_analog.h"

#include "pebble.h"

static Window *window;

// Text Time Data
static Layer *s_simple_bg_layer, *s_time_layer; 
static TextLayer *s_time_a_label, *s_time_b_label, *s_time_c_label;
static const char *str_null = "";
static const char *s_time_a_buffer = NULL, *s_time_b_buffer = NULL, *s_time_c_buffer = NULL;
static const char *single_digits[] = { "o'clock", "one", "two", "three", "four",
                         "five", "six", "seven", "eight", "nine"};
static const char *two_digits_a[] = {"ten", "eleven", "twelve", "thir", "four",
                      "fifteen", "sixteen", "seven", "eigh", "nine"};
static const char *two_digits_b[] = {"", "", "", "teen", "teen",
                      "", "", "teen", "teen", "teen"};
static const char *tens_multiple[] = {"", "", "twenty", "thirty", "forty", "fifty",
                         "sixty", "seventy", "eighty", "ninety"};

// Weather Data
static TextLayer *s_temperature_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;
enum WeatherKey {
  KEY_WEATHER_ICON = 0x0,         
  KEY_TEMPERATURE = 0x1      
};
static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, //0
  RESOURCE_ID_IMAGE_CLOUD, //1
  RESOURCE_ID_IMAGE_RAIN, //2
  RESOURCE_ID_IMAGE_SNOW //3
};

// Text Time Code
static void convert_to_words(const char **str_top, const char **str_bot, int num)
{
    int tens = num/10;
    int ones = num%10;
    *str_top = str_null;
    *str_bot = str_null;
   
    /* For single digit number */
    if (tens == 0) {
        *str_top = single_digits[ones];
        return;
    }
  
    if (tens == 1) {
        *str_top = two_digits_a[ones];
        *str_bot = two_digits_b[ones];
        return;     
    }
  
    if (tens > 1) {
        *str_top = tens_multiple[tens];
        if (ones > 0) {
          *str_bot = single_digits[ones];          
        }
        return;         
    }
}

static void time_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  convert_to_words(&s_time_a_buffer, &s_time_c_buffer, ((t->tm_hour%12)==0) ? 12 : t->tm_hour%12) ;
  text_layer_set_text(s_time_a_label, s_time_a_buffer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "%s", s_time_a_buffer);

  convert_to_words(&s_time_b_buffer, &s_time_c_buffer, t->tm_min);
  text_layer_set_text(s_time_b_label, s_time_b_buffer);
  text_layer_set_text(s_time_c_label, s_time_c_buffer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "%s %s", s_time_b_buffer, s_time_c_buffer);
  
  // Get weather update every 30 minutes
  if(t->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}

// Weather 

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      text_layer_set_text(s_temperature_layer, t->value->cstring);
      break;
      
    case KEY_WEATHER_ICON:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }
      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[t->value->uint8]);
#ifdef PBL_SDK_3
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
#endif
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "%d,%d,%d,%d", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
  
  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);

  // Text Time
  s_time_layer = layer_create(bounds);
  layer_set_update_proc(s_time_layer, time_update_proc);
  layer_add_child(window_layer, s_time_layer);
  
  //0,0,144,168
  #define H 46
  #define LEFT 0
  #define TOP 0
  s_time_a_label = text_layer_create(GRect(LEFT, TOP, 144, H));
  text_layer_set_text(s_time_a_label, s_time_a_buffer);
  text_layer_set_background_color(s_time_a_label, GColorBlack);
  text_layer_set_text_color(s_time_a_label, GColorWhite);
  text_layer_set_font(s_time_a_label, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_a_label));
  
  s_time_b_label = text_layer_create(GRect(LEFT, TOP+H, 144, H));
  text_layer_set_text(s_time_b_label, s_time_b_buffer);
  text_layer_set_background_color(s_time_b_label, GColorBlack);
  text_layer_set_text_color(s_time_b_label, GColorWhite);
  text_layer_set_font(s_time_b_label, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_b_label));
  
  s_time_c_label = text_layer_create(GRect(LEFT, TOP+H+H, 144, H));
  text_layer_set_text(s_time_c_label, s_time_c_buffer);
  text_layer_set_background_color(s_time_c_label, GColorBlack);
  text_layer_set_text_color(s_time_c_label, GColorWhite);
  text_layer_set_font(s_time_c_label, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_c_label));
  
  // Weather
  s_icon_layer = bitmap_layer_create(GRect(124, 144, 20, 20));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_temperature_layer = text_layer_create(GRect(80, 142, 44, 20));
  text_layer_set_text_color(s_temperature_layer, COLOR_FALLBACK(GColorBlack, GColorWhite));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_time_layer);

  text_layer_destroy(s_time_a_label);
  text_layer_destroy(s_time_b_label);
  text_layer_destroy(s_time_c_label);
  
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }
  bitmap_layer_destroy(s_icon_layer);
  text_layer_destroy(s_temperature_layer);
}

static void init() {
  //APP_LOG(APP_LOG_LEVEL_INFO, "init");
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  // Weather
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  //APP_LOG(APP_LOG_LEVEL_INFO, "main");
  init();
  app_event_loop();
  deinit();
}
