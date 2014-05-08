#include <pebble.h>

//#define TOTAL_TEXT_LINES 2
  
static Window *window;
//static TextLayer *text_line[TOTAL_TEXT_LINES];
//Inverter Layer
static InverterLayer *inv_layer;
//Bluetooth
static GBitmap *bluetooth_connected_image, *bluetooth_disconnected_image; //Bluetooth images
static BitmapLayer *bluetooth_layer; //Bluetooth layer
//Battery
static uint8_t batteryPercent; //for calculating fill state
static GBitmap *battery_image;
static BitmapLayer *battery_image_layer; //battery icon
static BitmapLayer *battery_fill_layer; //show fill status
//Text Lines
static TextLayer *minuteLayer_2longlines, *minuteLayer_3lines, *minuteLayer_2biglines; // The Minutes
static TextLayer *hourLayer; // The hours

//Set key ID´s
enum {
  KEY_INVERTED = 0,
  KEY_BLUETOOTH = 1,
  KEY_VIBE = 2,
  KEY_BATT_IMG = 3
};

//Default key values
static bool key_indicator_inverted = false; //true = white background
static bool key_indicator_bluetooth = true; //true = bluetooth icon on
static bool key_indicator_vibe = true; //true = vibe on bluetooth disconnect
static bool key_indicator_batt_img = true; //true = show batt usage image

//######## Custom Functions ########

//Battery - set image if charging, or set empty battery image if not charging
void change_battery_icon(bool charging) {
  gbitmap_destroy(battery_image);
  if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  } else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

//Update battery icon or hide it
static void update_battery(BatteryChargeState charge_state) {
  if (key_indicator_batt_img) {
    batteryPercent = charge_state.charge_percent;
    layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), !key_indicator_batt_img);
    if(batteryPercent==100 && key_indicator_batt_img) {
      change_battery_icon(false);
      layer_set_hidden(bitmap_layer_get_layer(battery_fill_layer), false);
    }
    layer_set_hidden(bitmap_layer_get_layer(battery_fill_layer), charge_state.is_charging);
    change_battery_icon(charge_state.is_charging);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(battery_fill_layer), !key_indicator_batt_img);
    layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), !key_indicator_batt_img);
  }
}

//draw the remaining battery percentage
void battery_layer_update_callback(Layer *me, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 5), 0, GCornerNone);
}

static void load_battery_layers() {
  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  GRect battery_frame = (GRect) {
    .origin = { .x = 3, .y = 2 },
    .size = battery_image->bounds.size
  };
  battery_fill_layer = bitmap_layer_create(battery_frame);
  battery_image_layer = bitmap_layer_create(battery_frame);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_set_update_proc(bitmap_layer_get_layer(battery_fill_layer), battery_layer_update_callback);
	
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_fill_layer));
  if (key_indicator_batt_img) {
    battery_state_service_subscribe(&update_battery);
  }
  update_battery(battery_state_service_peek());
}

//Bluetooth
static void toggle_bluetooth_icon(bool connected) { // Toggle bluetooth
  if (connected) {
    bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_connected_image);
  } else {
    bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_disconnected_image);
  }
  if (!connected && key_indicator_vibe) {
    vibes_long_pulse();
  }
}

void bluetooth_connection_callback(bool connected) {  //Bluetooth handler
  toggle_bluetooth_icon(connected);
}

static void load_bluetooth_layers() {
  bluetooth_connected_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_CONNECTED);
  bluetooth_disconnected_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);
  GRect bluetooth_frame = (GRect) {
    .origin = { .x = 129, .y = 2 },
    .size = bluetooth_connected_image->bounds.size
  };
  bluetooth_layer = bitmap_layer_create(bluetooth_frame);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bluetooth_layer));
  if (key_indicator_bluetooth) {
    bluetooth_connection_service_subscribe(bluetooth_connection_callback);
    bluetooth_connection_callback(bluetooth_connection_service_peek());
    layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), false);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), true);
  }
}

//If a Key is changing, do following:
void process_tuple(Tuple *t) {
  switch(t->key) {
    //Inverter Layer
    case KEY_INVERTED: {
      key_indicator_inverted = !strcmp(t->value->cstring,"on"); // easiest way to convert a on/off string into a boolean
      layer_set_hidden(inverter_layer_get_layer(inv_layer), !key_indicator_inverted);
      break;
    }
    case KEY_BLUETOOTH: {
		  key_indicator_bluetooth = !strcmp(t->value->cstring,"on");
      layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !key_indicator_bluetooth);
      if (key_indicator_bluetooth) {
        bluetooth_connection_service_subscribe(bluetooth_connection_callback);
        bluetooth_connection_callback(bluetooth_connection_service_peek());
      } else {
        bluetooth_connection_service_unsubscribe();
      }
      break;
    }
    case KEY_VIBE: {
		  key_indicator_vibe = !strcmp(t->value->cstring,"on");
      break;
    }
    case KEY_BATT_IMG: {
		  key_indicator_batt_img = !strcmp(t->value->cstring,"on");
      update_battery(battery_state_service_peek());
      
      if (key_indicator_batt_img) {
        battery_state_service_subscribe(&update_battery);
      }
      else {
        battery_state_service_unsubscribe();
      }
      break;
    }
  }
}

