/*
 * gui_main.c
 *
 *  Created on: Jun 28, 2024
 *      Author: wang
 */

#include "gui.h"
#include <stdio.h>
#include "mqtt.h"
#include <stdlib.h>
lv_style_t style;
struct my_lvgl_screen* cur_scr=NULL;
///////common


void update_status(lv_obj_t* lbl)
{
	char buf[512];
	pthread_mutex_lock(&mqttmutex);

	/*sprintf(buf,"ip:%s,atmospheric pressure:%s,bmp temp:%s,bmp altitude:%s",
			stm32ip,atom_pressure,temperature,altitude);*/
	if(last_mqtt_data.mqtt_flag)
	{
		sprintf(buf,"ip:%s,ap:%.2f,bmp temp:%.2f,bmp altitude:%.2f",
				last_mqtt_data.ip,
				last_mqtt_data.bmp_ap,
				last_mqtt_data.bmp_temp,
				last_mqtt_data.bmp_alt);
		lv_label_set_text(lbl,buf);
	}
	pthread_mutex_unlock(&mqttmutex);
	
}
void on_button_back_event(lv_event_t* e)
{
	struct my_lvgl_screen* scr=(struct my_lvgl_screen*)e->user_data;
	if(scr->create_cb)
	{
		struct my_lvgl_screen* newScr=scr->create_cb(scr);
		my_screen_load(newScr);
	}
}

lv_obj_t* create_button_back(lv_obj_t* parent,void* cb_data)
{
	lv_obj_t* x= create_button_with_label(parent,"返回",on_button_back_event,cb_data);
	lv_obj_set_size(x,85,25);
	lv_obj_set_pos(x,715,425);
	return x;
}
///////main

struct my_main_scr
{
	lv_obj_t* grid;
	lv_obj_t* btn_stock;
	lv_obj_t* btn_stm32h7;
	lv_obj_t* btn_linux;
	lv_obj_t* btn_weather;
	lv_obj_t* btn_exit;
	lv_obj_t* lbl_status;
};

void on_delete_scr_main(struct my_lvgl_screen* scr)
{
	struct my_main_scr* data=(struct my_main_scr*)scr->userData;
	scr->userData=NULL;
	printf("line %d,%s,delete ...\n",__LINE__,__FUNCTION__);
	lv_obj_del_async(data->btn_stock);
	lv_obj_del_async(data->btn_stm32h7);
	lv_obj_del_async(data->btn_linux);
	lv_obj_del_async(data->btn_weather);
	lv_obj_del_async(data->btn_exit);
	lv_obj_del_async(data->lbl_status);
	lv_obj_del_async(data->grid);
	free(data);
}

void on_update_scr_main(struct my_lvgl_screen* scr)
{
	struct my_main_scr* data=(struct my_main_scr*)scr->userData;
	time_t t;
    
    time(&t);
    struct tm* tm=localtime(&t);
	char disp_buf[128];
    sprintf(disp_buf,"%d年%02d月%02d日%s %02d:%02d:%02d",
        1900+tm->tm_year,tm->tm_mon+1,tm->tm_mday,
        getWeekName(tm->tm_wday),tm->tm_hour,tm->tm_min,tm->tm_sec);

	lv_label_set_text(data->lbl_status,disp_buf);
	//update_status(data->lbl_status);
}

void btn_event_stm32h7_cb(lv_event_t* e)
{
	struct my_lvgl_screen* scr=create_screen_dev_main();
	my_screen_load(scr);
}
void btn_event_stock_cb(lv_event_t* e)
{
	struct my_lvgl_screen* scr=create_screen_stock_main();
	my_screen_load(scr);
}
void btn_event_linux_cb(lv_event_t* e)
{
	struct my_lvgl_screen* scr=create_screen_linux_main();
	my_screen_load(scr);
}
void btn_event_weather_cb(lv_event_t* e)
{
	struct my_lvgl_screen* scr=create_screen_weather_main();
	my_screen_load(scr);
}

