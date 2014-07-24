#include <pebble.h>
#include "elements.h"
void bt_handler(bool connected);
void battery_handler(BatteryChargeState batt_charge);

static TextLayer* text_layer_init(GRect location, GColor background, GTextAlignment alignment, int font)
{
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, GColorWhite);
	text_layer_set_background_color(layer, background);
	text_layer_set_text_alignment(layer, alignment);
	if(font == 1){
		text_layer_set_font(layer, impact);
	}
	else if(font == 2){
		text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	}
	else if(font == 3){
		text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	}
	else if(font == 5){
		text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	}
	return layer;
}

void refresh_settings(){
	layer_set_hidden(inverter_layer_get_layer(theme), settings.theme);
	layer_set_hidden(text_layer_get_layer(actual_minute), settings.showactualmin);
	layer_set_hidden(bitmap_layer_get_layer(bt_icon_layer), settings.bticonhide);
	layer_mark_dirty(battery_lines_layer);
	layer_mark_dirty(bluetooth_circle_layer);
	
	if(settings.covernumbers == 0){
		layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(highlighter));
		layer_set_hidden(text_layer_get_layer(min_des_1), false);
		layer_set_hidden(text_layer_get_layer(min_des_2), false);
		layer_set_hidden(text_layer_get_layer(min_des_3), false);
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_1));
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_2));
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_3));
		layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(theme));
	}
}

void stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
     
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
     
    AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
     
    animation_schedule((Animation*) anim);
}

void glance_this(const char *glancetext, bool vibrate, int vibrateNum, int animationLength, int fullNotify){
			if(vibrate == true){
				if(vibrateNum == 1){
					vibes_short_pulse();
		        }
				else if(vibrateNum == 2){
					vibes_double_pulse();
		        }
				else if(vibrateNum == 3){
					vibes_long_pulse();
		        }
			}
			snprintf(glance_buffer, sizeof(glance_buffer), "%s", glancetext);
			text_layer_set_text(update_at_a_glance, glance_buffer);
				GRect start01 = GRect(0, 300, 144, 168);
				GRect finish02 = GRect(0, 300, 144, 168);
				if(fullNotify == 2){
					finish01 = GRect(0, 73, 144, 168);
					start02 = GRect(0, 73, 144, 168);
				}
				else if(fullNotify == 1){
					finish01 = GRect(0, 0, 144, 168);
					start02 = GRect(0, 0, 144, 168);
		        }
				else if(fullNotify == 0){
					finish01 = GRect(0, 145, 144, 168);
					start02 = GRect(0, 145, 144, 168);
		        }
				animate_layer(text_layer_get_layer(update_at_a_glance), &start01, &finish01, 1000, 0);
				animate_layer(text_layer_get_layer(update_at_a_glance), &start02, &finish02, 1000, animationLength);
}

