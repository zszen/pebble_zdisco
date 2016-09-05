#include <pebble.h>
#include "font.h"
#include "ZApi.h"

#define DEBUG_FASTER false

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define SQUARE_SIZE 6 //4
#define HOUR_SPACING 1
#define HOUR_THICKNESS 2//3
#define MINUTE_RADIUS 75
#define MINUTE_SIZE 3//5
#define PIXEL_SIZE 1000 //2500
// #define HOUR_ROW_START 11
// #define HOUR_COL_MIDDLE 15

static int NUMBER_HEIGHT_SQUARE = NUMBER_HEIGHT*HOUR_THICKNESS;
static int NUMBER_WIDTH_SQUARE = NUMBER_WIDTH * HOUR_THICKNESS;
static int HOUR_SQUARE_SIZE = HOUR_THICKNESS * SQUARE_SIZE;

static int MINUTE_SQUARE_SIZE = MINUTE_SIZE * SQUARE_SIZE;
static int MINUTE_SQUARE_SIZEINHALF = SQUARE_SIZE * (MINUTE_SIZE / 2);

static GSize cWinSize;
static GSize cSquareCounts;
static uint cPixelSizeReal;
static GSize cHourSquareCounts;

typedef enum Theme_Type{
	Theme_Type_White,
	Theme_Type_Black,
	Theme_Type_PureWhiteBackground,
	Theme_Type_PureBlackBackground,
	Theme_Type_WhiteBlue,
	Theme_Type_Custom,
}Theme_Type;

static int cTheme = Theme_Type_White; 
static int cBgColor = 0x000000;
static int cHourColor = 0xffffff;
static int cMinuteColor = 0xffffff;
static int cGridColor = 0x000000;


static uint8_t random_colors[PIXEL_SIZE]; 
static uint8_t random_colors2[PIXEL_SIZE];

static Window * main_window;
static Layer * main_layer;

typedef enum CONFIG_SAVEKEY{
	CONFIG_SAVEKEY_Theme,
	CONFIG_SAVEKEY_ColorBg,
	CONFIG_SAVEKEY_ColorHour,
	CONFIG_SAVEKEY_ColorMinute,
	CONFIG_SAVEKEY_ColorGrid,
	CONFIG_SAVEKEY_TOTAL,
}CONFIG_SAVEKEY;

static bool savePool[CONFIG_SAVEKEY_TOTAL];

static void save_config(){
	SaveInt(CONFIG_SAVEKEY_Theme, cTheme);
	SaveInt(CONFIG_SAVEKEY_ColorBg, cBgColor);
	SaveInt(CONFIG_SAVEKEY_ColorHour, cHourColor);
	SaveInt(CONFIG_SAVEKEY_ColorMinute, cMinuteColor);
	SaveInt(CONFIG_SAVEKEY_ColorGrid, cGridColor);
}

static void load_config(){
	LoadInt(CONFIG_SAVEKEY_Theme, Theme_Type_White, cTheme);
	LoadInt(CONFIG_SAVEKEY_ColorBg, 0x000000, cBgColor);
	LoadInt(CONFIG_SAVEKEY_ColorHour, 0xffffff, cHourColor);
	LoadInt(CONFIG_SAVEKEY_ColorMinute, 0xffffff, cMinuteColor);
	LoadInt(CONFIG_SAVEKEY_ColorGrid, 0x000000, cGridColor);
}

