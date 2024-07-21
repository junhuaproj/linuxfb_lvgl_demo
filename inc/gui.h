/*
 * gui.h
 *
 *  Created on: Jun 26, 2024
 *      Author: wang
 */

#ifndef INC_GUI_H_
#define INC_GUI_H_

#include "lvgl/lvgl.h"
#include "weather.h"

#define SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	600

typedef struct my_lvgl_screen;
typedef void (*delete_lvgl_screen)(struct my_lvgl_screen* scr);
typedef void (*update_lvgl_screen)(struct my_lvgl_screen* scr);
typedef struct my_lvgl_screen* (*create_lvgl_create)(struct my_lvgl_screen* scr);
struct my_lvgl_screen
{
	lv_obj_t* screen;
	lv_obj_t* btn_back;
	create_lvgl_create create_cb;
	delete_lvgl_screen delete_cb;
	update_lvgl_screen update_cb;
	void* userData;
};
extern lv_style_t style;
struct my_lvgl_screen* create_default_screen();

lv_obj_t* create_button_with_label(lv_obj_t* parent,const char* title,lv_event_cb_t cb,void* cb_data);
lv_obj_t* create_button_back(lv_obj_t* parent,void* cb_data);
lv_obj_t* create_status_bar(lv_obj_t* parent);
//清空lv_list所有项
void clear_lv_list(lv_obj_t* list);

void my_screen_load(struct my_lvgl_screen* scr);

struct my_lvgl_screen* create_main_scr();
void update_status(lv_obj_t* lbl);
void init_screen();
struct my_lvgl_screen* create_screen_stock_main();
struct my_lvgl_screen* create_screen_dev_main();
struct my_lvgl_screen* create_screen_linux_main();
struct my_lvgl_screen* create_screen_weather_main();
struct my_lvgl_screen* create_screen_weather_station_main();
struct my_lvgl_screen*  create_scr_predict(const struct Weather_Predict*);
const char* getWeekName(int week);

void update_screen();

#endif /* INC_GUI_H_ */