void process_tuple(Tuple *t)
{
	int key = t->key;
	int value = t->value->int32;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded key num: %d with value %d", key, value);
  switch (key) {
	  case HIDEBTICON_KEY:
	  	settings.bticonhide = value;
	  	break;
	  case THEME_KEY:
	  	settings.theme = value;
	  	break;
	  case COVERTEXT_KEY:
	  	settings.covernumbers = value;
	  	break;
	  case BTDISALERT_KEY:
	 	settings.btdisalert = value;
	  	break;
	  case BTREALERT_KEY:
	  	settings.btrealert = value;
	  	glance_this("Settings updated.", 1, 2, 5000, 0);
	  	refresh_settings();
	  	break;
	  case BATTERYBARSTYLE_KEY:
	  	settings.batterybarstyle = value;
	  	break;
	  case SHOWMINUTE_KEY:
	  	settings.showactualmin = value;
	  	break;
	  case WATCHAPPVER_KEY:
	  	if(versionChecked == 0){
				if(value > currentAppVer){
					APP_LOG(APP_LOG_LEVEL_WARNING, "Watchapp version outdated");
					glance_this("Watchface version out of date! Unload watchface and load again from the appstore or MyPebbleFaces to update. The new version contains new features and bug fixes.", 1, 3, 15000, 1);
				}
				else if(value == currentAppVer){
					APP_LOG(APP_LOG_LEVEL_INFO, "Watchapp version the same as API");
				}
				else if(value < currentAppVer){
					APP_LOG(APP_LOG_LEVEL_INFO, "Watchapp version ahead of API! You must be an eleet 1337 hax0r.");
					glance_this("Hello beta tester :)", 0, 0, 4000, 0);
				}
			versionChecked = 1;
		  }
	  	break;
	  case LANGUAGE_KEY:
	  	settings.lang = value;
	  	break;
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) 
{
	(void) context;

	Tuple *t = dict_read_first(iter);
	if(t)
	{
		process_tuple(t);
	}
	while(t != NULL)
	{
		t = dict_read_next(iter);
		if(t)
		{
			process_tuple(t);
		}
	}
}

void pixel_build_callback(void *data){
	stage++;
	layer_mark_dirty(fix_pixels_layer);
	if(stage == 4){
		pixel_build_timer = app_timer_register(1000, pixel_build_callback, NULL);
	}
	else{
		pixel_build_timer = app_timer_register(200, pixel_build_callback, NULL);
	}
}

void update_min_text(){
	snprintf(minute_buffer, sizeof(minute_buffer), "%d", minute);
	text_layer_set_text(actual_minute, minute_buffer);
	if(minute % 5 == 0 || minute == 0){
		text_layer_set_text(actual_minute, " ");
	}
}

void update_highlighter_location(){
	update_min_text();
	int grectminfix = 17+(10*(placements[minute]));
	inverter_layer_location = GRect(grectminfix, 8, 12, 94);
	int grectprevfix;
	if(minute % 10 != 0){
		grectprevfix = grectminfix-10;
	}
	else{
		grectprevfix = 107;
	}
	GRect previous_location = GRect(grectprevfix, 8, 12, 94);
	GRect forward1 = GRect(grectminfix+6, 8, 12, 94);
	GRect previous2 = GRect(grectprevfix-6, 8, 12, 94);
	GRect previous_location_tl = GRect(grectprevfix, 32, 12, 94);
	GRect forward1_tl = GRect(grectminfix+6, 32, 12, 94);
	GRect previous2_tl = GRect(grectprevfix-6, 32, 12, 94);
	GRect inverter_layer_location_tl = GRect(grectminfix, 32, 12, 94);
	if(animbooted == 1){
		if(minute % 10 != 0){
			if(minute != 11 && minute != 21 && minute != 31 && minute != 41 && minute != 51 && minute != 1){
				/*
				 * Never forget
				 *
				if(minute == 11 || minute == 21 || minute == 31 || minute == 41 || minute == 51 || minute == 1){
					stage = 0;
					bridge_is_right = 0;
					pixel_build_timer = app_timer_register(50, pixel_build_callback, NULL);
				}
				else if(minute == 9 || minute == 19 || minute == 29 || minute == 39 || minute == 49){
					stage = 0;
					bridge_is_right = 1;
					pixel_build_timer = app_timer_register(50, pixel_build_callback, NULL);
				}
				*/
				animate_layer(text_layer_get_layer(actual_minute), &previous_location_tl, &forward1_tl, 700, 10);
				animate_layer(text_layer_get_layer(actual_minute), &forward1_tl, &previous2_tl, 700, 710);
				animate_layer(text_layer_get_layer(actual_minute), &previous2_tl, &inverter_layer_location_tl, 700, 1420);
				animate_layer(inverter_layer_get_layer(highlighter), &previous_location, &forward1, 700, 10);
				animate_layer(inverter_layer_get_layer(highlighter), &forward1, &previous2, 700, 710);
				animate_layer(inverter_layer_get_layer(highlighter), &previous2, &inverter_layer_location, 700, 1420);
			}
			else{
				animate_layer(inverter_layer_get_layer(highlighter), &previous_location, &inverter_layer_location, 1000, 10);
				animate_layer(text_layer_get_layer(actual_minute), &previous_location_tl, &inverter_layer_location_tl, 1000, 10);
		    }
		}
		else{
			animate_layer(inverter_layer_get_layer(highlighter), &previous_location, &inverter_layer_location, 1000, 10);
			animate_layer(text_layer_get_layer(actual_minute), &previous_location_tl, &inverter_layer_location_tl, 1000, 10);
		}
	}
	else{
		animbooted = 1;
	}
}

void update_inverter_layer(){
	inverter_layer_destroy(highlighter);
	text_layer_destroy(actual_minute);
	int grectminfix = 17+(10*(placements[minute]));
	inverter_layer_location = GRect(grectminfix, 8, 12, 94);
	highlighter = inverter_layer_create(inverter_layer_location);
	actual_minute = text_layer_init(GRect(grectminfix, 32, 12, 94), GColorClear, GTextAlignmentCenter, 5);
	text_layer_set_text_color(actual_minute, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(actual_minute));
	update_min_text();
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(highlighter));
	if(settings.covernumbers == 0){
		layer_set_hidden(text_layer_get_layer(min_des_1), false);
		layer_set_hidden(text_layer_get_layer(min_des_2), false);
		layer_set_hidden(text_layer_get_layer(min_des_3), false);
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_1));
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_2));
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_des_3));
		layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(theme));
	}
}

