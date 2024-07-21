#include "gui.h"
#include "weather.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "thread.h"
#include "net.h"
#include "weather_station.h"
#include "mqtt.h"

#define DISP_BUF_LEN    256
struct my_scr_weather_weather
{
    lv_obj_t* temperature;
    lv_obj_t* icon;
    lv_obj_t* info;
    lv_obj_t* humidity;
    lv_obj_t* feelst;
    lv_obj_t* rain;
    lv_obj_t* wind;
};

struct my_scr_today
{
    lv_obj_t* yymm;
    //lv_obj_t* day;
    //lv_obj_t* weekday;
    lv_obj_t* time;
    lv_obj_t* temp;
    lv_obj_t* humidity;
    lv_obj_t* msg;
};
struct my_scr_weather
{
    struct my_scr_weather_weather gui_weather;
    struct my_scr_today gui_today;
    char disp_buf[DISP_BUF_LEN];
    
    lv_obj_t* clock;
    lv_obj_t * minute_hand;
    lv_obj_t * hour_hand;
    lv_point_precise_t minute_hand_points[2];
    //button,predict
    lv_obj_t* predict;
    lv_obj_t* weather_info;
    lv_obj_t* lbl_status;
    //lv_obj_t** lines;
//    lv_obj_t* menu;

    lv_font_t * font72;
    lv_font_t * font48;
    lv_font_t * font24;
    lv_font_t * font16;

    lv_style_t style72;
    lv_style_t style48;
    lv_style_t style24;
    lv_style_t style16;

    struct my_thread thread_update;
    uint32_t weather_flag;
    struct Weather weather;
};

extern lv_style_t style;
const char* weather_content="./weather.json";
const char* weekName[]={"周日","周一","周二","周三","周四","周五","周六"};
const char* getWeekName(int week)
{
    if(week>=0&&week<=6)
        return weekName[week];
        //printf("week %d\n",week);
    return weekName[0];
}
//#include "lvgl/src/misc/lv_ll.h"

void weather_init_font(struct my_scr_weather* data)
{
    /*lv_fs_drv_t ** drv;
    #define fsdrv_ll_p &(LV_GLOBAL_DEFAULT()->fsdrv_ll)
    //_lv_fs_init();
    printf("letter\n");
    _LV_LL_READ(fsdrv_ll_p, drv) {
        //if((*drv)->letter == letter) {
        //    return *drv;
       // }
       printf("letter=%d\n",(*drv)->letter);
    }*/
   
	data->font72= lv_binfont_create("A/mnt/UDISK/STXIHEI72.bin");

    data->font48= lv_binfont_create("A/mnt/UDISK/STXIHEI48.bin");
    
	data->font24= lv_freetype_font_create(
			 "/mnt/UDISK/HYSongYunLangHeiW-1.ttf",
			 LV_FREETYPE_FONT_RENDER_MODE_BITMAP,24,LV_FREETYPE_FONT_STYLE_NORMAL);
    data->font16= lv_freetype_font_create(
			 "/mnt/UDISK/HYSongYunLangHeiW-1.ttf",
			 LV_FREETYPE_FONT_RENDER_MODE_BITMAP,16,LV_FREETYPE_FONT_STYLE_NORMAL);
    printf("init freetype font.\n");
    if(data->font24==NULL)
        printf("font 24 null\n");
    if(data->font72==NULL)
        printf("font 72 null\n");
	lv_style_init(&data->style72);
	lv_style_set_text_font(&data->style72, data->font72);
    lv_style_set_text_align(&data->style72,LV_TEXT_ALIGN_CENTER);

    lv_style_init(&data->style48);
	lv_style_set_text_font(&data->style48, data->font48);
    lv_style_set_text_align(&data->style48,LV_TEXT_ALIGN_CENTER);

    lv_style_init(&data->style24);
	lv_style_set_text_font(&data->style24, data->font24);
    //lv_style_set_border_color(&data->style24,lv_palette_lighten(LV_PALETTE_RED,3));
    //lv_style_set_border_width(&data->style24,3);
    lv_style_set_text_align(&data->style24,LV_TEXT_ALIGN_CENTER);
    //lv_style_set_align(&data->style24,LV_ALIGN_CENTER);

    lv_style_init(&data->style16);
	lv_style_set_text_font(&data->style16, data->font16);
    lv_style_set_text_align(&data->style16,LV_TEXT_ALIGN_CENTER);
    lv_style_set_bg_color(&data->style16,lv_color_hex(0x38a2eD));
    lv_style_set_bg_opa(&data->style16,LV_OPA_50);
}

