#ifndef MAIN_UI_UI_WEATHER_SCREEN_H_
#define MAIN_UI_UI_WEATHER_SCREEN_H_

#include "main.h"

/**************************************************************
 * Public functions
 ***************************************************************/
void UI_WeatherScrren_Init(void);
void UI_WeatherScreen_Load(void);
void UI_WeatherScreen_UpdateCityName(char *city_name);
void UI_WeatherScreen_UpdateCountryName(char *country_name);
void UI_WeatherScreen_UpdateWeatherIcon(char *icon_path);
void UI_WeatherScreen_UpdateAvgTemp(int avg_temp);
void UI_WeatherScreen_UpdateMinMaxTemp(int min_temp, int max_temp);
void UI_WeatherScreen_UpdateSunriseSunsetTime(char *sunrise_time, char *sunset_time);
void UI_WeatherScreen_UpdatePrecipPercentRain(int precip, int percent);
void UI_WeatherScreen_UpdateAvgMaxWind(int avg, int max);
void UI_WeatherScreen_UpdatePrecipPercentSnow(int precip, int percent);

#endif /* MAIN_UI_UI_WEATHER_SCREEN_H_ */