int get_range_num(){
	int range = 0;
	if(minute > -1 && minute < 10){
		range = 1;
	}
	else if(minute > 9 && minute < 20){
		range = 2;
	}
	else if(minute > 19 && minute < 30){
		range = 3;
	}
	else if(minute > 29 && minute < 40){
		range = 4;
	}
	else if(minute > 39 && minute < 50){
		range = 5;
	}
	else if(minute > 49 && minute < 60){
		range = 6;
	}
	return range;
}

void update_range_numbers(int range){
	switch(range){
		case 1:
			text_layer_set_text(min_des_1, "0");
			text_layer_set_text(min_des_2, "5");
			text_layer_set_text(min_des_3, "10");
			break;
		case 2:
			text_layer_set_text(min_des_1, "10");
			text_layer_set_text(min_des_2, "15");
			text_layer_set_text(min_des_3, "20");
			break;
		case 3:
			text_layer_set_text(min_des_1, "20");
			text_layer_set_text(min_des_2, "25");
			text_layer_set_text(min_des_3, "30");
			break;
		case 4:
			text_layer_set_text(min_des_1, "30");
			text_layer_set_text(min_des_2, "35");
			text_layer_set_text(min_des_3, "40");
			break;
		case 5:
			text_layer_set_text(min_des_1, "40");
			text_layer_set_text(min_des_2, "45");
			text_layer_set_text(min_des_3, "50");
			break;
		case 6:
			text_layer_set_text(min_des_1, "50");
			text_layer_set_text(min_des_2, "55");
			text_layer_set_text(min_des_3, "00");
			break;
	}
}

void hide_colliding_layers(){
	int i, j, k = 0;
	for(i = 0; i < 13; i++){
		if(minute == des_1_p_h[i]){
			layer_set_hidden(text_layer_get_layer(min_des_1), true);
			i = 13;
		}
		else{
			layer_set_hidden(text_layer_get_layer(min_des_1), false);
		}
	}
	
	for(j = 0; j < 19; j++){
		if(minute == des_2_p_h[j]){
			layer_set_hidden(text_layer_get_layer(min_des_2), true);
			j = 19;
		}
		else{
			layer_set_hidden(text_layer_get_layer(min_des_2), false);
		}
	}
	
	for(k = 0; k < 7; k++){
		if(minute == des_3_p_h[k]){
			layer_set_hidden(text_layer_get_layer(min_des_3), true);
			k = 7;
		}
		else{
			layer_set_hidden(text_layer_get_layer(min_des_3), false);
		}
	}
}