void del_gui_weather(struct my_scr_weather_weather* gui)
{
    lv_obj_del_async(gui->temperature);
    lv_obj_del_async(gui->icon);
    lv_obj_del_async(gui->info);
    lv_obj_del_async(gui->humidity);
    lv_obj_del_async(gui->feelst);
    lv_obj_del_async(gui->rain);
    lv_obj_del_async(gui->wind);
}
void del_gui_today(struct my_scr_today* gui)
{
    //lv_obj_del_async(gui->day);
    lv_obj_del_async(gui->time);
    lv_obj_del_async(gui->yymm);

    lv_obj_del_async(gui->humidity);
    lv_obj_del_async(gui->temp);
    lv_obj_del_async(gui->msg);
    //lv_obj_del_async(gui->weekday);
}
void on_delete_scr_weather(struct my_lvgl_screen* scr)
{
	struct my_scr_weather* data=(struct my_scr_weather*)scr->userData;
	scr->userData=NULL;
    
	//lv_obj_del_async(data->lbl_weather_temperature);
    del_gui_weather(&data->gui_weather);
    del_gui_today(&data->gui_today);
    lv_freetype_font_delete(data->font16);
    lv_freetype_font_delete(data->font24);
    //lv_freetype_font_delete(data->font48);
    //lv_freetype_font_delete(data->font72);
    lv_style_reset(&data->style16);
    lv_style_reset(&data->style24);
    lv_style_reset(&data->style48);
    lv_style_reset(&data->style72);
    lv_binfont_destroy(data->font48);
    lv_binfont_destroy(data->font72);

    my_wait_thread_exit(&data->thread_update);
	del_weather(&data->weather);

    //_lv_fs_deinit();
	free(data);
}

void make_label_text_vertical_center(lv_obj_t* obj,lv_font_t* font,const char* title)
{
    uint32_t height=lv_font_get_line_height(font);
    uint32_t obj_h=lv_obj_get_height(obj);
    uint32_t pad;
    uint32_t line =1;
    lv_label_set_text(obj,title);
    while(title)
    {
        title=strchr(title,'\n');
        if(title)
        {
            line++;
            title+=1;
        }
    }
    pad=(obj_h-height*line)/2;
    lv_obj_set_style_pad_top(obj,pad,0);
}
void update_weather_real(struct my_scr_weather* scr,struct Weather_Real* real)
{
    char* disp_buf=scr->disp_buf;
    struct my_scr_weather_weather* gui=&scr->gui_weather;
    sprintf(disp_buf,"%.1f℃",real->weather.temperature);
    //lv_label_set_text(gui->temperature,);
    make_label_text_vertical_center(gui->temperature,scr->font72,disp_buf);

    sprintf(disp_buf,"体感\n%.1f℃",real->weather.feelst);
    //lv_label_set_text(gui->feelst,disp_buf);
    make_label_text_vertical_center(gui->feelst,scr->font16,disp_buf);

    sprintf(disp_buf,"降雨量\n%.0f",real->weather.rain);
    //lv_label_set_text(gui->rain,disp_buf);
    make_label_text_vertical_center(gui->rain,scr->font16,disp_buf);

    sprintf(disp_buf,"湿度\n%.0f",real->weather.humidity);
    //lv_label_set_text(gui->humidity,disp_buf);
    make_label_text_vertical_center(gui->humidity,scr->font16,disp_buf);

    sprintf(disp_buf,"%s\n%s",real->wind.direct,real->wind.power);
    //lv_label_set_text(gui->wind,disp_buf);
    make_label_text_vertical_center(gui->wind,scr->font16,disp_buf);

    //lv_label_set_text(gui->info,real->weather.info);
    make_label_text_vertical_center(gui->info,scr->font24,real->weather.info);
    

    //make_label_text_vertical_center(gui->info,scr->font48);

    //uint32_t height=lv_font_get_line_height(scr->font48);
    //uint32_t pad=(75-height)/2;
    //lv_obj_set_style_pad_top(gui->info,pad,0);
    //lv_obj_set_align(gui->info,LV_ALIGN_CENTER);

    //lv_obj_set_size(gui->info,155,75);
    //lv_obj_set_style_align(gui->info,LV_TEXT_ALIGN_CENTER,0);
    

    //lv_obj_set_style_bg_color(gui->info,lv_color_hex(0xff82CD),LV_PART_MAIN);

    sprintf(disp_buf,"A/mnt/UDISK/icon/%s.png",real->weather.img);
    lv_image_set_src(gui->icon,disp_buf);
    sprintf(disp_buf,"%s %s",real->station.province,real->station.city);
    
    lv_label_set_text(lv_obj_get_child(scr->weather_info,0),disp_buf);
    lv_label_set_text(scr->lbl_status,real->publish_time);
}
void update_weather_today(struct my_scr_weather* scr)
{
    char* disp_buf=scr->disp_buf;
    struct my_scr_today* gui=&scr->gui_today;
    time_t t;
    
    time(&t);
    struct tm* tm=localtime(&t);
    sprintf(disp_buf,"%d年%02d月%02d日%s",
        1900+tm->tm_year,tm->tm_mon+1,tm->tm_mday,
        getWeekName(tm->tm_wday));
    //lv_label_set_text(gui->yymm,disp_buf);
    make_label_text_vertical_center(gui->yymm,scr->font24,disp_buf);

    //sprintf(disp_buf,"%d",tm->tm_mday);
    //make_label_text_vertical_center(gui->day,scr->font48,disp_buf);

    //sprintf(disp_buf,"%s",getWeekName(tm->tm_wday));
    //make_label_text_vertical_center(gui->weekday,scr->font24,disp_buf);

    sprintf(disp_buf,"%02d:%02d:%02d",tm->tm_hour,tm->tm_min,tm->tm_sec);

    lv_scale_set_line_needle_value(scr->clock, scr->minute_hand, 60, tm->tm_min);
    

    /* the scale will allocate the hour hand line points */
    lv_scale_set_line_needle_value(scr->clock, scr->hour_hand, 40, tm->tm_hour * 5 + (tm->tm_min / 12));

    //lv_label_set_text(gui->time,disp_buf);
    make_label_text_vertical_center(gui->time,scr->font48,disp_buf);
    pthread_mutex_lock(&mqttmutex);
    if(last_mqtt_data.mqtt_flag&sensor_flag_bmp_temp)
    {
        sprintf(disp_buf,"温度:%.1f℃",last_mqtt_data.bmp_temp);
        lv_label_set_text(gui->temp,disp_buf);
    }
    if(last_mqtt_data.mqtt_flag&sensor_flag_bmp_ap)
    {
        sprintf(disp_buf,"气压:%.1f",last_mqtt_data.bmp_ap);
        lv_label_set_text(gui->humidity,disp_buf);
    }
    if(last_mqtt_data.mqtt_flag&sensor_flag_msg)
    {
        lv_label_set_text(gui->msg,last_mqtt_data.msg);
    }
    pthread_mutex_unlock(&mqttmutex);
    
}
void on_update_scr_weather(struct my_lvgl_screen* scr)
{
	struct my_scr_weather* data=(struct my_scr_weather*)scr->userData;
    //update ui when weather is updated.
    if(data->weather_flag&0x01)
    {
        printf("gui update...\n");
        update_weather_real(data,&data->weather.real);
        
        data->weather_flag=0;
    }
    //update time
    update_weather_today(data);
}

