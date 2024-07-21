/*
 * gui_dev.c
 *
 *  Created on: Jun 28, 2024
 *      Author: wang
 */




#include "gui.h"
#include "mqtt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define _SEN_TABLE_
#ifdef _SEN_TABLE_
#else
#define SENSOR_VALUE_MAX	200
struct ui_sensor
{
	lv_obj_t* layout;
	lv_obj_t* value;
	lv_obj_t* name;
	float values[SENSOR_VALUE_MAX];
	int val_cnt;
	char dev[32];
	char ip[32];
	char lasttime[32];
};

struct my_canvas
{
	int width;
	int height;
	lv_obj_t* canvas;
	uint8_t* buf_draw_buf;
	lv_draw_buf_t draw_buf;
};
#endif

struct my_scr_dev
{
	lv_obj_t* lbl_pwm;
	lv_obj_t* slider_pwm;
	lv_obj_t* lbl_status;
	#ifdef _SEN_TABLE_
	lv_obj_t* table_sensor;
	#else

	lv_obj_t* layout_sensor;
	struct ui_sensor ds18b20;
	struct ui_sensor dht_temp;
	struct ui_sensor dht_humidity;
	struct ui_sensor bmp_temp;
	struct ui_sensor bmp_alt;
	struct ui_sensor bmp_ap;
	lv_obj_t* lbl_cur;
	struct my_canvas canvas;
	int cur_update_flag;
	struct ui_sensor* cur_sensor;
	//lv_style_t sensor_normal_style;
	//lv_style_t sensor_check_style;
	#endif
};
#ifdef _SEN_TABLE_
#else

void free_my_canvas(struct my_canvas* canvas)
{
	lv_obj_del_async(canvas->canvas);
	if(canvas->buf_draw_buf)
	{
		free(canvas->buf_draw_buf);
		canvas->buf_draw_buf=NULL;
	}
}
void init_my_canvas(struct my_canvas* canvas,int width,int height,lv_obj_t* parent)
{
	canvas->canvas=lv_canvas_create(parent);
	//lv_obj_set_pos(canvas->canvas,5,250);
	lv_obj_set_size(canvas->canvas,width,height);
//LV_DRAW_BUF_DEFINE_STATIC
//LV_DRAW_BUF_INIT_STATIC
	int buf_size=_LV_DRAW_BUF_SIZE(width, height, LV_COLOR_FORMAT_ARGB8888);
	canvas->height=height;
	canvas->width=width;
	canvas->buf_draw_buf=(uint8_t*)malloc(buf_size);
	
	canvas->draw_buf.header.magic=LV_IMAGE_HEADER_MAGIC;
	canvas->draw_buf.header.cf = LV_COLOR_FORMAT_ARGB8888;
	canvas->draw_buf.header.flags=LV_IMAGE_FLAGS_MODIFIABLE;
	canvas->draw_buf.header.w=width;
	canvas->draw_buf.header.h=height;
	canvas->draw_buf.header.stride=_LV_DRAW_BUF_STRIDE(width, LV_COLOR_FORMAT_ARGB8888);
	canvas->draw_buf.header.reserved_2=0;
	canvas->draw_buf.data_size=buf_size;
	canvas->draw_buf.data=canvas->buf_draw_buf;
	canvas->draw_buf.unaligned_data=canvas->buf_draw_buf;
    
	
    lv_image_header_t * header = &canvas->draw_buf.header;
    lv_draw_buf_init(&canvas->draw_buf, header->w, header->h, header->cf, header->stride, 
		canvas->buf_draw_buf, buf_size);
    lv_draw_buf_set_flag(&canvas->draw_buf, LV_IMAGE_FLAGS_MODIFIABLE);

	lv_canvas_set_draw_buf(canvas->canvas,&canvas->draw_buf);
	lv_canvas_fill_bg(canvas->canvas,lv_palette_main(LV_PALETTE_BLUE_GREY),LV_OPA_COVER);
}
void sensor_update_value(float v,struct ui_sensor* sensor,struct sensor_data* mqtt)
{
	if(sensor->val_cnt<SENSOR_VALUE_MAX-1)
	{
		sensor->values[sensor->val_cnt]=v;
		sensor->val_cnt++;
	}
	else
	{
		memmove(sensor->values,sensor->values+1,sizeof(sizeof(float)*(SENSOR_VALUE_MAX-1)));
		sensor->values[SENSOR_VALUE_MAX-1]=v;
	}
	strcpy(sensor->dev,mqtt->dev);
	strcpy(sensor->ip,mqtt->ip);
	strcpy(sensor->lasttime,mqtt->datetime);	
}
static void destroy_uisensor(struct ui_sensor* ui)
{
	lv_obj_del_async(ui->name);
	lv_obj_del_async(ui->value);
	lv_obj_del_async(ui->layout);
}
static void create_uisensor(struct ui_sensor* ui,lv_obj_t* parent,struct my_scr_dev* dev);
struct ui_sensor* find_ui_sensor(struct my_scr_dev* data,lv_obj_t* layout,int *flag)
{
	if(data->ds18b20.layout==layout)
	{
		*flag=sensor_flag_ds18b20;
		return &data->ds18b20;
	}
	if(data->bmp_alt.layout==layout)
	{
		*flag=sensor_flag_bmp_alt;
		return &data->bmp_alt;
	}
	if(data->bmp_ap.layout==layout)
	{
		*flag=sensor_flag_bmp_ap;
		return &data->bmp_ap;
	}
	if(data->bmp_temp.layout==layout)
	{
		*flag=sensor_flag_bmp_temp;
		return &data->bmp_temp;
	}
	if(data->dht_humidity.layout==layout)
	{
		*flag=sensor_flag_dht_hum;
		return &data->dht_humidity;
	}
	if(data->dht_temp.layout==layout)
	{
		*flag=sensor_flag_dht_temp;
		return &data->dht_temp;
	}
	return NULL;
}

