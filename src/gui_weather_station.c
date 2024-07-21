#include "gui.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thread.h"
#include "weather_station.h"
#include "net.h"

#define _PROV_LIST_

extern lv_style_t style;

#define UPDATE_PROVINCE 0x01
#define UPDATE_CITY     0x02

struct my_scr_weather_station
{
    #ifdef _PROV_LIST_
    lv_obj_t* list_province;
    #else
    lv_obj_t* roller_province;
    #endif
    lv_obj_t* list_city;
#ifdef _PROV_LIST_    
    
#else
    char* roller_buf;
#endif
    struct station_key_value_arr province;
    struct station_key_value_arr city;

    uint32_t thread_task;
    struct station_key_value* cur_province;
    struct my_thread updater;
    uint32_t update_flag;
};

const char* station_cfg="./station.cfg";
//显示数组到lv_list
static void gui_update_list_arr(struct station_key_value_arr* arr,lv_obj_t* list,struct my_scr_weather_station* scr,lv_event_cb_t cb);
//数据更新到UI，省
static void gui_update_list_province(struct station_key_value_arr* province,lv_obj_t* list,struct my_scr_weather_station* scr);
//数据更新到UI，城市
static void gui_update_list_city(struct station_key_value_arr* city,lv_obj_t* list,struct my_scr_weather_station* scr);

//加载省，并显示到UI
static int load_province(struct my_scr_weather_station* data);
//删除元件，释放相关内存，关闭更新线程，准备销毁窗口
void on_delete_scr_weather_station(struct my_lvgl_screen* scr)
{
	struct my_scr_weather_station* data=(struct my_scr_weather_station*)scr->userData;
    scr->userData=NULL;
#ifdef _PROV_LIST_    
    lv_obj_del_async(data->list_province);
#else
    lv_obj_del_async(data->roller_province);
#endif
    lv_obj_del_async(data->list_city);
    del_station_key_val_arr(&data->province);
    del_station_key_val_arr(&data->city);
#ifdef _PROV_LIST_    
#else    
    free(data->roller_buf);
    #endif
    my_wait_thread_exit(&data->updater);
    pthread_mutex_destroy(&data->updater.mutex);
    free(data);
}
//更新到UI，把线程更新的内容显示到List
void on_update_scr_weather_station(struct my_lvgl_screen* scr)
{
    struct my_scr_weather_station* data=(struct my_scr_weather_station*)scr->userData;
    if(0==pthread_mutex_trylock(&data->updater.mutex))
    {
        if(data->update_flag&UPDATE_CITY)
        {
            //pthread_mutex_lock(&data->updater.mutex);
            printf("update city=%d\n",data->city.cnt);
            gui_update_list_city(&data->city,data->list_city,data);
            printf("ui update city ok\n");
            data->update_flag=data->update_flag&(~UPDATE_CITY);        
        }
        if(data->update_flag&UPDATE_PROVINCE)
        {
#ifdef _PROV_LIST_
            gui_update_list_province(&data->province,data->list_province,data);
#else                    
                    update_roller_province(&data->province,data->roller_province,&data->roller_buf);
#endif
            data->update_flag=data->update_flag&(~UPDATE_PROVINCE); 
        }
        pthread_mutex_unlock(&data->updater.mutex);
    }
    else{
        printf("gui lock error\n");
    }
    
    //
}

const char* province_path="./province.json";

#ifdef _PROV_LIST_
static int load_city(const char* province,struct my_scr_weather_station* data);
void station_province_event_cb(lv_event_t* e)
{
    struct my_scr_weather_station* scr=(struct my_scr_weather_station*)e->user_data;
    lv_obj_t* obj=lv_event_get_target(e);
    struct station_key_value* pdata=(struct station_key_value*)lv_obj_get_user_data(obj);
    scr->cur_province=pdata;
    //printf(pdata->code);
    printf("province:%s\n",scr->cur_province->code);
    if(load_city(scr->cur_province->code,scr)<0)
    {
        scr->thread_task=2;
    }
}