//If a Key is changing, call process_tuple
void in_received_handler(DictionaryIterator *iter, void *context) {
	for(Tuple *t=dict_read_first(iter); t!=NULL; t=dict_read_next(iter)) process_tuple(t);
}

//Create Inverter Layer
void load_inv_layer() {
  inv_layer = inverter_layer_create((GRect) {.origin = {0, 0}, .size = {144, 168}});
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inv_layer));
  if (key_indicator_inverted) {
    layer_set_hidden(inverter_layer_get_layer(inv_layer), false);
  }
  else {
    layer_set_hidden(inverter_layer_get_layer(inv_layer), true);
  }
}

/*//Load Text Lines
void load_text_lines() {
  for (int i = 1; i < TOTAL_TEXT_LINES; ++i) {
    text_line[i] = text_layer_create((GRect) {.origin = {0, 0}, .size = {0, 0}});
    text_layer_set_text_color(text_line[i], GColorWhite);
    text_layer_set_background_color(text_line[i], GColorClear);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_line[i]));
    //layer_set_hidden(text_layer_get_layer(text_line[i]), true);
  }
}*/

static void load_text_layers() {
  //Load Fonts
  GFont bitham = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  GFont bithamBold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  //ResHandle font_handle_three_lines = resource_get_handle(RESOURCE_ID_FONT_TEST_34);
  ResHandle robotoLight = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_34);
  ResHandle robotoReg = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_REG_34);
    
  // Configure Minute Layers
  minuteLayer_3lines = text_layer_create((GRect) { .origin = {0, 10}, .size = {144, 168-10}});
  text_layer_set_text_color(minuteLayer_3lines, GColorWhite);
  text_layer_set_background_color(minuteLayer_3lines, GColorClear);
  text_layer_set_font(minuteLayer_3lines, fonts_load_custom_font(robotoLight));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minuteLayer_3lines));
  
  minuteLayer_2longlines = text_layer_create((GRect) { .origin = {0, 44}, .size = {144, 168-44}});
  text_layer_set_text_color(minuteLayer_2longlines, GColorWhite);
  text_layer_set_background_color(minuteLayer_2longlines, GColorClear);
  text_layer_set_font(minuteLayer_2longlines, fonts_load_custom_font(robotoLight));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minuteLayer_2longlines));
  
  minuteLayer_2biglines = text_layer_create((GRect) {.origin = {0, 23}, .size = {144, 168-23}});
  text_layer_set_text_color(minuteLayer_2biglines, GColorWhite);
  text_layer_set_background_color(minuteLayer_2biglines, GColorClear);
  text_layer_set_font(minuteLayer_2biglines, bitham);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minuteLayer_2biglines));
  
  // Configure Hour Layer
  hourLayer = text_layer_create((GRect) { .origin = {0, 109}, .size = {144, 168-109}});
  text_layer_set_text_color(hourLayer, GColorWhite);
  text_layer_set_background_color(hourLayer, GColorClear);
  text_layer_set_font(hourLayer, bithamBold);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hourLayer));
}

