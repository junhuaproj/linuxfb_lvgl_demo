/*
 * gui_stock.c
 *
 *  Created on: Jun 28, 2024
 *      Author: wang
 */

#include <string.h>

#include "pthread.h"
#include "gui.h"
#include "net.h"
#include <stdio.h>
#include "thread.h"
#include <stdlib.h>
extern int stock_count;
struct my_scr_stock_main
{
	lv_obj_t* table;
	lv_obj_t* lbl_status;
	pthread_t update_id;
};

#define CANVAS_WIDTH	1014
#define CANVAS_HEIGHT	500

struct my_lvgl_screen* create_stock_screen_detail(const char* code);
void* updateListThread(void* param)
{
	struct my_lvgl_screen* scr=(struct my_lvgl_screen*)param;
	int wait_update=10;
	while(scr->userData)
	{
		usleep(10000);
		wait_update--;
		if(wait_update<=0)
		{
			struct resp_buf buf={NULL,0};
			request_url("192.168.1.5","5000","/stock/price/000997,000561,601939,600016,600089",&buf);
			if(buf.buf)
			{
				main_parser_list(buf.buf,buf.len,NULL);
				free(buf.buf);
			}
			//read_cpu_stat();
			//read_cpu_infos();
			//read_mem_info();
			wait_update=8000;
		}
	}
	printf("thread exit line:%d.\n",__LINE__);
	pthread_exit(NULL);
	return NULL;
}


void on_delete_scr_stock_main(struct my_lvgl_screen* scr)
{
	struct my_scr_stock_main* data=(struct my_scr_stock_main*)scr->userData;
	scr->userData=NULL;
	printf("delete stock main...\n");
	lv_obj_del_async(data->table);
	lv_obj_del_async(data->lbl_status);
	pthread_join(data->update_id,NULL);
	free(data);
}

void on_update_scr_stock_main(struct my_lvgl_screen* scr)
{
	int i,colCnt,col;
	struct my_scr_stock_main* data=(struct my_scr_stock_main*)scr->userData;
	update_status(data->lbl_status);
	if(!stock_updated)
		return;


	stock_updated=0;
	if(stocks==NULL)
	{
		lv_table_set_row_count(data->table,0);
		return;
	}
	printf("update_scr_main..\n");
	colCnt=stocks[0].count;
	//多余的不显示
	if(colCnt>8)
		colCnt=8;
	
	lv_table_set_col_cnt(data->table,colCnt);
	lv_table_set_row_count(data->table,stock_count+1);
			//lv_table_set_column_width()
	lv_table_set_cell_value(data->table,0,0,"编号");
	lv_table_set_cell_value(data->table,0,1,"名称");
	lv_table_set_cell_value(data->table,0,2,"价格");
	lv_table_set_cell_value(data->table,0,3,"涨跌幅");
	lv_table_set_cell_value(data->table,0,4,"涨跌额");
	lv_table_set_cell_value(data->table,0,5,"成交量");
	lv_table_set_cell_value(data->table,0,6,"现手");
	lv_table_set_cell_value(data->table,0,7,"金额");
	for(i=0;i<stock_count;i++)
	{
		printf("stock %s\n",stocks[i].values[1]);
		for(col=0;col<colCnt&&col<stocks[i].count;col++)
		{
			lv_table_set_cell_value(data->table,i+1,col,stocks[i].values[col]);
			//if(stocks[i].values[3][0]=='-')
			//	lv_table_set_cell_type(data->table,i+1,col,LV_TABLE_PART_CELL2);
			//else
			//	lv_table_set_cell_type(data->table,i+1,col,LV_TABLE_PART_CELL3);
		}
	}
	lv_obj_set_style_pad_ver(data->table,20,LV_PART_ITEMS);
	//lv_obj_set_style_pad_hor(data->table,5,LV_PART_ITEMS);
	//lv_obj_set_style_pad_gap(scr1_table_stock,0,_LV_STYLE_STATE_CMP_SAME);
	for(i=2;i<colCnt;i++)
		lv_table_set_column_width(data->table,0,65);
	lv_table_set_column_width(data->table,0,130);
	lv_table_set_column_width(data->table,1,100);
}

