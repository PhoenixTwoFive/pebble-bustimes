#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_name_header_layer;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_times_layer;
Layer *window_layer;


static void main_window_load(Window *window) {
  // Get information about the Window
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);

  // Create the TextLayer with specific bounds
  s_name_header_layer = text_layer_create(
      GRect(0, 0, bounds.size.w, 24));

  
  
  
  s_times_layer = text_layer_create(max_text_bounds);

  // Initialize the scroll layer
  s_scroll_layer = scroll_layer_create(
      GRect(0, 24, bounds.size.w, bounds.size.h-24 ));

  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_name_header_layer, GColorDarkGreen);
  text_layer_set_text_color(s_name_header_layer, GColorWhite);
  text_layer_set_text(s_name_header_layer, "Suche...");
  text_layer_set_font(s_name_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_name_header_layer, GTextAlignmentCenter);

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_times_layer, GColorWhite);
  text_layer_set_text_color(s_times_layer, GColorBlack);
  text_layer_set_text(s_times_layer, "");
  text_layer_set_font(s_times_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_times_layer, GTextAlignmentLeft);



  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_name_header_layer));

}


static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_name_header_layer);
  text_layer_destroy(s_times_layer);
  scroll_layer_destroy(s_scroll_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char name_buffer[50];
  static char times_buffer[512];
  // Read tuples for data
  Tuple *name_tuple = dict_find(iterator, MESSAGE_KEY_NAME);
  Tuple *times_tuple = dict_find(iterator, MESSAGE_KEY_TIMES);
  snprintf(name_buffer, sizeof(name_buffer), "%s", name_tuple->value->cstring);
  snprintf(times_buffer, sizeof(times_buffer), "%s", times_tuple->value->cstring);
  text_layer_set_text(s_name_header_layer, name_buffer);
  text_layer_set_text(s_times_layer, times_buffer);
  
  GRect bounds = layer_get_bounds(window_layer);
  GSize max_size = text_layer_get_content_size(s_times_layer);
  text_layer_set_size(s_times_layer, max_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, max_size.h + 4));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_times_layer));
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  // Open AppMessage
  const int inbox_size = 512;
  const int outbox_size = 512;
  app_message_open(inbox_size, outbox_size);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