//Display Time
static void display_time(struct tm *time) {
  //Hour Text
  const char *hour_string[25] = { "zwölf", "eins","zwei", "drei", "vier", "fünf", "sechs",
	  "sieben", "acht", "neun", "zehn", "elf", "zwölf", "eins", "zwei", "drei", "vier",
    "fünf", "sechs", "sieben", "acht", "neun", "zehn", "elf" , "zwölf"};
  //Minute Text
  const char *minute_string[] = {
    "\npunkt", "eins\nnach", "zwei\nnach", "drei\nnach", "vier\nnach", "fünf\nnach",
    "sechs\nnach", "sieben\nnach", "acht\nnach", "neun\nnach", "zehn\nnach",
    "elf\nnach", "zwölf\nnach", "dreizehn nach", "vierzehn nach", "viertel nach",
    "sechzehn nach", "siebzehn nach", "achtzehn nach", "neunzehn nach", "zehn\nvor\nhalb",
    "neun\nvor\nhalb", "acht\nvor\nhalb", "sieben\nvor\nhalb", "sechs\nvor\nhalb", "fünf\nvor\nhalb",
    "vier\nvor\nhalb", "drei\nvor\nhalb", "zwei\nvor\nhalb", "eins\nvor\nhalb", "\nhalb",
    "eins\nnach\nhalb", "zwei\nnach\nhalb", "drei\nnach\nhalb", "vier\nnach\nhalb", "fünf\nnach\nhalb",
    "sechs\nnach\nhalb", "sieben\nnach\nhalb", "acht\nnach\nhalb", "neun\nnach\nhalb", "\nzwanzig vor",
    "neunzehn vor", "achtzehn vor", "siebzehn vor", "sechzehn vor", "drei-\nviertel",
    "vierzehn vor", "dreizehn vor", "zwölf\nvor", "elf\nvor", "zehn\nvor",
    "neun\nvor", "acht\nvor", "sieben\nvor", "sechs\nvor", "fünf\nvor",
    "vier\nvor", "drei\nvor", "zwei\nvor", "eins\nvor"
  };

  //Set Time for DEBUG
  //int hour = 12;
  //int min = 15;

  // Set Time
  int hour = time->tm_hour;
  int min = time->tm_min;
  //DEBUG with second unit
  //int min = time->tm_sec;

  char minute_text[50];
  char hour_text[50];
  
  // Minute Text
  strcpy(minute_text , minute_string[min]);
  layer_set_hidden(text_layer_get_layer(minuteLayer_3lines), true);
  layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), true);
  layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), true);  
  
  if ((20 <= min && min <= 29) ||
      (31 <= min && min <= 40)) {
        layer_set_hidden(text_layer_get_layer(minuteLayer_3lines), false);
  } else if (min == 13 ||
             min == 14 ||
             (16 <= min && min <= 19) ||
             (41 <= min && min <= 44) ||
             min == 46 ||
             min == 47) {
      layer_set_hidden(text_layer_get_layer(minuteLayer_2longlines), false);
  } else if ((0 <= min && min <= 12) ||
             min == 15 ||
             min == 30 ||
             min == 45 ||
             (48 <= min && min <= 60)) {
      layer_set_hidden(text_layer_get_layer(minuteLayer_2biglines), false);
  }
  
  static char staticTimeText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticTimeText , "");
  strcat(staticTimeText , minute_text);
  text_layer_set_text(minuteLayer_3lines, staticTimeText);
  text_layer_set_text(minuteLayer_2longlines, staticTimeText);
  text_layer_set_text(minuteLayer_2biglines, staticTimeText);
  
  // Hour Text
  if (min < 20) {
    strcpy(hour_text , hour_string[hour]);
  } else {
  	strcpy(hour_text , hour_string[hour + 1]);
  }
  
  static char staticHourText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticHourText , "");
  strcat(staticHourText , hour_text);
  text_layer_set_text(hourLayer, staticHourText);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  display_time(tick_time);
}

//######## Generic Functions ########

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  //Key
  app_message_register_inbox_received(in_received_handler); //register key receiving
	app_message_open(512, 512); //Key buffer in- and outbound
  
  //Load value from storage, if storage is empty load default value
  key_indicator_inverted = persist_exists(KEY_INVERTED) ? persist_read_bool(KEY_INVERTED) : key_indicator_inverted;
  key_indicator_bluetooth = persist_exists(KEY_BLUETOOTH) ? persist_read_bool(KEY_BLUETOOTH) : key_indicator_bluetooth;
  key_indicator_vibe = persist_exists(KEY_VIBE) ? persist_read_bool(KEY_VIBE) : key_indicator_vibe;
  key_indicator_batt_img = persist_exists(KEY_BATT_IMG) ? persist_read_bool(KEY_BATT_IMG) : key_indicator_batt_img;
  
  //Load Time and Text lines
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  //load_text_lines();
  load_text_layers();
  load_text_layers();
  display_time(tick_time);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  load_battery_layers();
  load_bluetooth_layers();
  load_inv_layer();
}

static void window_unload(Window *window) {
  /*for (int i = 1; i < TOTAL_TEXT_LINES; ++i) {
    text_layer_destroy(text_line[i]);
  }*/
  inverter_layer_destroy(inv_layer);
  text_layer_destroy(minuteLayer_3lines);
  text_layer_destroy(minuteLayer_2longlines);
  text_layer_destroy(minuteLayer_2biglines);
  text_layer_destroy(hourLayer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true); //Push to Display
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  
  //Bluetooth
  bluetooth_connection_service_unsubscribe();
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_connected_image);
  gbitmap_destroy(bluetooth_disconnected_image);
  
  //Battery
  battery_state_service_unsubscribe();
  layer_remove_from_parent(bitmap_layer_get_layer(battery_fill_layer));
  bitmap_layer_destroy(battery_fill_layer);
  gbitmap_destroy(battery_image);
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);
    
  //Save key´s to persistent storage
  persist_write_bool(KEY_INVERTED, key_indicator_inverted);
  persist_write_bool(KEY_BLUETOOTH, key_indicator_bluetooth);
  persist_write_bool(KEY_VIBE, key_indicator_vibe);
  persist_write_bool(KEY_BATT_IMG, key_indicator_batt_img);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}