static void update_proc(Layer * layer, GContext * ctx) {
	
  graphics_context_set_antialiased(ctx, false);
  
  time_t now = time(NULL);
  struct tm * now_tm = localtime(&now);
  
  // background
	switch(cTheme){
		case Theme_Type_Custom:
			graphics_context_set_fill_color(ctx, GColorFromHEX(cBgColor));
			graphics_fill_rect(ctx, GRect(0, 0, cWinSize.w, cWinSize.h), 0, GCornerNone);
		break;
		case Theme_Type_WhiteBlue:
			graphics_context_set_fill_color(ctx, GColorDukeBlue);
			graphics_fill_rect(ctx, GRect(0, 0, cWinSize.w, cWinSize.h), 0, GCornerNone);
		break;
		case Theme_Type_PureWhiteBackground:
			graphics_context_set_fill_color(ctx, GColorWhite);
			graphics_fill_rect(ctx, GRect(0, 0, cWinSize.w, cWinSize.h), 0, GCornerNone);
		break;
		case Theme_Type_PureBlackBackground:
			graphics_context_set_fill_color(ctx, GColorBlack);
			graphics_fill_rect(ctx, GRect(0, 0, cWinSize.w, cWinSize.h), 0, GCornerNone);
		break;
		case Theme_Type_Black:
		case Theme_Type_White:
		default:
			for(unsigned int i = 0; i < cPixelSizeReal; ++i){
				random_colors[i] = 0b11000000 + (rand() & 0x3F);
				random_colors2[i] = 0b11000000 + (rand() & 0x3F);
			}
			for(int r = 0; r < cWinSize.h; ++r){
				for(int c = 0; c < cWinSize.w; ++c){
					uint8_t r_color_index = r / SQUARE_SIZE;
					uint8_t c_color_index = c / SQUARE_SIZE;
					uint8_t color_index = r_color_index * cSquareCounts.w + c_color_index;
					uint8_t current_color = random_colors[color_index];
					if((r + c) % 2 == 0){
						current_color = random_colors2[color_index];
					}
					graphics_context_set_stroke_color(ctx, (GColor)current_color);
					graphics_draw_pixel(ctx, GPoint(c, r));
				}
			}
		break;
	}
  
	switch(cTheme){
		case Theme_Type_Custom:
 		 	graphics_context_set_fill_color(ctx, GColorFromHEX(cHourColor));
		break;
		case Theme_Type_Black:
		case Theme_Type_PureWhiteBackground:
 		 	graphics_context_set_fill_color(ctx, GColorBlack);
		break;
		case Theme_Type_White:
		case Theme_Type_PureBlackBackground:
		case Theme_Type_WhiteBlue:
		default:
	  		graphics_context_set_fill_color(ctx, GColorWhite);
		break;
	}
  
  // hour
  int hour = now_tm->tm_hour;
  if(!clock_is_24h_style()){
    if(hour >= 13){
      hour -= 12;
    }
    else{
      if(hour < 1){
        hour += 12;
      }
    }
  }
  
  int num_hour_digits = hour >= 10 ? 2 : 1;
  int total_number_width = NUMBER_WIDTHS[hour / 10] + NUMBER_WIDTHS[hour % 10];
  int hour_col_start = num_hour_digits == 2 ? cHourSquareCounts.w - (total_number_width * HOUR_THICKNESS + HOUR_SPACING) / 2 : cHourSquareCounts.w - NUMBER_WIDTHS[hour] * HOUR_THICKNESS / 2;
    
  for(int r = cHourSquareCounts.h; r < cHourSquareCounts.h + NUMBER_HEIGHT_SQUARE; r += HOUR_THICKNESS){
    int hour_col = hour_col_start;
    for(int i = 0; i < num_hour_digits; ++i){
      int digit = num_hour_digits > 1 ? (i == 0 ? hour / 10 : hour % 10) : hour;
      
      for(int c = hour_col; c < hour_col + NUMBER_WIDTH_SQUARE; c += HOUR_THICKNESS){
        int number_r = (r - cHourSquareCounts.h) / HOUR_THICKNESS;
        int number_c = (c - hour_col) / HOUR_THICKNESS;
        if(NUMBERS[digit][number_r][number_c]){
          graphics_fill_rect(ctx, GRect(c * SQUARE_SIZE, r * SQUARE_SIZE, HOUR_SQUARE_SIZE, HOUR_SQUARE_SIZE), 0, GCornerNone);
        }
      }
      hour_col += (NUMBER_WIDTHS[digit] * HOUR_THICKNESS) + HOUR_SPACING;
    }
  }

  // minute
#if DEBUG_FASTER
  int minute = now_tm->tm_sec;
#else
	int minute = now_tm->tm_min;
#endif
	switch(cTheme){
		case Theme_Type_Custom:
			graphics_context_set_fill_color(ctx, GColorFromHEX(cMinuteColor));
		break;
		default:break;
	}
	
  int32_t minute_angle = TRIG_MAX_ANGLE * minute / 60 - TRIG_MAX_ANGLE / 4;
  int minute_center_start_x = ((cWinSize.w / 2) + cos_lookup(minute_angle) * MINUTE_RADIUS / TRIG_MAX_RATIO)  - MINUTE_SQUARE_SIZEINHALF;
  int minute_center_start_y = ((cWinSize.h / 2) + sin_lookup(minute_angle) * MINUTE_RADIUS / TRIG_MAX_RATIO)  - MINUTE_SQUARE_SIZEINHALF;
  
  graphics_fill_rect(ctx, GRect(MAX(0,MIN(cWinSize.w-MINUTE_SQUARE_SIZE,minute_center_start_x)), MAX(0,MIN(cWinSize.h-MINUTE_SQUARE_SIZE,minute_center_start_y)), MINUTE_SQUARE_SIZE, MINUTE_SQUARE_SIZE), 0, GCornerNone);
  

	
	
  // grid
	switch(cTheme){
		case Theme_Type_Custom:
 		 	graphics_context_set_stroke_color(ctx, GColorFromHEX(cGridColor));
		break;
		case Theme_Type_WhiteBlue:
	  		graphics_context_set_stroke_color(ctx, GColorLightGray);
		break;
		case Theme_Type_Black:
		case Theme_Type_PureWhiteBackground:
	  		graphics_context_set_stroke_color(ctx, GColorLightGray);
		break;
		case Theme_Type_PureBlackBackground:
	  		graphics_context_set_stroke_color(ctx, GColorDarkGray);
		break;
		case Theme_Type_White:
		default:
 		 	graphics_context_set_stroke_color(ctx, GColorBlack);
		break;
	}
  for(int r = 0; r < cSquareCounts.h; ++r){
    graphics_draw_line(ctx, GPoint(0, r * SQUARE_SIZE), GPoint(cWinSize.w, r * SQUARE_SIZE));
  }
  for(int c = 0; c < cSquareCounts.w; ++c){
    graphics_draw_line(ctx, GPoint(c * SQUARE_SIZE, 0), GPoint(c * SQUARE_SIZE, cWinSize.h));
  }	
}