static void gui_update_list_province(struct station_key_value_arr* data,lv_obj_t* list,struct my_scr_weather_station* scr)
{
    gui_update_list_arr(data,list,scr,station_province_event_cb);
    /*int i,buf_len=0;
    lv_obj_t* btn;
    for(i=0;i<data->cnt;i++)
    {
        btn=lv_list_add_button(list,NULL,data->val[i].name);
        lv_obj_add_event_cb(btn,station_province_event_cb,LV_EVENT_CLICKED,scr);
        lv_obj_set_user_data(btn,(void*)(data->val+i));
        //buf_len+=strlen((data->val+i)->name)+1;
    }*/
}
#else
static void update_roller_province(struct station_key_value_arr* data,lv_obj_t* roller,char** roller_buf)
{
    int i,buf_len=0;
    if(*roller_buf)
        free(*roller_buf);
    for(i=0;i<data->cnt;i++)
        buf_len+=strlen(data->val[i].name)+1;
    *roller_buf=(char*)malloc(buf_len);
    memset(*roller_buf,0,buf_len);
    for(i=0;i<data->cnt;i++)
    {
        strcat(*roller_buf,data->val[i].name);
        if(i!=data->cnt-1)
            strcat(*roller_buf,"\n");
        //buf_len+=strlen((data->val+i)->name)+1;
    }
    lv_roller_set_options(roller,*roller_buf,LV_ROLLER_MODE_INFINITE);
}
#endif
static void gui_update_list_arr(struct station_key_value_arr* arr,lv_obj_t* list,struct my_scr_weather_station* scr,lv_event_cb_t cb)
{
    int i;
    lv_obj_t* btn;
    for(i=0;i<arr->cnt;i++)
    {
        btn=lv_list_add_button(list,NULL,arr->val[i].name);
        lv_obj_add_event_cb(btn,cb,LV_EVENT_CLICKED,scr);
        lv_obj_set_user_data(btn,(void*)(arr->val+i));
        //buf_len+=strlen((data->val+i)->name)+1;
    }
}
static int load_province(struct my_scr_weather_station* data)
{
    if(load_weather_station_province(&data->province,province_path)>0)
    {
#ifdef _PROV_LIST_
        gui_update_list_province(&data->province,data->list_province,data);
#else        
        update_roller_province(&data->province,data->roller_province,&data->roller_buf);
#endif
        return 1;
    }
    return -1;
}

//线程任务回调
int station_update(void* p,pthread_mutex_t* mutex )
{
    struct my_scr_weather_station* data=(struct my_scr_weather_station*)p;
    //printf("%s:%d\n",__FUNCTION__,data->thread_task);
    if(data->thread_task)
    {
        struct resp_buf buf={NULL,0};
        switch (data->thread_task)
        {
        case UPDATE_PROVINCE:
        //http://www.nmc.cn/rest/province/all
            request_url("www.nmc.cn","80","/rest/province/all",&buf);
            if(buf.buf)
            {
                save_to_file(buf.buf,buf.len,province_path);
                pthread_mutex_lock(mutex);
                del_station_key_val_arr(&data->province);
                if(load_weather_station_province_buf(&data->province,buf.buf,buf.len)>0)
                {

                    data->update_flag|=UPDATE_PROVINCE;

                }
                else
                    printf("update error\n");
                pthread_mutex_unlock(mutex);
                free(buf.buf);
            }
            data->thread_task=0;
            break;
        case UPDATE_CITY:
        {
            char path[64];

            sprintf(path,"/rest/province/%s",data->cur_province->code);
            printf("request %s\n",path);
            request_url("www.nmc.cn","80",path,&buf);
            if(buf.buf)
            {
                sprintf(path,"./province_%s.json",data->cur_province->code);
                //sprintf(path,"/rest/province/%s",data->cur_province->code);
                save_to_file(buf.buf,buf.len,path);
                pthread_mutex_lock(mutex);
                del_station_key_val_arr(&data->city);
                if(load_weather_station_city_buf(&data->city,buf.buf,buf.len)>0)
                {
                    data->update_flag|=UPDATE_CITY;
                     //update_roller_province(&data->province,data->roller_province,&data->roller_buf);
                }
                else
                     printf("update error\n");
                pthread_mutex_unlock(mutex);
                free(buf.buf);
            }
            printf("request finished\n");
            data->thread_task=0;
        }
            break;
        default:
            break;
        }
    }
    return 10;
}

void station_city_event_cb(lv_event_t* e)
{
    struct my_scr_weather_station* scr=(struct my_scr_weather_station*)e->user_data;
    lv_obj_t* obj=lv_event_get_target(e);
    struct station_key_value* city=(struct station_key_value*)lv_obj_get_user_data(obj);
    printf("city:%s,%s\n",city->name,city->code);
    save_station(city,station_cfg);
}
static void gui_update_list_city(struct station_key_value_arr* city,lv_obj_t* list,struct my_scr_weather_station* scr)
{
    clear_lv_list(list);
    gui_update_list_arr(city,list,scr,station_city_event_cb);
    /*uint32_t i;
    lv_obj_t* btn;
    clear_lv_list(list);
    printf("%s:%d\n",__FUNCTION__,city->cnt);
    for(i=0;i<city->cnt;i++)
    {
        //printf("update city=%s\n",city->val[i].name);
        btn=lv_list_add_btn(list,NULL,city->val[i].name);
        lv_obj_add_event_cb(btn,station_city_event_cb,LV_EVENT_CLICKED,scr);
        lv_obj_set_user_data(btn,(void*)(city->val+i));
        //lv_list_add_text(list,city->val[i].name);
    }*/
}
static int load_city(const char* province,struct my_scr_weather_station* data)
{
    char city_path[64];
    sprintf(city_path,"../bin/province_%s.json",province);
    if(load_weather_station_city(&data->city,city_path)>0)
    {
        gui_update_list_city(&data->city,data->list_city,data);
        //update_roller_province(&data->province,data->roller_province,&data->roller_buf);
        return 1;
    }
    return -1;
}