void set_cur_label(struct ui_sensor* sensor,lv_obj_t* label)
{
	char buf[96];
	sprintf(buf,"%s:%s,%s",sensor->dev,sensor->ip,sensor->lasttime);
	lv_label_set_text(label,buf);
}
struct sensor_plot_arg
{
	float minV;
	float maxV;
	float rate;
	float item_width;
};
#include <math.h>
void get_sensor_plot_arg(struct sensor_plot_arg* arg,struct ui_sensor* sensor)
{
	int i;
	arg->minV=__FLT_MAX__;
	arg->maxV=__FLT_MIN__;
	for(i=0;i<sensor->val_cnt;i++)
	{
		arg->minV=fmin(arg->minV,sensor->values[i]);
		arg->maxV=fmax(arg->maxV,sensor->values[i]);
	}
}

void on_update_canvas(struct ui_sensor* sensor,struct my_canvas* canvas)
{
	//const int height=180;
	//const int top=5;
	int i,y;
	int prevx=0,prevy=0;
	//float rate;
	lv_layer_t layer;
	lv_draw_line_dsc_t dsc;
	//struct minute_line_arg arg;
	struct sensor_plot_arg arg;
	float curVal;
	const int top=5,bottom=canvas->height-5;
	const int height=bottom-top;
	get_sensor_plot_arg(&arg,sensor);

	lv_canvas_fill_bg(canvas->canvas,lv_palette_main(LV_PALETTE_BLUE),LV_OPA_COVER);
	if(sensor->val_cnt==0)return;
	arg.item_width=2;//lv_obj_get_width(canvas)/arg.minute_cnt;
	arg.rate=height/(arg.maxV-arg.minV);
	//rate=height/(high-low);
	printf("price range:%.2f\n",arg.rate);
	
	lv_canvas_init_layer(canvas->canvas,&layer);
	lv_draw_line_dsc_init(&dsc);
	dsc.color=lv_palette_main(LV_PALETTE_RED);
	dsc.width=1;

	for(i=0;i<sensor->val_cnt;i++)
	{
		curVal=sensor->values[i];
		
		y=top+height-(curVal-arg.minV)*arg.rate;
			//printf("height:%.2f\n",rate);
		if(prevy==0)
		{
			prevy=y;
			prevx=0;
		}
		else
		{
			dsc.p1.x=prevx;
			dsc.p1.y=prevy;
			dsc.p2.x=prevx+arg.item_width;
			dsc.p2.y=y;
			lv_draw_line(&layer,&dsc);
			prevy=y;
			prevx=dsc.p2.x;
		}
			//lv_canvas_set_px(data->canvas,x,y,lv_palette_main(LV_PALETTE_RED),LV_OPA_20);
	}
	
	lv_canvas_finish_layer(canvas->canvas,&layer);
}
void on_cur_sensor_changed(struct my_scr_dev* data,struct my_canvas* canvas)
{
	if(data->cur_sensor)
	{
		set_cur_label(data->cur_sensor,data->lbl_cur);
	}
	else
		lv_label_set_text(data->lbl_cur,"");
}

