#include "ui.h"

/**************************************************************
 *
 *	Function prototypes
 *
 ***************************************************************/
static void ui_weather_screen_evt_handler(lv_event_t * e);

/**************************************************************
 *
 *	Global variables
 *
 ***************************************************************/
static lv_obj_t *ui_WeatherScreen, *ui_WeatherScreenBackButton,
			*ui_WeatherScreenCityLabel, *ui_WeatherScreenCountryLabel, *ui_WeatherScreenIcon,
			*ui_WeatherScreenTempArc, *ui_WeatherScreenTempLabel, *ui_WeatherScreenTempMinMaxLabel,
			*ui_WeatherScreenSunriseIconLabel, *ui_WeatherScreenRainIconLabel,
			*ui_WeatherScreenSnowIconLabel, *ui_WeatherScreenWindIconLabel,
			*ui_WeatherScreenSunriseLabel, *ui_WeatherScreenRainLabel,
			*ui_WeatherScreenWindLabel, *ui_WeatherScreenSnowLabel;

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* initialize weather screen */
void UI_WeatherScreen_Init(void){

	UI_ScreenCreate(&ui_WeatherScreen);

	UI_BackButtonCreate(&ui_WeatherScreen, &ui_WeatherScreenBackButton);
	lv_obj_add_event_cb(ui_WeatherScreenBackButton, ui_weather_screen_evt_handler, LV_EVENT_RELEASED, NULL);

    ui_WeatherScreenCityLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenCityLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenCityLabel, 30);
    lv_obj_set_y(ui_WeatherScreenCityLabel, 30);
    lv_obj_set_width(ui_WeatherScreenCityLabel, 190);
    lv_label_set_long_mode(ui_WeatherScreenCityLabel, LV_LABEL_LONG_SCROLL);

    ui_WeatherScreenCountryLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenCountryLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_WeatherScreenCountryLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenCountryLabel, 30);
    lv_obj_set_y(ui_WeatherScreenCountryLabel, 65);

    ui_WeatherScreenIcon = lv_img_create(ui_WeatherScreen);
    lv_obj_set_width(ui_WeatherScreenIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_WeatherScreenIcon, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_WeatherScreenIcon, -30);
    lv_obj_set_y(ui_WeatherScreenIcon, 30);
    lv_obj_set_align(ui_WeatherScreenIcon, LV_ALIGN_TOP_RIGHT);
    lv_obj_clear_flag(ui_WeatherScreenIcon, LV_OBJ_FLAG_SCROLLABLE);

    ui_WeatherScreenTempArc = lv_arc_create(ui_WeatherScreen);
    lv_obj_set_width(ui_WeatherScreenTempArc, 150);
    lv_obj_set_height(ui_WeatherScreenTempArc, 150);
    lv_obj_set_x(ui_WeatherScreenTempArc, 0);
    lv_obj_set_y(ui_WeatherScreenTempArc, -60);
    lv_obj_set_align(ui_WeatherScreenTempArc, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_WeatherScreenTempArc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_range(ui_WeatherScreenTempArc, -50, 50);
    lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_WeatherScreenTempArc, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_hex(0x4040FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_WeatherScreenTempArc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_WeatherScreenTempArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_WeatherScreenTempLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenTempLabel, &UI_Text30Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenTempLabel, 0);
    lv_obj_set_y(ui_WeatherScreenTempLabel, -70);
    lv_obj_set_align(ui_WeatherScreenTempLabel, LV_ALIGN_CENTER);

    ui_WeatherScreenTempMinMaxLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenTempMinMaxLabel, &UI_Text14Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(ui_WeatherScreenTempMinMaxLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(ui_WeatherScreenTempMinMaxLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_WeatherScreenSunriseIconLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenSunriseIconLabel, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenSunriseIconLabel, -110);
    lv_obj_set_y(ui_WeatherScreenSunriseIconLabel, 60);
    lv_obj_set_align(ui_WeatherScreenSunriseIconLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(ui_WeatherScreenSunriseIconLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_WeatherScreenSunriseIconLabel, "%c", ICON_SUNRISE);

    ui_WeatherScreenRainIconLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenRainIconLabel, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenRainIconLabel, -110);
    lv_obj_set_y(ui_WeatherScreenRainIconLabel, 120);
    lv_obj_set_align(ui_WeatherScreenRainIconLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(ui_WeatherScreenRainIconLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_WeatherScreenRainIconLabel, "%c", ICON_RAIN);

    ui_WeatherScreenSnowIconLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenSnowIconLabel, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenSnowIconLabel, 40);
    lv_obj_set_y(ui_WeatherScreenSnowIconLabel, 120);
    lv_obj_set_align(ui_WeatherScreenSnowIconLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(ui_WeatherScreenSnowIconLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_WeatherScreenSnowIconLabel, "%c", ICON_SNOW);

    ui_WeatherScreenWindIconLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenWindIconLabel, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenWindIconLabel, 40);
    lv_obj_set_y(ui_WeatherScreenWindIconLabel, 60);
    lv_obj_set_align(ui_WeatherScreenWindIconLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(ui_WeatherScreenWindIconLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_WeatherScreenWindIconLabel, "%c", ICON_WIND);

    ui_WeatherScreenSunriseLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenSunriseLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenSunriseLabel, -50);
    lv_obj_set_y(ui_WeatherScreenSunriseLabel, 60);
    lv_obj_set_align(ui_WeatherScreenSunriseLabel, LV_ALIGN_CENTER);

    ui_WeatherScreenRainLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenRainLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenRainLabel, -50);
    lv_obj_set_y(ui_WeatherScreenRainLabel, 120);
    lv_obj_set_align(ui_WeatherScreenRainLabel, LV_ALIGN_CENTER);

    ui_WeatherScreenWindLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenWindLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenWindLabel, 100);
    lv_obj_set_y(ui_WeatherScreenWindLabel, 60);
    lv_obj_set_align(ui_WeatherScreenWindLabel, LV_ALIGN_CENTER);

    ui_WeatherScreenSnowLabel = lv_label_create(ui_WeatherScreen);
    lv_obj_add_style(ui_WeatherScreenSnowLabel, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_WeatherScreenSnowLabel, 100);
    lv_obj_set_y(ui_WeatherScreenSnowLabel, 120);
    lv_obj_set_align(ui_WeatherScreenSnowLabel, LV_ALIGN_CENTER);
}