#ifdef _PROV_LIST_
#else
void roller_province_cb(lv_event_t* e)
{
    struct my_scr_weather_station* data=(struct my_scr_weather_station*)e->user_data;
    uint32_t index=lv_roller_get_selected(lv_event_get_target(e));
    if(index<data->province.cnt)
    {
        data->cur_province=data->province.val+index;
        printf("province:%s\n",data->cur_province->code);
        if(load_city(data->cur_province->code,data)<0)
        {
            data->thread_task=2;
        }
    }
}
#endif

struct my_lvgl_screen* create_screen_weather_station_main()
{
    struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_weather_station* data=(struct my_scr_weather_station*)malloc(sizeof(struct my_scr_weather_station));
    
    static int32_t col_dec[]={360,360,30,LV_GRID_TEMPLATE_LAST};
	static int32_t row_dec[]={400,60,LV_GRID_TEMPLATE_LAST};
    
    lv_obj_set_style_bg_color(ret->screen,lv_color_hex(0x1882CD),LV_PART_MAIN);
    lv_obj_set_style_pad_all(ret->screen,5,LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(ret->screen,col_dec,0);
	lv_obj_set_style_grid_row_dsc_array(ret->screen,row_dec,0);
	lv_obj_set_layout(ret->screen,LV_LAYOUT_GRID);
#ifdef _PROV_LIST_
#else
    data->roller_buf=NULL;
#endif
    data->cur_province=NULL;
    ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_screen_weather_main;
    lv_obj_set_grid_cell(ret->btn_back,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,1,1);

#ifdef _PROV_LIST_
    data->list_province=lv_list_create(ret->screen);
    //lv_obj_add_event_cb(data->list_province,roller_province_cb,LV_EVENT_VALUE_CHANGED,data);
    lv_obj_set_grid_cell(data->list_province,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,1);
    lv_obj_add_style(data->list_province,&style,0);
#else
    data->roller_province=lv_roller_create(ret->screen);
    lv_obj_add_event_cb(data->roller_province,roller_province_cb,LV_EVENT_VALUE_CHANGED,data);
    lv_obj_set_grid_cell(data->roller_province,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,1);
    lv_obj_add_style(data->roller_province,&style,0);
#endif
    data->list_city=lv_list_create(ret->screen);
    lv_obj_set_grid_cell(data->list_city,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,0,1);
    
    lv_obj_add_style(data->list_city,&style,0);
    /*static int32_t col_dec[]={155,155,155,200,LV_GRID_TEMPLATE_LAST};
	static int32_t row_dec[]={55,55,55,55,55,55,55,LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_bg_color(ret->screen,lv_color_hex(0x1882CD),LV_PART_MAIN);
    lv_obj_set_style_pad_all(ret->screen,5,LV_PART_MAIN);
	memset(&data->weather,0,sizeof(struct Weather));
    weather_init_font(data);
    
    weather_parser_file("../bin/weather.json",&data->weather);
	ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;
    lv_obj_set_grid_cell(ret->btn_back,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,5,1);

    data->predict=create_button_with_label(ret->screen,"predict",on_event_predict_cb,ret);
    lv_obj_set_grid_cell(data->predict,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,5,1);

	lv_obj_set_style_grid_column_dsc_array(ret->screen,col_dec,0);
	lv_obj_set_style_grid_row_dsc_array(ret->screen,row_dec,0);
	lv_obj_set_layout(ret->screen,LV_LAYOUT_GRID);

    create_weather_gui_weather(ret->screen,data,&data->gui_weather);
    create_weather_gui_today(ret->screen,data,&data->gui_today);
    
    ret->delete_cb=on_delete_scr_weather;
	ret->update_cb=on_update_scr_weather;
    data->lbl_status=create_status_bar(ret->screen);
    lv_obj_set_grid_cell(data->lbl_status,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,6,1);
    data->weather_info=create_button_with_label(ret->screen,"station",weather_station_event_cb,ret);
    lv_obj_set_grid_cell(data->weather_info,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_STRETCH,6,1);

    my_thread_init(&data->thread_update);
    data->thread_update.scr=ret;
    data->thread_update.loop=weather_update;
    data->weather_flag=0;
    my_create_thread(&data->thread_update);*/

    memset(&data->city,0,sizeof(struct station_key_value_arr));
    memset(&data->province,0,sizeof(struct station_key_value_arr));

    my_thread_init(&data->updater);

    data->updater.scr=ret;
    data->updater.loop=station_update;
    data->update_flag=0;
    data->thread_task=0;
    
    if(load_province(data)<0)
        data->thread_task=UPDATE_PROVINCE;

    ret->delete_cb=on_delete_scr_weather_station;
	ret->update_cb=on_update_scr_weather_station;
	ret->userData=data;
    my_create_thread(&data->updater);
	return ret;
}