void create_weather_gui_weather(lv_obj_t* scr,struct my_scr_weather* weather,struct my_scr_weather_weather* gui)
{
    gui->temperature=lv_label_create(scr);
    //lv_label_set_text(data->lbl_temperature,data->weather.real.publish_time);
    lv_obj_add_style(gui->temperature,&weather->style72,0);

    lv_obj_set_style_align(gui->temperature,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_grid_cell(gui->temperature,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_STRETCH,1,2);
    lv_obj_set_size(gui->temperature,310,150);
    //make_label_text_vertical_center(gui->temperature,weather->font72);

    gui->icon=lv_image_create(scr);
    lv_obj_set_grid_cell(gui->icon,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,1);
    lv_obj_set_size(gui->icon,155,75);
    

    gui->info=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->info,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,0,1);
    lv_obj_add_style(gui->info,&weather->style24,0);
    //make_label_text_vertical_center(gui->info,weather->font24);
    //lv_obj_center(gui->info);
    

    gui->feelst=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->feelst,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,3,1);
    lv_obj_add_style(gui->feelst,&weather->style16,0);
    lv_obj_set_style_align(gui->feelst,LV_TEXT_ALIGN_CENTER,0);
    //make_label_text_vertical_center(gui->feelst,weather->font16);

    gui->rain=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->rain,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,3,1);
    lv_obj_add_style(gui->rain,&weather->style16,0);
    lv_obj_set_style_align(gui->rain,LV_TEXT_ALIGN_CENTER,0);
    //make_label_text_vertical_center(gui->rain,weather->font16);
    gui->humidity=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->humidity,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,4,1);
    lv_obj_add_style(gui->humidity,&weather->style16,0);
    lv_obj_set_style_align(gui->humidity,LV_TEXT_ALIGN_CENTER,0);
    //make_label_text_vertical_center(gui->humidity,weather->font16);

    gui->wind=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->wind,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,4,1);
    lv_obj_add_style(gui->wind,&weather->style16,0);
    lv_obj_set_style_align(gui->wind,LV_TEXT_ALIGN_CENTER,0);
    //make_label_text_vertical_center(gui->wind,weather->font16);
}