void tick_handler(struct tm *t, TimeUnits units_changed){
	static char year_buffer[] = "'01";
	
	minute = t->tm_min;
	if(clock_is_24h_style()){
		strftime(hourBuffer, sizeof(hourBuffer), "%H", t);
	}
	else{
		strftime(hourBuffer,sizeof(hourBuffer),"%I", t);
	}
	int day = t->tm_wday;
	snprintf(day_buffer, sizeof(day_buffer), "%s", days[settings.lang][day]);
	
	int daym = t->tm_mday;
	int month = t->tm_mon;
	
	//For some weird reason struct tm doesn't seem to include the year, so let's make a buffer for it
	strftime(year_buffer, sizeof(year_buffer), "'%y", t);
	
	snprintf(month_buffer, sizeof(month_buffer), "%d %s. %s", daym, months[settings.lang][month], year_buffer);
	
	text_layer_set_text(day_layer, day_buffer);
	text_layer_set_text(month_layer, month_buffer);
	text_layer_set_text(hour_layer, hourBuffer);
	update_highlighter_location();
	int range = get_range_num();
	update_range_numbers(range);
	if(settings.covernumbers){
		hide_colliding_layers();
	}
	layer_mark_dirty(fix_pixels_layer);
	refresh_settings();
}

void initial_tick_handler(struct tm *t){
	static char year_buffer[] = "'01";
	
	minute = t->tm_min;
	if(clock_is_24h_style()){
		strftime(hourBuffer, sizeof(hourBuffer), "%H", t);
	}
	else{
		strftime(hourBuffer,sizeof(hourBuffer),"%I", t);
	}
	int day = t->tm_wday;
	snprintf(day_buffer, sizeof(day_buffer), "%s", days[settings.lang][day]);
	
	int daym = t->tm_mday;
	int month = t->tm_mon;
	
	strftime(year_buffer, sizeof(year_buffer), "'%y", t);
	
	snprintf(month_buffer, sizeof(month_buffer), "%d %s. %s", daym, months[settings.lang][month], year_buffer);
	
	text_layer_set_text(day_layer, day_buffer);
	text_layer_set_text(month_layer, month_buffer);
	update_inverter_layer();
	int range = get_range_num();
	update_range_numbers(range);
	if(settings.covernumbers){
		hide_colliding_layers();
	}
	layer_mark_dirty(fix_pixels_layer);
	boot = 1;
}
	
void update_proc(Layer *layer, GContext *ctx){
	graphics_draw_line(ctx, GPoint(32, 51), GPoint(32, 101));
	graphics_draw_line(ctx, GPoint(42, 51), GPoint(42, 101));
	graphics_draw_line(ctx, GPoint(52, 51), GPoint(52, 101));
	graphics_draw_line(ctx, GPoint(62, 51), GPoint(62, 101));
	graphics_draw_line(ctx, GPoint(82, 51), GPoint(82, 101));
	graphics_draw_line(ctx, GPoint(92, 51), GPoint(92, 101));
	graphics_draw_line(ctx, GPoint(102, 51), GPoint(102, 101));
	graphics_draw_line(ctx, GPoint(112, 51), GPoint(112, 101));
	graphics_fill_rect(ctx, GRect(21, 32, 3, 70), 0, GCornerNone);
	graphics_fill_rect(ctx, GRect(71, 32, 3, 70), 0, GCornerNone);
	graphics_fill_rect(ctx, GRect(121, 32, 3, 70), 0, GCornerNone);
}

void update_proc_circle(Layer *layer, GContext *ctx){
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);
	if(settings.bticonhide){
		if(phone_is_connected){
			graphics_fill_circle(ctx, GPoint(10, 157), 6);
		}
		else{
			graphics_draw_circle(ctx, GPoint(10, 157), 6);
		}
		layer_set_hidden(bitmap_layer_get_layer(bt_icon_layer), true);
	}
	else{
		if(phone_is_connected){
			bitmap_layer_set_bitmap(bt_icon_layer, bt_icon);
		}
		else{
			bitmap_layer_set_bitmap(bt_icon_layer, bt_icon_invert);
		}
	}
}

