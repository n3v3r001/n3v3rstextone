#include <pebble.h>

static Window *window;
static TextLayer *minuteLayer; // The Minutes
//static TextLayer *beforeAfterLayer; // The before and after text
static TextLayer *hourLayer; // The hours

static void change_text_size() {
  
}

static void init_text_layers(int linecount) {
  (void)linecount;
  int skippLines = 3 - linecount;

  ResHandle minute_font_handle = resource_get_handle(RESOURCE_ID_FONT_TEST_34); //Load custom Font
  
  // Configure Minute Layer
  minuteLayer = text_layer_create((GRect) { .origin = {10, 10 - (skippLines * 21)}, .size = {144-10 /* width */, 168-14 /* height */}});
  text_layer_set_text_color(minuteLayer, GColorWhite);
  text_layer_set_background_color(minuteLayer, GColorClear);
  //GFont bitham = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  //text_layer_set_font(minuteLayer, bitham);
  text_layer_set_font(minuteLayer, fonts_load_custom_font(minute_font_handle));
  
  /*
  // Configure  Half Hour Layer
  beforeAfterLayer = text_layer_create((GRect) {.origin = {10, 26 - (skippLines * 21)}, .size = {144-10, 168-14}});
  text_layer_set_text_color(beforeAfterLayer, GColorWhite);
  text_layer_set_background_color(beforeAfterLayer, GColorClear);
  text_layer_set_font(beforeAfterLayer, fonts_load_custom_font(minute_font_handle));
  */
  
  // Configure Hour Layer
  hourLayer = text_layer_create((GRect) { .origin = {10, 110 - (skippLines * 21)}, .size = {144-10 /* width */, 168-14 /* height */}});
  text_layer_set_text_color(hourLayer, GColorWhite);
  text_layer_set_background_color(hourLayer, GColorClear);
  GFont bithamBold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  text_layer_set_font(hourLayer, bithamBold);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  init_text_layers(3);
  layer_add_child(window_layer, text_layer_get_layer(minuteLayer));
  // layer_add_child(window_layer, text_layer_get_layer(beforeAfterLayer));
  layer_add_child(window_layer, text_layer_get_layer(hourLayer));
}

static void window_unload(Window *window) {
  text_layer_destroy(minuteLayer);
  text_layer_destroy(hourLayer);
  //text_layer_destroy(beforeAfterLayer);
}

static void display_time(struct tm *time) {

const char *hour_string[25] = { "zwölf", "eins","zwei", "drei", "vier", "fünf", "sechs",
		 "sieben", "acht", "neun", "zehn", "elf", "zwölf", "eins", "zwei", "drei", "vier",
		 "fünf", "sechs", "sieben", "acht", "neun", "zehn", "elf" , "zwölf"};
  
//Minutenweise probleme ab dreizehn
const char *minute_string[] = {
    "\npunkt", "\neins \nnach", "\nzwei \nnach", "\ndrei \nnach", "\nvier \nnach", "\nfünf \nnach",
    "\nsechs \nnach", "\nsieben \nnach", "\nacht \nnach", "\nneun \nnach", "\nzehn \nnach",
  "\nelf \nnach", "\nzwölf \nnach", "dreizehn nach", "vierzehn nach", "viertel nach",
  "sechzehn nach", "siebzehn nach", "achtzehn nach", "neunzehn nach", "zehn vor halb",
  "neun vor halb", "acht vor halb", "sieben vor halb", "sechs vor halb", "fünf vor halb",
  "vier vor halb", "drei vor halb", "zwei vor halb", "eins vor halb", "\nhalb",
  "eins nach halb", "zwei nach halb", "drei nach halb", "vier nach halb", "fünf nach halb",
  "sechs nach halb", "sieben nach halb", "acht nach halb", "neun nach halb", "zwanzig vor",
  "elf nach halb", "zwölf nach halb", "dreizehn nach halb", "vierzehn nach halb", "dreiviertel",
  "vierzehn \nvor", "dreizehn \nvor", "zwölf \nvor", "elf \nvor", "zehn \nvor",
  "neun \nvor", "acht \nvor", "sieben \nvor", "sechs \nvor", "fünf \nvor",
  "vier \nvor", "drei \nvor", "zwei \nvor", "eins \nvor"
  };

  
  /* Debug
  int hour = time->tm_hour;
  int min = time->tm_min;
  */
  int hour = 11;
  int min = 4;  
  
  char minute_text[50];
  // char halfhour_text[50];
  char hour_text[50];
  
  // Hour Text
  if (min < 20) {
    strcpy(hour_text , hour_string[hour]);
  } else {
  	strcpy(hour_text , hour_string[hour + 1]);
  }
  
  // Minute Text
  if (0 <= min && min <= 60) {
    strcpy(minute_text , minute_string[min]);
  }
  
  
/*
  if (linecount > 4) {
	  init_text_layers(linecount);
  }
*/
  
  static char staticTimeText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticTimeText , "");
  strcat(staticTimeText , minute_text);
  text_layer_set_text(minuteLayer, staticTimeText);

  /*
  static char staticBeforeAfterText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticBeforeAfterText , "");
  strcat(staticBeforeAfterText , halfhour_text);
  text_layer_set_text(beforeAfterLayer, staticBeforeAfterText);
  */
  
  static char staticHourText[50] = ""; // Needs to be static because it's used by the system later.
  strcpy(staticHourText , "");
  strcat(staticHourText , hour_text);
  text_layer_set_text(hourLayer, staticHourText);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  change_text_size();
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
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
  window_destroy(window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