void create_weather_gui_today(lv_obj_t* scr,struct my_scr_weather* weather,struct my_scr_today* gui)
{
    gui->yymm=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->yymm,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,0,1);
    lv_obj_add_style(gui->yymm,&weather->style24,0);
    
    /*gui->day=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->day,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,1,1);
    lv_obj_add_style(gui->day,&weather->style48,0);

    gui->weekday=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->weekday,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,2,1);
    lv_obj_add_style(gui->weekday,&weather->style24,0);
    *///lv_obj_set_style_align(gui->weekday,LV_TEXT_ALIGN_CENTER,0);
    printf("%s:%d\n",__FUNCTION__,__LINE__);

    gui->time=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->time,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,1,1);
    lv_obj_add_style(gui->time,&weather->style48,0);
    //lv_obj_set_style_align(gui->time,LV_TEXT_ALIGN_CENTER,0);
printf("%s:%d\n",__FUNCTION__,__LINE__);
    gui->temp=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->temp,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,2,1);
    lv_obj_add_style(gui->temp,&weather->style24,0);
printf("%s:%d\n",__FUNCTION__,__LINE__);
    gui->humidity=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->humidity,LV_GRID_ALIGN_STRETCH,2,2,LV_GRID_ALIGN_STRETCH,3,1);
    lv_obj_add_style(gui->humidity,&weather->style24,0);
    gui->msg=lv_label_create(scr);
    lv_obj_set_grid_cell(gui->msg,LV_GRID_ALIGN_STRETCH,2,3,LV_GRID_ALIGN_STRETCH,4,2);
    lv_obj_add_style(gui->msg,&weather->style24,0);
    printf("%s:%d\n",__FUNCTION__,__LINE__);
}

void on_event_predict_cb(lv_event_t* e)
{
    struct my_lvgl_screen* predict_scr=(struct my_lvgl_screen*)e->user_data;
    struct my_scr_weather* data=(struct my_scr_weather*)predict_scr->userData;
    struct my_lvgl_screen* scr=create_scr_predict(&data->weather.predict);
	my_screen_load(scr);
}



static int weather_update(void* p,pthread_mutex_t* mutex )
{
    struct resp_buf buf={NULL,0};
    struct my_scr_weather* data=(struct my_scr_weather*)p;
    char url[128];
    time_t t;
    struct station_key_value station;
    
    if(load_station(&station,station_cfg))
    {
        time(&t);
        printf("requesturl...\n");
        sprintf(url,"/rest/weather?stationid=%s&_=%d",station.code,t*1000);
        request_url("www.nmc.cn","80",url,&buf);
        if(buf.buf)
        {
            save_to_file(buf.buf,buf.len,weather_content);
            pthread_mutex_lock(mutex);
            printf("update...\n");
            del_weather(&data->weather);
            weather_parser_src(buf.buf,buf.len,&data->weather);
            data->weather_flag|=0x1;
            if(buf.buf)
                free(buf.buf);
            pthread_mutex_unlock(mutex);
        }
        clear_station_key_value(&station);
        return 1000;
    }
    

    return 10000;
}

