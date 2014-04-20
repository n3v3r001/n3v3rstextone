#include <pebble.h>

static Window *window;
static TextLayer *minuteLayer_2longlines, *minuteLayer_3longlines, *minuteLayer_2biglines; // The Minutes
static TextLayer *hourLayer; // The hours

static GBitmap *bluetooth_connected_image, *bluetooth_disconnected_image; //Bluetooth images
static BitmapLayer *bluetooth_layer; //Bluetooth layer

static void toggle_bluetooth_icon(bool connected) { // Toggle bluetooth
  if(!connected) {
    bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_disconnected_image);
    vibes_long_pulse();
  }
  else {
    bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_connected_image);
  }
}

void bluetooth_connection_callback(bool connected) {  //Bluetooth handler
  toggle_bluetooth_icon(connected);
}

static void init_bluetooth_layers() {
  bluetooth_connected_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_CONNECTED);
  bluetooth_disconnected_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);
  GRect bluetooth_frame = (GRect) {
    .origin = { .x = 129, .y = 2 },
    .size = bluetooth_connected_image->bounds.size
  };
  bluetooth_layer = bitmap_layer_create(bluetooth_frame);
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bluetooth_layer));
}

static void init_text_layers() {
  
  //Load Fonts
  GFont bitham = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  GFont bithamBold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  ResHandle font_handle_three_lines = resource_get_handle(RESOURCE_ID_FONT_TEST_34);
  //GFont font_handle_three_lines = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  
  // Configure Minute Layers
  minuteLayer_3longlines = text_layer_create((GRect) { .origin = {5, 8}, .size = {134, 154}});
  text_layer_set_text_color(minuteLayer_3longlines, GColorWhite);
  text_layer_set_background_color(minuteLayer_3longlines, GColorClear);
  text_layer_set_font(minuteLayer_3longlines, fonts_load_custom_font(font_handle_three_lines));
  layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
  
  minuteLayer_2longlines = text_layer_create((GRect) { .origin = {5, 39}, .size = {134, 154}});
  text_layer_set_text_color(minuteLayer_2longlines, GColorWhite);
  text_layer_set_background_color(minuteLayer_2longlines, GColorClear);
  text_layer_set_font(minuteLayer_2longlines, fonts_load_custom_font(font_handle_three_lines));
  layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
  
  minuteLayer_2biglines = text_layer_create((GRect) {.origin = {5, 21}, .size = {134, 154}});
  text_layer_set_text_color(minuteLayer_2biglines, GColorWhite);
  text_layer_set_background_color(minuteLayer_2biglines, GColorClear);
  text_layer_set_font(minuteLayer_2biglines, bitham);
  layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  
  // Configure Hour Layer
  hourLayer = text_layer_create((GRect) { .origin = {5, 107}, .size = {134, 154}});
  text_layer_set_text_color(hourLayer, GColorWhite);
  text_layer_set_background_color(hourLayer, GColorClear);
  text_layer_set_font(hourLayer, bithamBold);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  init_text_layers();
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer_3longlines));
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer_2longlines));
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer_2biglines));
  layer_add_child(window_layer, text_layer_get_layer(hourLayer));
}

static void window_unload(Window *window) {
  text_layer_destroy(minuteLayer_3longlines);
  text_layer_destroy(minuteLayer_2longlines);
  text_layer_destroy(minuteLayer_2biglines);
  text_layer_destroy(hourLayer);
}

static void display_time(struct tm *time) {

  const char *hour_string[25] = { "zwölf", "eins","zwei", "drei", "vier", "fünf", "sechs",
	  "sieben", "acht", "neun", "zehn", "elf", "zwölf", "eins", "zwei", "drei", "vier",
    "fünf", "sechs", "sieben", "acht", "neun", "zehn", "elf" , "zwölf"};
  
  //Minutenweise probleme ab dreizehn
  const char *minute_string[] = {
    "\npunkt", "eins \nnach", "zwei \nnach", "drei \nnach", "vier \nnach", "fünf \nnach",
    "sechs \nnach", "sieben \nnach", "acht \nnach", "neun \nnach", "zehn \nnach",
    "elf \nnach", "zwölf \nnach", "dreizehn nach", "vierzehn nach", "viertel nach",
    "sech-\nzehn nach", "sieb-\nzehn nach", "acht-\nzehn nach", "neun-\nzehn nach", "zehn vor halb",
    "neun vor halb", "acht vor halb", "sieben vor halb", "sechs vor halb", "fünf vor halb",
    "vier vor halb", "drei vor halb", "zwei vor halb", "eins vor halb", "\nhalb",
    "eins nach halb", "zwei nach halb", "drei nach halb", "vier\nnach halb", "fünf nach halb",
    "sechs nach halb", "sieben nach halb", "acht nach halb", "neun nach halb", "zwanzig vor",
    "elf nach halb", "zwölf nach halb", "dreizehn nach halb", "vierzehn nach halb", "\ndrei-\nviertel",
    "vierzehn \nvor", "dreizehn \nvor", "zwölf \nvor", "elf \nvor", "zehn \nvor",
    "neun \nvor", "acht \nvor", "sieben \nvor", "sechs \nvor", "fünf \nvor",
    "vier \nvor", "drei \nvor", "zwei \nvor", "eins \nvor"
  };

  // Set Time for DEBUG
  //int hour = 8;
  //int min = 37;
  
  // Set Time
  int hour = time->tm_hour;
  int min = time->tm_min;
  
  
  char minute_text[50];
  char hour_text[50];
  
  // Hour Text
  if (min < 20) {
    strcpy(hour_text , hour_string[hour]);
  } else {
  	strcpy(hour_text , hour_string[hour + 1]);
  }
  
  // Minute Text
  if (0 <= min && min <= 12) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), false);
  }
  if (13 <= min && min <= 14) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (min == 15) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), false);
  }
  if (16 <= min && min <= 19) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (20 <= min && min <= 29) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (min == 30) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), false);
    //TODO: beides hoch rücken
  }
  if (31 <= min && min <= 39) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (min == 40) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (41 <= min && min <= 44) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (min == 45) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (46 <= min && min <= 47) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), false);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);
  }
  if (48 <= min && min <= 60) {
    strcpy(minute_text , minute_string[min]);
    layer_set_hidden(text_layer_get_layer(minuteLayer_3longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
    layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), false);
  }
  
  static char staticTimeText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticTimeText , "");
  strcat(staticTimeText , minute_text);
  text_layer_set_text(minuteLayer_3longlines, staticTimeText);
  text_layer_set_text(minuteLayer_2longlines, staticTimeText);
  text_layer_set_text(minuteLayer_2biglines, staticTimeText);
  
  static char staticHourText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticHourText , "");
  strcat(staticHourText , hour_text);
  text_layer_set_text(hourLayer, staticHourText);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_time(tick_time);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);
  
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  display_time(tick_time);
  init_bluetooth_layers();
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
}

static void deinit(void) {
  bluetooth_connection_service_unsubscribe();
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_connected_image);
  gbitmap_destroy(bluetooth_disconnected_image);
  
  window_destroy(window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}