void sensor_event_handler(lv_event_t* e)
{
	struct my_scr_dev* data=(struct my_scr_dev*)lv_event_get_user_data(e);
	lv_obj_t * cont = lv_event_get_current_target(e);
	lv_obj_t* old=lv_obj_get_user_data(data->layout_sensor);
	if(1)//cont==data->ds18b20.layout)
	{
		printf("get...\n");
		if(old)
		{
			//lv_obj_set_style_bg_color(cont,0xff,LV_PART_MAIN);
			lv_obj_set_style_bg_opa(cont,0xff,LV_PART_MAIN);
			//lv_obj_remove_state(old,LV_STATE_CHECKED);
			lv_obj_set_style_bg_color(old,lv_color_hex3(0xfff),LV_PART_MAIN);
		}

		lv_obj_add_state(cont,LV_STATE_CHECKED);
		lv_obj_set_style_bg_opa(cont,0xff,LV_PART_MAIN);
		lv_obj_set_style_bg_color(cont,lv_color_hex3(0x99f),LV_PART_MAIN);
		data->cur_sensor=find_ui_sensor(data,cont,&data->cur_update_flag);
		on_cur_sensor_changed(data,&data->canvas);
		//printf("cur dev:%s\n",data->cur_sensor->dev);

		if(data->cur_sensor)
			on_update_canvas(data->cur_sensor,&data->canvas);
		lv_obj_set_user_data(data->layout_sensor,cont);
	}
	else
	{
		printf("error...\n");
	}
	#if 0
    
	int old_index=(int)lv_obj_get_user_data(cont);
    lv_obj_t * act_cb = lv_event_get_target(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, old_index);

    /*Do nothing if the container was clicked*/
    if(act_cb == cont) return;

    lv_obj_remove_state(old_cb, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/

	lv_obj_set_user_data(cont,(void*)lv_obj_get_index(act_cb));

    printf("Selected radio buttons: %d\n", (int)lv_obj_get_index(act_cb));
	#endif
}
void create_ui_sensor(struct my_scr_dev* dev,lv_obj_t* screen)
{
	static uint32_t col_dsc[]={325,325,325,LV_GRID_TEMPLATE_LAST};
	static uint32_t row_dsc[]={110,110,LV_GRID_TEMPLATE_LAST};
	dev->layout_sensor=lv_obj_create(screen);
	lv_obj_set_style_grid_column_dsc_array(dev->layout_sensor,col_dsc,LV_PART_MAIN);
	lv_obj_set_style_grid_row_dsc_array(dev->layout_sensor,row_dsc,LV_PART_MAIN);
	lv_obj_set_size(dev->layout_sensor,1014,255);
	lv_obj_set_pos(dev->layout_sensor,5,5);
	lv_obj_set_layout(dev->layout_sensor,LV_LAYOUT_GRID);
	lv_obj_set_style_pad_all(dev->layout_sensor,3,LV_PART_MAIN);

	
	create_uisensor(&dev->ds18b20,dev->layout_sensor,dev);
	create_uisensor(&dev->dht_temp,dev->layout_sensor,dev);
	create_uisensor(&dev->dht_humidity,dev->layout_sensor,dev);
	create_uisensor(&dev->bmp_temp,dev->layout_sensor,dev);
	create_uisensor(&dev->bmp_ap,dev->layout_sensor,dev);
	create_uisensor(&dev->bmp_alt,dev->layout_sensor,dev);
	lv_obj_set_grid_cell(dev->ds18b20.layout,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,1);
	lv_obj_set_grid_cell(dev->dht_temp.layout,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,0,1);
	lv_obj_set_grid_cell(dev->dht_humidity.layout,LV_GRID_ALIGN_STRETCH,2,1,LV_GRID_ALIGN_STRETCH,0,1);
	lv_obj_set_grid_cell(dev->bmp_temp.layout,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,1,1);
	lv_obj_set_grid_cell(dev->bmp_ap.layout,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,1,1);
	lv_obj_set_grid_cell(dev->bmp_alt.layout,LV_GRID_ALIGN_STRETCH,2,1,LV_GRID_ALIGN_STRETCH,1,1);

	lv_obj_add_event_cb(dev->ds18b20.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);
	lv_obj_add_event_cb(dev->dht_humidity.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);
	lv_obj_add_event_cb(dev->dht_temp.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);

	lv_obj_add_event_cb(dev->bmp_alt.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);
	lv_obj_add_event_cb(dev->bmp_ap.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);
	lv_obj_add_event_cb(dev->bmp_temp.layout,sensor_event_handler,LV_EVENT_CLICKED,dev);
}
#endif

extern lv_style_t style;

void on_delete_scr_dev(struct my_lvgl_screen* scr)
{
	struct my_scr_dev* data=(struct my_scr_dev*)scr->userData;
	scr->userData=NULL;
	lv_obj_del_async(data->slider_pwm);
	lv_obj_del_async(data->lbl_status);
#ifdef _SEN_TABLE_	
	lv_obj_del_async(data->table_sensor);
#else
	destroy_uisensor(&data->bmp_ap);
	destroy_uisensor(&data->bmp_alt);
	destroy_uisensor(&data->bmp_temp);
	destroy_uisensor(&data->dht_humidity);
	destroy_uisensor(&data->dht_temp);
	destroy_uisensor(&data->ds18b20);
	lv_obj_del_async(data->lbl_cur);
	free_my_canvas(&data->canvas);
	//if(data->buf_draw_buf)
	//	free(data->buf_draw_buf);
#endif
	free(data);
}

static void update_table_row(lv_obj_t* table,int row,float value,const char* unit,struct sensor_data* sensor)
{
	lv_table_set_cell_value_fmt(table,row,1,"%.2f%s",value,unit);
	lv_table_set_cell_value_fmt(table,row,2,"%s:%s",sensor->dev,sensor->datetime);
}
enum sensor_row{
	sensor_row_ds18b20,
	sensor_row_dht_temp,
    sensor_row_dht_hum,
    sensor_row_bmp_ap,
    sensor_row_bmp_alt,
    sensor_row_bmp_temp,
};
void on_update_scr_dev(struct my_lvgl_screen* scr)
{
	struct my_scr_dev* data=(struct my_scr_dev*)scr->userData;
	update_status(data->lbl_status);
	pthread_mutex_lock(&mqttmutex);
	char buf[32];
	if(last_mqtt_data.mqtt_flag&sensor_flag_ds18b20)
	{
		#ifdef _SEN_TABLE_		
		update_table_row(data->table_sensor,sensor_row_ds18b20,
			last_mqtt_data.ds18b20,"",&last_mqtt_data);
		#else
		sprintf(buf,"%.2f℃",last_mqtt_data.ds18b20);
		lv_label_set_text(data->ds18b20.value,buf);
		sensor_update_value(last_mqtt_data.ds18b20,&data->ds18b20,&last_mqtt_data);
		#endif
	}
	if(last_mqtt_data.mqtt_flag&sensor_flag_dht_hum)
	{
		#ifdef _SEN_TABLE_
		update_table_row(data->table_sensor,sensor_row_dht_hum,
			last_mqtt_data.dht_hum,"%",&last_mqtt_data);
		#else
		sprintf(buf,"%.2f%%",last_mqtt_data.dht_hum);
		lv_label_set_text(data->dht_humidity.value,buf);
		sensor_update_value(last_mqtt_data.dht_hum,&data->dht_humidity,&last_mqtt_data);
		#endif
	}
	if(last_mqtt_data.mqtt_flag&sensor_flag_dht_temp)
	{
		#ifdef _SEN_TABLE_
		update_table_row(data->table_sensor,sensor_row_dht_temp,
			last_mqtt_data.dht_temp,"",&last_mqtt_data);
		#else
		sprintf(buf,"%.2f℃",last_mqtt_data.dht_temp);
		lv_label_set_text(data->dht_temp.value,buf);
		sensor_update_value(last_mqtt_data.dht_temp,&data->dht_temp,&last_mqtt_data);
		#endif
	}
	if(last_mqtt_data.mqtt_flag&sensor_flag_bmp_alt)
	{
		#ifdef _SEN_TABLE_
		update_table_row(data->table_sensor,sensor_row_bmp_alt,
			last_mqtt_data.bmp_alt,"m",&last_mqtt_data);
		#else
		sprintf(buf,"%.2fm",last_mqtt_data.bmp_alt);
		lv_label_set_text(data->bmp_alt.value,buf);
		sensor_update_value(last_mqtt_data.bmp_alt,&data->bmp_alt,&last_mqtt_data);
		#endif
	}
	if(last_mqtt_data.mqtt_flag&sensor_flag_bmp_ap)
	{
		#ifdef _SEN_TABLE_
		update_table_row(data->table_sensor,sensor_row_bmp_ap,
			last_mqtt_data.bmp_ap,"",&last_mqtt_data);
		#else
		sprintf(buf,"%.2f",last_mqtt_data.bmp_ap);
		lv_label_set_text(data->bmp_ap.value,buf);
		sensor_update_value(last_mqtt_data.bmp_ap,&data->bmp_ap,&last_mqtt_data);
		#endif
	}
	if(last_mqtt_data.mqtt_flag&sensor_flag_bmp_temp)
	{
		#ifdef _SEN_TABLE_
		update_table_row(data->table_sensor,sensor_row_bmp_temp,
			last_mqtt_data.bmp_temp,"",&last_mqtt_data);
		#else
		sprintf(buf,"%.2f℃",last_mqtt_data.bmp_temp);
		lv_label_set_text(data->bmp_temp.value,buf);
		sensor_update_value(last_mqtt_data.bmp_temp,&data->bmp_temp,&last_mqtt_data);
		#endif
	}
	if(data->cur_update_flag&&data->cur_sensor&&
		(data->cur_update_flag&last_mqtt_data.mqtt_flag))
	{
		on_update_canvas(data->cur_sensor,&data->canvas);
		on_cur_sensor_changed(data,&data->canvas);
	}
	last_mqtt_data.mqtt_flag=0;
	pthread_mutex_unlock(&mqttmutex);
	
}

static void slider_event_cb(lv_event_t* e)
{
	lv_obj_t* slider=lv_event_get_target(e);
	int value=(int)lv_slider_get_value(slider);
	my_mqtt_publish("/client/set_moto_pwm",&value,sizeof(int));
}

#ifdef _SEN_TABLE_
void init_table_row(lv_obj_t* table)
{
	lv_table_set_cell_value(table,sensor_row_ds18b20,0,"ds18b20");
	lv_table_set_cell_value(table,sensor_row_dht_temp,0,"dht_temp");
    lv_table_set_cell_value(table,sensor_row_dht_hum,0,"dht_hum");
    lv_table_set_cell_value(table,sensor_row_bmp_ap,0,"bmp_ap");
    lv_table_set_cell_value(table,sensor_row_bmp_alt,0,"bmp_alt");
    lv_table_set_cell_value(table,sensor_row_bmp_temp,0,"bmp_temp");
}
#else
void init_sensor(struct my_scr_dev* data)
{
	lv_label_set_text(data->ds18b20.name,"ds18b20");
	lv_label_set_text(data->bmp_alt.name,"bmp_alt");
	lv_label_set_text(data->bmp_ap.name,"bmp_ap");
	lv_label_set_text(data->bmp_temp.name,"bmp_temp");
	lv_label_set_text(data->dht_humidity.name,"dht_humidity");
	lv_label_set_text(data->dht_temp.name,"dht_temp");
}
#endif

static void create_uisensor(struct ui_sensor* ui,lv_obj_t* parent,struct my_scr_dev* dev)
{
	memset(ui,0,sizeof(struct ui_sensor));
	ui->layout=lv_obj_create(parent);
	lv_obj_set_flex_flow(ui->layout,LV_FLEX_FLOW_COLUMN);
	ui->value=lv_label_create(ui->layout);
	lv_obj_set_size(ui->value,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	ui->name=lv_label_create(ui->layout);
	lv_obj_set_size(ui->name,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
	lv_obj_add_style(ui->value,&style,LV_PART_MAIN);
	lv_obj_add_style(ui->name,&style,LV_PART_MAIN);
	
	//lv_obj_add_flag(ui->layout,LV_OBJ_FLAG_EVENT_BUBBLE);
	//lv_obj_add_style(ui->layout,&dev->sensor_normal_style,LV_PART_INDICATOR);
	//lv_obj_add_style(ui->layout,&dev->sensor_check_style,LV_PART_INDICATOR|LV_STATE_CHECKED);
}

struct my_lvgl_screen* create_screen_dev_main()
{
	struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_dev* data=(struct my_scr_dev*)malloc(sizeof(struct my_scr_dev));

	ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;


	data->lbl_pwm=lv_label_create(ret->screen);
	lv_label_set_text(data->lbl_pwm,"风扇转速");
	lv_obj_set_size(data->lbl_pwm,90,20);
	lv_obj_set_pos(data->lbl_pwm,5,520);
	lv_obj_add_style(data->lbl_pwm,&style,0);
	data->slider_pwm=lv_slider_create(ret->screen);
	lv_obj_set_size(data->slider_pwm,700,20);
	lv_obj_set_pos(data->slider_pwm,100,520);
	lv_obj_add_event_cb(data->slider_pwm,slider_event_cb,LV_EVENT_VALUE_CHANGED,ret);

	data->lbl_status=create_status_bar(ret->screen);
	//lv_obj_set_pos(data->lbl_cur,5,270);
	//lv_obj_set_size(data->lbl_cur,100,280);
	lv_obj_set_size(data->lbl_status,SCREEN_WIDTH-100,30);
	lv_obj_set_pos(data->lbl_status,5,560);

	lv_obj_set_size(ret->btn_back,90,40);
	lv_obj_set_pos(ret->btn_back,SCREEN_WIDTH-100,550);

#ifdef _SEN_TABLE_
	data->table_sensor=lv_table_create(ret->screen);
	lv_obj_set_size(data->table_sensor,500,400);
	lv_obj_set_pos(data->table_sensor,10,10);
	lv_table_set_column_count(data->table_sensor,3);
	lv_table_set_row_count(data->table_sensor,6);
	init_table_row(data->table_sensor);
	lv_table_set_col_width(data->table_sensor,1,120);
	lv_table_set_col_width(data->table_sensor,2,220);
#else
	//lv_style_init(&data->sensor_normal_style);
	//lv_style_set_bg_opa(&data->sensor_normal_style,0);
	//lv_style_set_bg_color(&data->sensor_normal_style,lv_color_hex3(0xfff));
	//lv_style_init(&data->sensor_check_style);
	//lv_style_set_bg_opa(&data->sensor_normal_style,0x7f);
	//lv_style_set_bg_color(&data->sensor_check_style,lv_color_hex3(0x00a));
	create_ui_sensor(data,ret->screen);
	init_sensor(data);
	data->cur_sensor=NULL;
	data->cur_update_flag=0;
	data->lbl_cur=lv_label_create(ret->screen);
	lv_obj_set_pos(data->lbl_cur,5,270);
	lv_obj_set_size(data->lbl_cur,100,235);
	init_my_canvas(&data->canvas,1014-105,235,ret->screen);
	lv_obj_set_pos(data->canvas.canvas,105,270);
#endif
	//create_uisensor(&data->ds18b20,ret->screen);
	//lv_obj_set_size(data->ds18b20.layout,200,90);
	//lv_obj_set_pos(data->ds18b20.layout,520,10);
	

	ret->delete_cb=on_delete_scr_dev;
	ret->update_cb=on_update_scr_dev;
	ret->userData=data;
	return ret;
}