static void table_main_event_cb(lv_event_t* e)
{
	struct my_scr_stock_main* scr=(struct my_scr_stock_main*)e->user_data;
	//lv_obj_t* obj=lv_event_get_current_target(e);
	uint32_t row=-1;
	uint32_t col=-1;
	
	lv_table_get_selected_cell(scr->table,&row,&col);

	printf("click r=%u,c=%u\n",row,col);
	if(col==LV_TABLE_CELL_NONE)
		return;
	const char* value=lv_table_get_cell_value(scr->table,row,0);

	struct my_lvgl_screen* newScr= create_stock_screen_detail(value);
	my_screen_load(newScr);
	//lv_screen_load_anim(newScr->screen,LV_SCR_LOAD_ANIM_MOVE_LEFT,100,10,true);
}

static void draw_event_cb(lv_event_t * e)
{	
	lv_obj_t* table=lv_event_get_target(e);
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = draw_task->draw_dsc;
    /*If the cells are drawn...*/
    if(base_dsc->part == LV_PART_ITEMS) {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        /*Make the texts in the first cell center aligned*/
        if(row == 0) {
            lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if(label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
				//label_draw_dsc->color=lv_palette_main(LV_PALETTE_RED);
            }
            lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if(fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), fill_draw_dsc->color, LV_OPA_20);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
		else if(col>1)
		{
			const char* for_color=lv_table_get_cell_value(table,row,4);
			lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if(label_draw_dsc) {
				if(for_color[0]=='+')
					label_draw_dsc->color=lv_palette_main(LV_PALETTE_RED);
				else if(for_color[0]=='-')
					label_draw_dsc->color=lv_palette_main(LV_PALETTE_GREEN);
				else
					label_draw_dsc->color=lv_color_hex3(0);

            }
		}

        /*Make every 2nd row grayish*/
        if((row != 0 && row % 2) == 0) {
            lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if(fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fill_draw_dsc->color, LV_OPA_10);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
    }
}
struct my_lvgl_screen* create_screen_stock_main()
{
	struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_stock_main* data=(struct my_scr_stock_main*)malloc(sizeof(struct my_scr_stock_main));

	ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;
	data->table=lv_table_create(ret->screen);
	lv_obj_set_style_pad_right(data->table,5,LV_PART_ITEMS);
    lv_obj_set_style_pad_left(data->table,5,LV_PART_ITEMS);

	//lv_obj_set_style_margin_right(data->table,0,LV_PART_ITEMS);
    //lv_obj_set_style_margin_left(data->table,0,LV_PART_ITEMS);

	//lv_obj_set_style_margin_right(data->table,0,LV_PART_ITEMS);
	lv_obj_add_event_cb(data->table, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(data->table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	//lv_obj_set_local_style_prop(data->table, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, LV_COLOR_GREEN);
	//lv_obj_set_style_local_text_color(data->table, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, LV_COLOR_RED);

	lv_obj_set_size(data->table,SCREEN_WIDTH-10,540);
	lv_obj_set_pos(data->table,5,5);
	lv_obj_add_style(data->table,&style,0);

	data->lbl_status=create_status_bar(ret->screen);

	lv_obj_set_size(data->lbl_status,SCREEN_WIDTH-100,30);
	lv_obj_set_pos(data->lbl_status,5,560);

	lv_obj_set_size(ret->btn_back,90,40);
	lv_obj_set_pos(ret->btn_back,SCREEN_WIDTH-100,550);

	lv_obj_add_event_cb(data->table,table_main_event_cb,LV_EVENT_PRESSED,data);
	printf("thread start...\n");
	pthread_create(&data->update_id,NULL,updateListThread,ret);


	ret->delete_cb=on_delete_scr_stock_main;
	ret->update_cb=on_update_scr_stock_main;
	ret->userData=data;
	return ret;
}

////////////////////////////////////////////////////////


struct my_scr_stock_detail
{
	lv_obj_t* title;
	lv_obj_t* today;
	lv_obj_t* day5;
	lv_obj_t* layout_day;
	lv_obj_t* canvas;
	lv_obj_t* stock_info;
	lv_obj_t* lbl_status;
	lv_obj_t* back;
	char* code;
	pthread_t update_id;

	lv_style_t radio_style;
	lv_style_t radio_style_chk;
	lv_style_t layout_style;
	pthread_mutex_t stock_mutex;
	struct stock_day_minute day_minute;
};

void* updateMinuteThread(void* param)
{
	struct my_lvgl_screen* scr=(struct my_lvgl_screen*)param;
	int wait_update=10;
	char url[128];
	struct my_scr_stock_detail* data=(struct my_scr_stock_detail*)scr->userData;
	update_status(data->lbl_status);
	while(scr->userData)
	{
		sleep(1);
		wait_update--;
		if(wait_update<=0)
		{
			int type=(int)lv_obj_get_user_data(data->layout_day);
			struct resp_buf buf={NULL,0};
			if(type==0)
			{
				//http://192.168.1.5:5000/stock/min/line/20240625/301448
				sprintf(url,"/stock/min/line/20240715/%s",data->code+3);
			}
			else{
				//sprintf(url,"/stock/mult_min/3/%s",data->code+3);
				sprintf(url,"/stock/min/line/20240715/%s",data->code+3);
			}
			printf("url:%s\n",url);
			
			request_url("192.168.1.5","5000",url,&buf);
			if(buf.buf)
			{
				pthread_mutex_lock(&data->stock_mutex);
				detail_parser_minute(&data->day_minute,buf.buf,buf.len,data,type);
				pthread_mutex_unlock(&data->stock_mutex);
				free(buf.buf);
			}
			wait_update=30;
		}
	}
	printf("thread exit line:%d.\n",__LINE__);
	pthread_exit(NULL);
	return NULL;
}
void on_delete_scr_stock_detail(struct my_lvgl_screen* scr)
{
	struct my_scr_stock_detail* data=(struct my_scr_stock_detail*)scr->userData;
	scr->userData=NULL;
	lv_obj_del_async(data->canvas);
	lv_obj_del_async(data->lbl_status);
	lv_obj_del_async(data->back);
	lv_obj_del_async(data->title);
	lv_obj_del_async(data->today);
	lv_obj_del_async(data->day5);
	lv_obj_del_async(data->layout_day);
	lv_style_reset(&data->radio_style);
	lv_style_reset(&data->radio_style_chk);
	lv_style_reset(&data->layout_style);
	if(data->code)
		free(data->code);
	free(data);
	pthread_join(data->update_id,NULL);
	pthread_mutex_destroy(&data->stock_mutex);
	clear_day_minute_line(&data->day_minute);

	/*if(data->day_minute.days)
	{
		for(int i=0;i<data->day_minute.cnt;i++)
		{
			deinit_minute_line(data->day_minute.days+i);
		}
		free(data->day_minute);
	}*/
	//deinit_minute_line();
}
struct minute_line_arg
{
	float lowPrice;
	float highPrice;
	float lowVol;
	float highVol;
	float rate;
	float item_width;
	int minute_cnt;
};

void get_day_minute_arg(struct stock_day_minute* days,struct minute_line_arg* arg)
{
	struct stock_minute_trade* pminute;
	struct stock_minute* minute;
	int x=0;
	arg->minute_cnt=0;
	arg->lowPrice=__FLT_MAX__;
	arg->highPrice=__FLT_MIN__;
	
	for(int i=0;i<days->cnt;i++)
	{
		minute=days->days+i;
		pminute=minute->minutes;
		for(x=0;x<minute->min_count;x++)//&&pminute[x].price!=0
		{
			if(pminute->price==0)
			{
				printf("day %d, minute idx:%d\n",i,x);
				continue;
			}
			if(arg->highPrice<pminute->price)
				arg->highPrice=pminute->price;
			if(arg->lowPrice>pminute->price)
				arg->lowPrice=pminute->price;

			if(arg->highVol<pminute->vol)
				arg->highVol=pminute->vol;
			if(arg->lowVol>pminute->vol)
				arg->lowVol=pminute->vol;
			pminute++;
			arg->minute_cnt++;
		}
		printf("day %d,count:%d\n",i,x);
	}
}

static void canvas_update(lv_obj_t* canvas,struct stock_day_minute* days)
{
	uint32_t x,y,i;
	//float high=0,low=999999;
	const int height=300;
	const int top=5,left=5;
	int prevx=0,prevy=0;
	//float rate;
	lv_layer_t layer;
	lv_draw_line_dsc_t dsc;
	struct minute_line_arg arg;
	struct stock_minute* minute;
	struct stock_minute_trade* pminute;
	
	lv_canvas_fill_bg(canvas,lv_palette_main(LV_PALETTE_BLUE),LV_OPA_COVER);
	get_day_minute_arg(days,&arg);
	arg.item_width=2;//lv_obj_get_width(canvas)/arg.minute_cnt;
	arg.rate=height/(arg.highPrice-arg.lowPrice);
	//rate=height/(high-low);
	printf("price range:%.2f\n",arg.rate);
	
	lv_canvas_init_layer(canvas,&layer);
	lv_draw_line_dsc_init(&dsc);
	dsc.color=lv_palette_main(LV_PALETTE_RED);
	dsc.width=1;

	for(i=0;i<days->cnt;i++)
	{
		minute=days->days+i;
		pminute=minute->minutes;
		for(x=0;x<minute->min_count;x++)//pminute[x].price!=0
		{
			if(pminute->price==0)
			{
				continue;
			}
			
			y=top+height-(pminute->price-arg.lowPrice)*arg.rate;
			//printf("height:%.2f\n",rate);
			if(prevy==0)
			{
				prevy=y;
				prevx=left;
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
			pminute++;
		}
	}
	const int vol_height=480-height-top;
	dsc.p1.x=left;
	dsc.p2.y=dsc.p1.y=top+height;
	dsc.p2.x=CANVAS_WIDTH-left*2;//left+days->cnt*arg.item_width;
	dsc.color=lv_color_hex(0x00000);
	lv_draw_line(&layer,&dsc);

	arg.rate=vol_height/(arg.highVol-arg.lowVol);
	prevx=left;
	for(i=0;i<days->cnt;i++)
	{
		minute=days->days+i;
		pminute=minute->minutes;
		for(x=0;x<minute->min_count;x++)//pminute[x].price!=0
		{
			if(pminute->price==0)
			{
				continue;
			}
			
			y=top+480-(pminute->vol-arg.lowVol)*arg.rate;
			//printf("height:%.2f\n",rate);
			prevx=prevx+arg.item_width;
				dsc.p1.x=prevx;
				dsc.p1.y=top+480;
				dsc.p2.x=dsc.p1.x;//prevx+arg.item_width;
				dsc.p2.y=y;
				lv_draw_line(&layer,&dsc);
				//prevy=y;
				//prevx=dsc.p2.x;
			
			//lv_canvas_set_px(data->canvas,x,y,lv_palette_main(LV_PALETTE_RED),LV_OPA_20);
			pminute++;
		}
	}

	lv_canvas_finish_layer(canvas,&layer);
}

void on_update_scr_stock_detail(struct my_lvgl_screen* scr)
{
	struct my_scr_stock_detail* data=(struct my_scr_stock_detail*)scr->userData;
	if(minute_updated)
	{
		canvas_update(data->canvas,&data->day_minute);
		/*
		lv_canvas_fill_bg(data->canvas,lv_palette_main(LV_PALETTE_BLUE),LV_OPA_COVER);
		uint32_t x,y;
		float high=0,low=999999;
		const int height=180;
		const int top=5;
		int prevx=0,prevy=0;
		float rate;
		lv_layer_t layer;
		lv_draw_line_dsc_t dsc;
		struct stock_minute_trade* pminute=stock_minute_line->minutes;

		for(x=0;x<stock_minute_line->min_count&&pminute[0].price!=0;x++)
		{
			if(high<pminute->price)
				high=pminute->price;
			if(low>pminute->price)
				low=pminute->price;
			pminute++;
		}
		rate=height/(high-low);
		printf("price range:%.2f\n",rate);
		pminute=stock_minute_line->minutes;

		lv_canvas_init_layer(data->canvas,&layer);
		lv_draw_line_dsc_init(&dsc);
		dsc.color=lv_palette_main(LV_PALETTE_RED);
		dsc.width=2;

		for(x=0;x<stock_minute_line->min_count&&pminute[0].price!=0;x++)
		{
			y=top+height-(pminute->price-low)*rate;
			//printf("height:%.2f\n",rate);
			if(prevy==0)
			{
				prevy=y;
				prevx=x;
			}
			else
			{
				dsc.p1.x=prevx;
				dsc.p1.y=prevy;
				dsc.p2.x=x;
				dsc.p2.y=y;
				lv_draw_line(&layer,&dsc);
				prevy=y;
				prevx=x;
			}
			//lv_canvas_set_px(data->canvas,x,y,lv_palette_main(LV_PALETTE_RED),LV_OPA_20);
			pminute++;
		}
		lv_canvas_finish_layer(data->canvas,&layer);*/
		minute_updated=0;
	}
}
void scr2_delete_cb(lv_event_t* e)
{
	struct my_lvgl_screen* scr=(struct my_lvgl_screen*)e->user_data;
	scr->delete_cb(scr);
}

void btn_stock_detail_back_event_cb(lv_event_t* e)
{
	lv_event_code_t code=lv_event_get_code(e);
	if(code==LV_EVENT_CLICKED)
	{
		struct my_lvgl_screen* newScr= create_screen_stock_main();
		my_screen_load(newScr);
	}
}

void screen_detail_init_style(struct my_scr_stock_detail* detail)
{
	lv_style_init(&detail->radio_style);
	lv_style_set_radius(&detail->radio_style,LV_RADIUS_CIRCLE);

	lv_style_init(&detail->radio_style_chk);
	lv_style_set_bg_image_src(&detail->radio_style_chk,NULL);

	lv_style_init(&detail->layout_style);
	lv_style_set_bg_opa(&detail->layout_style,0);
	lv_style_set_pad_all(&detail->layout_style,3);
	lv_style_set_border_width(&detail->layout_style,0);
	//lv_style_set_pad_all(&detail->layout_style,0);
}

void radio_event_cb(lv_event_t* e)
{
	struct my_scr_stock_detail* data=(struct my_scr_stock_detail*)lv_event_get_user_data(e);

	lv_obj_t* layout=lv_event_get_current_target(e);
	lv_obj_t* cur=lv_event_get_target(e);
	int old_index=(int)lv_obj_get_user_data(layout);
	int new_index=lv_obj_get_index(cur);
	lv_obj_t* old=lv_obj_get_child(layout,old_index);
	if(cur==layout)return;
	lv_obj_remove_state(old,LV_STATE_CHECKED);
	lv_obj_add_state(cur,LV_STATE_CHECKED);
	old_index=new_index;
	lv_obj_set_user_data(layout,(void*)old_index);
	printf("check index=%d\n",old_index);
	
}

struct my_lvgl_screen* create_stock_screen_detail(const char* code)
{
	LV_DRAW_BUF_DEFINE(draw_buf,CANVAS_WIDTH,CANVAS_HEIGHT,LV_COLOR_FORMAT_RGB565);
	struct my_lvgl_screen* ret=create_default_screen();
	//init_minute_line();

	struct my_scr_stock_detail* data=(struct my_scr_stock_detail*)malloc(sizeof(struct my_scr_stock_detail));
	if(code)
	{
		data->code=(char*)malloc(strlen(code)+1);
		strcpy(data->code,code);
	}
	else
		data->code=NULL;
	memset(&data->day_minute,0,sizeof(struct stock_day_minute));
	
	screen_detail_init_style(data);

	pthread_mutex_init(&data->stock_mutex,NULL);
	data->layout_day=lv_obj_create(ret->screen);
	lv_obj_set_flex_flow(data->layout_day,LV_FLEX_FLOW_ROW);
	lv_obj_set_size(data->layout_day,240,30);
	lv_obj_set_pos(data->layout_day,700,0);
	//lv_obj_set_style_pad_all(data->layout_day,0,LV_PART_MAIN);
	lv_obj_set_style_margin_all(data->layout_day,0,LV_PART_MAIN);
	//lv_obj_set_style_bg_opa(data->layout_day,0,LV_PART_MAIN);
	lv_obj_add_style(data->layout_day,&data->layout_style,LV_PART_MAIN);

	lv_obj_set_style_bg_opa(data->layout_day,LV_OPA_0,0);

	data->title=lv_label_create(ret->screen);
	lv_label_set_text(data->title,code);

	data->today=lv_checkbox_create(data->layout_day);
	

	lv_checkbox_set_text(data->today,"today");
	lv_obj_add_flag(data->today,LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_add_style(data->today,&data->radio_style,LV_PART_INDICATOR);
	lv_obj_add_style(data->today,&data->radio_style_chk,LV_PART_INDICATOR|LV_STATE_CHECKED);

	data->day5=lv_checkbox_create(data->layout_day);
	lv_checkbox_set_text(data->day5,"day 5");
	lv_obj_add_flag(data->day5,LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_add_style(data->day5,&data->radio_style,LV_PART_INDICATOR);
	lv_obj_add_style(data->day5,&data->radio_style_chk,LV_PART_INDICATOR|LV_STATE_CHECKED);

	lv_obj_add_event_cb(data->layout_day,radio_event_cb,LV_EVENT_CLICKED,data);
	lv_obj_add_state(lv_obj_get_child(data->layout_day,0),LV_STATE_CHECKED);
	lv_obj_set_user_data(data->layout_day,(void*)0);

	lv_obj_set_size(data->title,600,30);
	lv_obj_set_pos(data->title,5,0);

	//lv_obj_set_size(data->today,90,30);
	//lv_obj_set_pos(data->today,605,0);
	//lv_obj_set_size(data->day5,90,30);
	//lv_obj_set_pos(data->day5,700,0);
	
	data->canvas=lv_canvas_create(ret->screen);
	lv_canvas_set_draw_buf(data->canvas,&draw_buf);
	lv_obj_set_size(data->canvas,CANVAS_WIDTH,CANVAS_HEIGHT);
	lv_obj_set_pos(data->canvas,5,40);
	lv_canvas_fill_bg(data->canvas,lv_palette_main(LV_PALETTE_BLUE),LV_OPA_COVER);

	//lv_obj_add_style(data->canvas,&style,0);
	data->lbl_status=create_status_bar(ret->screen);
	lv_obj_set_size(data->lbl_status,100,30);
	lv_obj_set_pos(data->lbl_status,0,CANVAS_HEIGHT+40);

	data->back=create_button_with_label(ret->screen,"返回",btn_stock_detail_back_event_cb,data);

	lv_obj_set_size(data->back,100,30);
	lv_obj_set_pos(data->back,890,5);


	//pthread_create(&data->update_id,NULL,updateMinuteThread,ret);
	ret->delete_cb=on_delete_scr_stock_detail;
	ret->update_cb=on_update_scr_stock_detail;
	ret->userData=data;
	pthread_create(&data->update_id,NULL,updateMinuteThread,ret);
	return ret;
}