/* load weather screen */
void UI_WeatherScreen_Load(void){

	lv_scr_load_anim(ui_WeatherScreen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/*
 * update detailed information about weather:
 * - current city and country
 * - weather icon
 * - average, minimum and maximum temperature
 * - sunrise and sunset time
 * - precipation and chance of of rain and snow
 * - current and maximum wind
 * */

void UI_WeatherScreen_UpdateCityName(char *city_name){

	if(0 == city_name) return;

	lv_label_set_text(ui_WeatherScreenCityLabel, city_name);
	if(heap_caps_get_allocated_size(city_name)) free(city_name);
}

void UI_WeatherScreen_UpdateCountryName(char *country_name){

	if(0 == country_name) return;

	lv_label_set_text(ui_WeatherScreenCountryLabel, country_name);
	if(heap_caps_get_allocated_size(country_name)) free(country_name);
}

void UI_WeatherScreen_UpdateWeatherIcon(char *icon_path){

	if(0 == icon_path) return;

	size_t len;
	int a;
	char *buff = 0;

	// allocate buffer for image path
	len = strnlen(icon_path, 64);
	if(64 == len) goto cleanup;
	buff = malloc(len + 3);
	if(0 == buff) goto cleanup;

	// prepare path string
	a = sprintf(buff, "%c:%s", LV_FS_STDIO_LETTER, icon_path);
	if(a != (len + 2)) goto cleanup;

	// set image from path
	lv_img_set_src(ui_WeatherScreenIcon, buff);

	cleanup:
		if(buff){
			if(heap_caps_get_allocated_size(buff)) free(buff);
		}
		if(icon_path){
			if(heap_caps_get_allocated_size(icon_path)) free(icon_path);
		}
}

void UI_WeatherScreen_UpdateAvgTemp(int avg_temp){

	uint8_t r, g, b, diff;

	if(((int)-50 > avg_temp) || ((int)50 < avg_temp)) return;

	// set temprature value as text and as arc value
	lv_label_set_text_fmt(ui_WeatherScreenTempLabel, "%+d°C", avg_temp);
	lv_arc_set_value(ui_WeatherScreenTempArc, avg_temp);

	// calculate color of ui_WeatherScreenTempArc
	/* <------------------------------------------------------------------------------------------------>
	 * -50                                   -10          0          10          25                    50     degC
	 * 0                                      0                      0 --------> 255                   255    R
	 * 0 -----------------------------------> 255                    255         255 <---------------- 0      G
	 * 255                                    255 <----------------- 0           0                     0      B
	 *
	 * The lowest values of temperature are pure blue, rising up, color shifts to green, then yellow, orange and
	 * pure red by highest values
	 * */

	if((int)-10 >= avg_temp){

		r = 0;
		b = 255;
		diff = abs((int)-50 - avg_temp);
		g = ((unsigned int)(diff * 255))/(unsigned int)40;
	}
	else if(((int)-10 < avg_temp) && ((int)10 >= avg_temp)){

		r = 0;
		g = 255;
		diff = abs((int)10 - avg_temp);
		b = ((unsigned int)(diff * 255))/(unsigned int)20;
	}
	else if(((int)10 < avg_temp) && ((int)25 >= avg_temp)){

		g = 255;
		b = 0;
		diff = abs(avg_temp - (int)10);
		r = ((unsigned int)(diff * 255))/(unsigned int)15;
	}
	else{

		r = 255;
		b = 0;
		diff = abs((int)50 - avg_temp);
		g = ((unsigned int)(diff * 255))/(unsigned int)25;
	}

	lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_make(r, g, b), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(ui_WeatherScreenTempArc, lv_color_make(r, g, b), LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

void UI_WeatherScreen_UpdateMinMaxTemp(int min_temp, int max_temp){

	lv_label_set_text_fmt(ui_WeatherScreenTempMinMaxLabel, "min/max\n%+d°C / %+d°C", min_temp, max_temp);
}

void UI_WeatherScreen_UpdateSunriseSunsetTime(char *sunrise_time, char *sunset_time){

	if((0 == sunrise_time) || (0 == sunset_time)) goto cleanup;

	lv_label_set_text_fmt(ui_WeatherScreenSunriseLabel, "%s\n%s", sunrise_time, sunset_time);

	cleanup:
		if(sunrise_time){
			if(heap_caps_get_allocated_size(sunrise_time)) free(sunrise_time);
		}
		if(sunset_time){
			if(heap_caps_get_allocated_size(sunset_time)) free(sunset_time);
		}
}

void UI_WeatherScreen_UpdatePrecipPercentRain(int precip, int percent){

	lv_label_set_text_fmt(ui_WeatherScreenRainLabel, "%dmm\n%d%%", precip, percent);
}

void UI_WeatherScreen_UpdateAvgMaxWind(int avg, int max){

	lv_label_set_text_fmt(ui_WeatherScreenWindLabel, "%dkm/h\n%dkm/h", avg, max);
}

void UI_WeatherScreen_UpdatePrecipPercentSnow(int precip, int percent){

	lv_label_set_text_fmt(ui_WeatherScreenSnowLabel, "%dmm\n%d%%", precip, percent);
}


/**************************************************************
 *
 * Private function definitions
 *
 ***************************************************************/

/* weather screen events handler */
static void ui_weather_screen_evt_handler(lv_event_t * e){

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if((event_code == LV_EVENT_RELEASED) && (target == ui_WeatherScreenBackButton)) {

    	UI_ReportEvt(UI_EVT_WEATHERSCR_BACK_BTN_CLICKED, 0);
    }
}