static void tick_handler(struct tm * tick_time, TimeUnits units_changed) {
  layer_mark_dirty(main_layer);
}

static void main_window_load(Window * window) {
  Layer * window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  main_layer = layer_create(bounds);
	
	
	cWinSize = bounds.size;
	cSquareCounts = GSize(cWinSize.w/SQUARE_SIZE, cWinSize.h/SQUARE_SIZE);
	cPixelSizeReal = cSquareCounts.w*cSquareCounts.h;
	cHourSquareCounts = GSize(cSquareCounts.w/2,(cSquareCounts.h-NUMBER_HEIGHT_SQUARE)/2);
	
  layer_set_update_proc(main_layer, update_proc);
  layer_add_child(window_layer, main_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	Tuple *t_type = dict_find(iterator, MESSAGE_KEY_TYPE);
	if(!t_type){
		return;
	}
	const char* strType = t_type->value->cstring;
	if(!strcmp(strType,"config")){
		Tuple *t_data = dict_find(iterator, MESSAGE_KEY_CONFIG_theme);
		if(t_data){
			APP_LOG(APP_LOG_LEVEL_INFO, "CONFIG_theme %s",t_data->value->cstring);
			savePool[CONFIG_SAVEKEY_Theme] = true;
			cTheme = ZAPI_atoi(t_data->value->cstring);
		}
		t_data = dict_find(iterator, MESSAGE_KEY_CONFIG_bgColor);
		if(t_data){
			APP_LOG(APP_LOG_LEVEL_INFO, "CONFIG_bgColor %ld",t_data->value->int32);
			savePool[CONFIG_SAVEKEY_ColorBg] = true;
			cBgColor = t_data->value->int32;
		}
		t_data = dict_find(iterator, MESSAGE_KEY_CONFIG_hourColor);
		if(t_data){
			APP_LOG(APP_LOG_LEVEL_INFO, "CONFIG_hourColor %ld",t_data->value->int32 );
			savePool[CONFIG_SAVEKEY_ColorHour] = true;
			cHourColor = t_data->value->int32 ;
		}
		t_data = dict_find(iterator, MESSAGE_KEY_CONFIG_minuteColor);
		if(t_data){
			APP_LOG(APP_LOG_LEVEL_INFO, "CONFIG_minuteColor %ld",t_data->value->int32);
			savePool[CONFIG_SAVEKEY_ColorMinute] = true;
			cMinuteColor = t_data->value->int32;
		}
		t_data = dict_find(iterator, MESSAGE_KEY_CONFIG_gridColor);
		if(t_data){
			APP_LOG(APP_LOG_LEVEL_INFO, "CONFIG_gridColor %ld",t_data->value->int32);
			savePool[CONFIG_SAVEKEY_ColorGrid] = true;
			cGridColor = t_data->value->int32;
		}
		save_config();
		layer_mark_dirty(main_layer);
	}
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


static void main_window_unload(Window * window) {
  layer_destroy(main_layer);
}

static void init() {
  srand(time(NULL));
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);
#if DEBUG_FASTER
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
#else
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
#endif
	
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	
	load_config();
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}