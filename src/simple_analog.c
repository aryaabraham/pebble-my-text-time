#include "simple_analog.h"

#include "pebble.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_time_layer; 
static TextLayer *s_time_a_label, *s_time_b_label, *s_time_c_label;

static const char *str_null = "";
static const char *s_time_a_buffer = NULL, *s_time_b_buffer = NULL, *s_time_c_buffer = NULL;

/* A function that prints given number in words */
/* The first string is not used, it is to make array indexing simple */
static const char *single_digits[] = { "o'clock", "one", "two", "three", "four",
                         "five", "six", "seven", "eight", "nine"};
/* The first string is not used, it is to make array indexing simple */
static const char *two_digits_a[] = {"ten", "eleven", "twelve", "thir", "four",
                      "fifteen", "sixteen", "seven", "eigh", "nine"};
static const char *two_digits_b[] = {"", "", "", "teen", "teen",
                      "", "", "teen", "teen", "teen"};
/* The first two string are not used, they are to make array indexing simple*/
static const char *tens_multiple[] = {"", "", "twenty", "thirty", "forty", "fifty",
                         "sixty", "seventy", "eighty", "ninety"};
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

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
}

static void time_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  convert_to_words(&s_time_a_buffer, NULL, (t->tm_hour==0) ? 12 : t->tm_hour) ;
  text_layer_set_text(s_time_a_label, s_time_a_buffer);
  APP_LOG(APP_LOG_LEVEL_INFO, "%s", s_time_a_buffer);

  convert_to_words(&s_time_b_buffer, &s_time_c_buffer, t->tm_min);
  text_layer_set_text(s_time_b_label, s_time_b_buffer);
  text_layer_set_text(s_time_c_label, s_time_c_buffer);
  APP_LOG(APP_LOG_LEVEL_INFO, "%s %s", s_time_b_buffer, s_time_c_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "%d,%d,%d,%d", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
  
  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);

  s_time_layer = layer_create(bounds);
  layer_set_update_proc(s_time_layer, time_update_proc);
  layer_add_child(window_layer, s_time_layer);
  
  //0,0,144,168
  #define H 30
  #define LEFT 10
  #define TOP 20
  s_time_a_label = text_layer_create(GRect(LEFT, TOP, 144, H));
  text_layer_set_text(s_time_a_label, s_time_a_buffer);
  text_layer_set_background_color(s_time_a_label, GColorBlack);
  text_layer_set_text_color(s_time_a_label, GColorWhite);
  text_layer_set_font(s_time_a_label, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_a_label));
  
  s_time_b_label = text_layer_create(GRect(LEFT, TOP+H+5, 144, H));
  text_layer_set_text(s_time_b_label, s_time_b_buffer);
  text_layer_set_background_color(s_time_b_label, GColorBlack);
  text_layer_set_text_color(s_time_b_label, GColorWhite);
  text_layer_set_font(s_time_b_label, fonts_get_system_font(FONT_KEY_GOTHIC_28)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_b_label));
  
  s_time_c_label = text_layer_create(GRect(LEFT, TOP+H+5+H+5, 144, H));
  text_layer_set_text(s_time_c_label, s_time_c_buffer);
  text_layer_set_background_color(s_time_c_label, GColorBlack);
  text_layer_set_text_color(s_time_c_label, GColorWhite);
  text_layer_set_font(s_time_c_label, fonts_get_system_font(FONT_KEY_GOTHIC_28)); 
  layer_add_child(s_time_layer, text_layer_get_layer(s_time_c_label));
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_time_layer);

  text_layer_destroy(s_time_a_label);
  text_layer_destroy(s_time_b_label);
  text_layer_destroy(s_time_c_label);
}

static void init() {
  APP_LOG(APP_LOG_LEVEL_INFO, "init");
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  APP_LOG(APP_LOG_LEVEL_INFO, "main");
  init();
  app_event_loop();
  deinit();
}