void weather_station_event_cb(lv_event_t* e)
{
    struct my_lvgl_screen* predict_scr=(struct my_lvgl_screen*)e->user_data;
    struct my_scr_weather* data=(struct my_scr_weather*)predict_scr->userData;
    struct my_lvgl_screen* scr=create_screen_weather_station_main();
	my_screen_load(scr);
}
void create_clock(struct my_scr_weather* weather,lv_obj_t* parent)
{
    weather->clock=lv_scale_create(parent);
    lv_obj_set_style_margin_all(weather->clock,0,0);
    lv_obj_set_size(weather->clock,220,220);
    lv_scale_set_mode(weather->clock,LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(weather->clock,LV_OPA_60,0);
    lv_obj_set_style_bg_color(weather->clock,lv_color_black(),0);
    lv_obj_set_style_radius(weather->clock,LV_RADIUS_CIRCLE,0);
    lv_obj_set_style_clip_corner(weather->clock,true,0);

    lv_scale_set_label_show(weather->clock,true);
    lv_scale_set_total_tick_count(weather->clock, 61);
    lv_scale_set_major_tick_every(weather->clock, 5);

    static const char * hour_ticks[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", NULL};
    lv_scale_set_text_src(weather->clock, hour_ticks);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_main(LV_PALETTE_YELLOW));

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_length(&indicator_style, 8); /* tick length */
    lv_style_set_line_width(&indicator_style, 2); /* tick width */
    lv_obj_add_style(weather->clock, &indicator_style, LV_PART_INDICATOR);

    /* Minor tick properties */
    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_length(&minor_ticks_style, 6); /* tick length */
    lv_style_set_line_width(&minor_ticks_style, 2); /* tick width */
    lv_obj_add_style(weather->clock, &minor_ticks_style, LV_PART_ITEMS);

    /* Main line properties */
    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    lv_style_set_arc_color(&main_line_style, lv_color_black());
    lv_style_set_arc_width(&main_line_style, 5);
    lv_obj_add_style(weather->clock, &main_line_style, LV_PART_MAIN);

    lv_scale_set_range(weather->clock, 0, 60);

    lv_scale_set_angle_range(weather->clock, 360);
    lv_scale_set_rotation(weather->clock, 270);

    weather->minute_hand = lv_line_create(weather->clock);
    lv_line_set_points_mutable(weather->minute_hand, weather->minute_hand_points, 2);

    lv_obj_set_style_line_width(weather->minute_hand, 3, 0);
    lv_obj_set_style_line_rounded(weather->minute_hand, true, 0);
    lv_obj_set_style_line_color(weather->minute_hand, lv_color_white(), 0);

    weather->hour_hand = lv_line_create(weather->clock);

    lv_obj_set_style_line_width(weather->hour_hand, 5, 0);
    lv_obj_set_style_line_rounded(weather->hour_hand, true, 0);
    lv_obj_set_style_line_color(weather->hour_hand, lv_palette_main(LV_PALETTE_RED), 0);

}
/**
 * |   天气             |   month year
 * |   温度             |   day
 * |                    |   weekday
 * |feelst  |rain       |   time
 * |humidity|wind|
 */
struct my_lvgl_screen* create_screen_weather_main()
{
    struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_weather* data=(struct my_scr_weather*)malloc(sizeof(struct my_scr_weather));
    static int32_t col_dec[]={175,175,175,175,220,LV_GRID_TEMPLATE_LAST};
	static int32_t row_dec[]={66,66,66,66,66,66,66,40,LV_GRID_TEMPLATE_LAST};
    lv_obj_set_style_bg_color(ret->screen,lv_color_hex(0x1882CD),LV_PART_MAIN);
    lv_obj_set_style_pad_all(ret->screen,5,LV_PART_MAIN);
	memset(&data->weather,0,sizeof(struct Weather));
    weather_init_font(data);
    
    create_clock(data,ret->screen);
    //lv_obj_set_pos(data->clock,175*4,5);
    lv_obj_set_grid_cell(data->clock,LV_GRID_ALIGN_STRETCH,4,1,LV_GRID_ALIGN_STRETCH,0,3);
    //weather_parser_file("../bin/weather.json",&data->weather);
    weather_parser_file(weather_content,&data->weather);
    
	ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;
    lv_obj_set_grid_cell(ret->btn_back,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,6,1);

    data->predict=create_button_with_label(ret->screen,"预报",on_event_predict_cb,ret);
    lv_obj_set_grid_cell(data->predict,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,6,1);

	lv_obj_set_style_grid_column_dsc_array(ret->screen,col_dec,0);
	lv_obj_set_style_grid_row_dsc_array(ret->screen,row_dec,0);
	lv_obj_set_layout(ret->screen,LV_LAYOUT_GRID);

    create_weather_gui_weather(ret->screen,data,&data->gui_weather);
    printf("init create_weather_gui_today\n");
    create_weather_gui_today(ret->screen,data,&data->gui_today);
    printf("end create_weather_gui_today\n");
    ret->delete_cb=on_delete_scr_weather;
	ret->update_cb=on_update_scr_weather;
    data->lbl_status=create_status_bar(ret->screen);
    lv_obj_set_grid_cell(data->lbl_status,LV_GRID_ALIGN_STRETCH,0,4,LV_GRID_ALIGN_STRETCH,7,1);
    data->weather_info=create_button_with_label(ret->screen,"预报站点",weather_station_event_cb,ret);
    lv_obj_set_grid_cell(data->weather_info,LV_GRID_ALIGN_STRETCH,0,2,LV_GRID_ALIGN_STRETCH,5,1);

    printf("init thread\n");
    my_thread_init(&data->thread_update);
    data->thread_update.scr=ret;
    data->thread_update.loop=weather_update;
    data->weather_flag=0;
    my_create_thread(&data->thread_update);
	ret->userData=data;
	return ret;
}