void update_proc_battery(Layer *layer, GContext *ctx){
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);
	if(settings.batterybarstyle == 0){
		graphics_draw_rect(ctx, GRect(20, 153, 114, 9));
		int bat_bar_len = 1.14*battery_percent;
		graphics_fill_rect(ctx, GRect(20, 153, bat_bar_len, 9), 0, GCornerNone);
	}
	else{
		int height = 157;
		int circle_radius = 4;
		int k, l;
		for(k = 10; k > 0; k--){
			l = 15+(12*k);
			graphics_draw_circle(ctx, GPoint(l, height), circle_radius);
		}
		
		int i, j;
		for(i = battery_percent/10; i > 0; i--){
			j = 15+(i*12);
			graphics_fill_circle(ctx, GPoint(j, height), circle_radius);
		}
	}
}

void pixels_proc(Layer *layer, GContext *ctx){
	graphics_context_set_stroke_color(ctx, GColorBlack);
	if(stage == 0){
		if(minute % 10 == 0 || minute == 0){
			graphics_draw_line(ctx, GPoint(17, 102), GPoint(19, 102));
			graphics_draw_pixel(ctx, GPoint(17, 103));
		}
		else{
			//Remove everything
		}
	}
	/*
	 * #NeverForget
	else{
		int pixel_1, pixel_2, pixel_3;
		if(bridge_is_right){
			pixel_1 = 124;
			pixel_2 = 125;
			pixel_3 = 126;
		}
		else{
			pixel_1 = 17;
			pixel_2 = 18;
			pixel_3 = 19;
		}
		if(stage < 5){
			graphics_context_set_stroke_color(ctx, GColorBlack);
		}
		else if(stage > 4){
			graphics_context_set_stroke_color(ctx, GColorWhite);
		}
		switch(stage){
			case 1:
				graphics_draw_pixel(ctx, GPoint(pixel_1, 102));
				break;
			case 2:
				graphics_draw_pixel(ctx, GPoint(pixel_2, 102));
				break;
			case 3:
				graphics_draw_pixel(ctx, GPoint(pixel_3, 102));
				break;
			case 4:
				graphics_draw_pixel(ctx, GPoint(pixel_1, 103));
				break;
			case 5:
				graphics_draw_pixel(ctx, GPoint(pixel_1, 102));
				break;
			case 6:
				graphics_draw_pixel(ctx, GPoint(pixel_2, 102));
				break;
			case 7:
				graphics_draw_pixel(ctx, GPoint(pixel_3, 102));
				break;
			case 8:
				graphics_draw_pixel(ctx, GPoint(pixel_1, 103));
				break;
		}
	}
	*/
}
	
