/*
 * gui.c
 *
 *  Created on: Jun 25, 2024
 *      Author: wang
 */




#include "lvgl/lvgl.h"
#include "stock.h"
#include "lvgl/src/libs/freetype/lv_freetype.h"
#include "gui.h"
#include "pthread.h"
#include "net.h"
#include "mqtt.h"
#include "lvgl/src/misc/lv_types.h"
//lv_obj_t* scr1_table_stock;
//lv_obj_t* scr2_canvas;

//extern int stock_updated;
//extern int stock_count;
extern struct my_lvgl_screen* cur_scr;
lv_font_t * font;

void init_font()
{
	//lv_freetype_init(8,0,1000);
	lv_freetype_init(1000);
	//
	font= lv_freetype_font_create(
			 //"/home/wang/lv_port_pc_eclipse/bin/Lato-Regular.ttf",
			 "/mnt/UDISK/HYSongYunLangHeiW-1.ttf",
			 LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
			 18,LV_FREETYPE_FONT_STYLE_NORMAL);

	if(font==NULL)
	{
		 printf("font error\n");
	}
	lv_style_init(&style);
	lv_style_set_text_font(&style, font);
}

void my_screen_load(struct my_lvgl_screen* scr)
{
	cur_scr=scr;
	lv_screen_load_anim(cur_scr->screen,LV_SCR_LOAD_ANIM_MOVE_RIGHT,200,10,true);
}
void delete_scr_default(lv_event_t* e)
{
	struct my_lvgl_screen* scr=(struct my_lvgl_screen*)e->user_data;
	printf("scr delete...\n");
	scr->delete_cb(scr);
	if(scr->btn_back)
		lv_obj_del_async(scr->btn_back);
	free(scr);
	printf("scr deleted\n");
}
void clear_lv_list(lv_obj_t* list)
{
	lv_obj_clean(list);

    /*uint32_t cnt=lv_obj_get_child_count(list);
    uint32_t i;
    lv_obj_t** children=(lv_obj_t**)malloc(sizeof(lv_obj_t*)*cnt);
    printf("%s:%d\n",__FUNCTION__,cnt);
	
    for(i=0;i<cnt;i++)
    {
        children[i]=lv_obj_get_child(list,i);
    }
    for(i=0;i<cnt;i++)
    {
        lv_obj_del(children[i]);
    }
    free(children);
	*/
}

struct my_lvgl_screen* create_default_screen()
{
	struct my_lvgl_screen* ret=(struct my_lvgl_screen* )malloc(sizeof(struct my_lvgl_screen));
	memset(ret,0,sizeof(struct my_lvgl_screen));
	ret->screen=lv_obj_create(NULL);

	lv_obj_add_event_cb(ret->screen,delete_scr_default,LV_EVENT_DELETE,ret);
	return ret;
}

lv_obj_t* create_button_with_label(lv_obj_t* parent,const char* title,lv_event_cb_t cb,void* cb_data)
{
	lv_obj_t* btn=lv_button_create(parent);
	lv_obj_add_event_cb(btn,cb,LV_EVENT_CLICKED,cb_data);
	lv_obj_t* lbl=lv_label_create(btn);
	lv_label_set_text(lbl,title);
	lv_obj_center(lbl);
	lv_obj_add_style(btn,&style,0);
	return btn;
}

lv_obj_t* create_status_bar(lv_obj_t* parent)
{
	lv_obj_t* lbl_status=lv_label_create(parent);
	lv_obj_set_size(lbl_status,775,25);
	lv_obj_set_pos(lbl_status,0,455);
	lv_obj_add_style(lbl_status,&style,0);
	return lbl_status;
}

void init_screen()
{
	cur_scr=create_main_scr();
	lv_screen_load(cur_scr->screen);
}
void update_screen()
{
	cur_scr->update_cb(cur_scr);
}

