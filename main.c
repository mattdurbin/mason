#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_battery_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_time_font, s_date_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  // Copy date into buffer from tm structure
static char date_buffer[16];
strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

// Show the date
text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% Charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_mason);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 65), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorRed);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
   
  // Create date TextLayer
s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(125, 140), bounds.size.w, 50));
text_layer_set_text_color(s_date_layer, PBL_IF_ROUND_ELSE(GColorRed, GColorBlack));
text_layer_set_background_color(s_date_layer, GColorClear);
text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Create GFont
   s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  s_battery_layer = text_layer_create(GRect(0, 0, bounds.size.w, 25));
  text_layer_set_text_color(s_battery_layer, GColorBlue);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100% Charged");

  battery_state_service_subscribe(handle_battery);
  
   layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
    handle_battery(battery_state_service_peek());
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