void window_load(Window *main){
	Layer *window_layer = window_get_root_layer(main);
	
	ruler_base_outline = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BASE);
	
	base_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(base_layer, ruler_base_outline);
	layer_add_child(window_layer, bitmap_layer_get_layer(base_layer));
	
	lines_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(lines_layer, update_proc);
	layer_add_child(window_layer, lines_layer);
	
	bluetooth_circle_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(bluetooth_circle_layer, update_proc_circle);
	layer_add_child(window_layer, bluetooth_circle_layer);
	
	battery_lines_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(battery_lines_layer, update_proc_battery);
	layer_add_child(window_layer, battery_lines_layer);
	
	fix_pixels_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(fix_pixels_layer, pixels_proc);
	layer_add_child(window_layer, fix_pixels_layer);
	
	hour_layer = text_layer_init(GRect(-36, 103, 144, 168), GColorClear, GTextAlignmentCenter, 1);
	layer_add_child(window_layer, text_layer_get_layer(hour_layer));
	
	inverter_layer_location = GRect(11, 2, 12, 99);
	highlighter = inverter_layer_create(inverter_layer_location);
	layer_add_child(window_layer, inverter_layer_get_layer(highlighter));
	
	min_des_1 = text_layer_init(GRect(-50, 10, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	text_layer_set_text_color(min_des_1, GColorBlack);
	layer_add_child(window_layer, text_layer_get_layer(min_des_1));
	
	min_des_2 = text_layer_init(GRect(0, 10, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	text_layer_set_text_color(min_des_2, GColorBlack);
	layer_add_child(window_layer, text_layer_get_layer(min_des_2));
	
	min_des_3 = text_layer_init(GRect(50, 10, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	text_layer_set_text_color(min_des_3, GColorBlack);
	layer_add_child(window_layer, text_layer_get_layer(min_des_3));
	
	day_layer = text_layer_init(GRect(64, 102, 144, 168), GColorClear, GTextAlignmentLeft, 3);
	layer_add_child(window_layer, text_layer_get_layer(day_layer));
	
	month_layer = text_layer_init(GRect(21, 120, 144, 168), GColorClear, GTextAlignmentCenter, 3);
	layer_add_child(window_layer, text_layer_get_layer(month_layer));
	
	theme = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_layer, inverter_layer_get_layer(theme));
	
	bt_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
	bt_icon_invert = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_BLACK);
	
	bt_icon_layer = bitmap_layer_create(GRect(2, 148, 18, 18));
	bitmap_layer_set_bitmap(bt_icon_layer, bt_icon);
	layer_add_child(window_layer, bitmap_layer_get_layer(bt_icon_layer));
	
	update_at_a_glance = text_layer_init(GRect(0, 300, 144, 168), GColorBlack, GTextAlignmentCenter, 3);
	layer_add_child(window_layer, text_layer_get_layer(update_at_a_glance));
	
	actual_minute = text_layer_init(GRect(-20, 15, 144, 168), GColorClear, GTextAlignmentCenter, 1);
	text_layer_set_text_color(actual_minute, GColorBlack); //GG WP NO RE
	layer_add_child(window_layer, text_layer_get_layer(actual_minute));
	
	refresh_settings();
	
	struct tm *t;
  	time_t temp;        
  	temp = time(NULL);        
  	t = localtime(&temp);
	
	initial_tick_handler(t);
		
	bool con = bluetooth_connection_service_peek();
	bt_handler(con);
	
	BatteryChargeState charge = battery_state_service_peek();
	battery_handler(charge);
}

void window_unload(Window *main){
	gbitmap_destroy(ruler_base_outline);
	bitmap_layer_destroy(base_layer);
}
	
void bt_handler(bool connected){
	phone_is_connected = connected;
	layer_mark_dirty(bluetooth_circle_layer);
	if(boot == 1){
		if(booted == 1){
			if(settings.btdisalert == 1 && connected == 0){
				glance_this("Bluetooth disconnected.", 1, 3, 5000, 0);
			}

			if(settings.btrealert == 1 && connected == 1){
				glance_this("Bluetooth reconnected.", 1, 2, 5000, 0);
			}
		}
		else{
			booted = 1;
		}
	}
}

void charge_invert(void *data){
	invert = !invert;
	if(invert){
		if(battery_percent != 100){
			battery_percent += 10;
		}
		layer_mark_dirty(battery_lines_layer);
	}
	else{
		if(battery_percent != 0){
			battery_percent -= 10;
		}
		layer_mark_dirty(battery_lines_layer);
	}
	charge_timer = app_timer_register(1000, charge_invert, NULL);
}

void battery_handler(BatteryChargeState batt_charge){
	battery_percent = batt_charge.charge_percent;
	layer_mark_dirty(battery_lines_layer);
	if(batt_charge.is_charging){
		cancelled = 0;
		app_timer_cancel(charge_timer);
		charge_timer = app_timer_register(1000, charge_invert, NULL);
	}
	else{
		if(!cancelled){
			app_timer_cancel(charge_timer);
			cancelled = 1;
		}
	}
}

void init(){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
	});
	tick_timer_service_subscribe(MINUTE_UNIT, &tick_handler);
	bluetooth_connection_service_subscribe(&bt_handler);
	battery_state_service_subscribe(battery_handler);
	app_message_open(512, 258);
	app_message_register_inbox_received(in_received_handler);
	
	
	int value = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
	APP_LOG(APP_LOG_LEVEL_INFO, "%d bytes read from settings", value);
	
	settings.lang = 0;
	
	impact = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMPACT_35));	
	window_stack_push(window, true);
}

void deinit(){
	window_destroy(window);
	tick_timer_service_unsubscribe();
	fonts_unload_custom_font(impact);
	int value = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	APP_LOG(APP_LOG_LEVEL_INFO, "%d bytes written to settings", value);
}

int main(){
	init();
	app_event_loop();
	deinit();
}