void btn_event_exit_cb(lv_event_t* e)
{
	exit(0);
}
struct my_lvgl_screen* create_main_scr()
{
	#define BUTTON_SIZE	180
	struct my_lvgl_screen* ret=create_default_screen();
	struct my_main_scr* data=(struct my_main_scr*)malloc(sizeof(struct my_main_scr));
	static int32_t col_dsc[]={BUTTON_SIZE,BUTTON_SIZE,BUTTON_SIZE,BUTTON_SIZE,BUTTON_SIZE,LV_GRID_TEMPLATE_LAST};
	static int32_t row_dsc[]={BUTTON_SIZE,BUTTON_SIZE,130,40,LV_GRID_TEMPLATE_LAST};
	data->grid=lv_obj_create(ret->screen);
	//lv_obj_set_grid_dsc_array(ret->screen,col_dsc,row_dsc);
	lv_obj_set_style_grid_column_dsc_array(data->grid,col_dsc,0);
	lv_obj_set_style_grid_row_dsc_array(data->grid,row_dsc,0);
	lv_obj_set_layout(data->grid,LV_LAYOUT_GRID);
	lv_obj_set_size(data->grid,SCREEN_WIDTH,SCREEN_HEIGHT);
	lv_obj_set_pos(data->grid,0,0);

	data->btn_stock=create_button_with_label(data->grid,"stock",btn_event_stock_cb,ret);
	lv_obj_set_size(data->btn_stock,BUTTON_SIZE,BUTTON_SIZE);
	//lv_obj_set_size(data->btn_stock,200,200);
	//lv_obj_set_pos(data->btn_stock,5,5);
	lv_obj_set_grid_cell(data->btn_stock,LV_GRID_ALIGN_START,0,1,LV_GRID_ALIGN_START,0,1);


	data->btn_stm32h7=create_button_with_label(data->grid,"stm32h7",btn_event_stm32h7_cb,ret);
	lv_obj_set_size(data->btn_stm32h7,BUTTON_SIZE,BUTTON_SIZE);
	//lv_obj_set_size(data->btn_stm32h7,200,200);
	//lv_obj_set_pos(data->btn_stm32h7,210,5);
	lv_obj_set_grid_cell(data->btn_stm32h7,LV_GRID_ALIGN_START,1,1,LV_GRID_ALIGN_START,0,1);

	data->btn_linux=create_button_with_label(data->grid,"linux",btn_event_linux_cb,ret);
	lv_obj_set_size(data->btn_linux,BUTTON_SIZE,BUTTON_SIZE);
	//lv_obj_set_size(data->btn_linux,200,200);
	//lv_obj_set_pos(data->btn_linux,415,5);
	lv_obj_set_grid_cell(data->btn_linux,LV_GRID_ALIGN_START,2,1,LV_GRID_ALIGN_START,0,1);

	data->btn_weather=create_button_with_label(data->grid,"weather",btn_event_weather_cb,ret);
	lv_obj_set_size(data->btn_weather,BUTTON_SIZE,BUTTON_SIZE);
	//lv_obj_set_size(data->btn_weather,200,200);
	//lv_obj_set_pos(data->btn_weather,415,5);
	lv_obj_set_grid_cell(data->btn_weather,LV_GRID_ALIGN_START,3,1,LV_GRID_ALIGN_START,0,1);

	data->btn_exit=create_button_with_label(data->grid,"exit",btn_event_exit_cb,ret);
	lv_obj_set_size(data->btn_exit,BUTTON_SIZE,BUTTON_SIZE);
	lv_obj_set_grid_cell(data->btn_exit,LV_GRID_ALIGN_START,4,1,LV_GRID_ALIGN_START,1,1);

	data->lbl_status=create_status_bar(data->grid);
	lv_obj_set_grid_cell(data->lbl_status,LV_GRID_ALIGN_START,0,5,LV_GRID_ALIGN_START,3,1);

	ret->delete_cb=on_delete_scr_main;
	ret->update_cb=on_update_scr_main;
	ret->userData=data;
	printf("main\n");
	return ret;
}
