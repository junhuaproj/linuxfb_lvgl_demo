#include "gui.h"
#include <stdlib.h>
#include "weather.h"
#include <time.h>

struct my_scr_predict_day
{
    lv_obj_t* container;
    lv_obj_t* mmdd;
    lv_obj_t* weekday;
    lv_obj_t* day_icon;
    lv_obj_t* day_info;
    lv_obj_t* day_wind_direct;
    lv_obj_t* day_wind_power;
    lv_obj_t* day_temp;
    lv_obj_t* night_temp;
    lv_obj_t* night_icon;
    lv_obj_t* night_info;
    lv_obj_t* night_wind_direct;
    lv_obj_t* night_wind_power;
    lv_obj_t* vertical_line;
    lv_point_precise_t vertical_line_points[2];
};

struct my_scr_predict
{
    /*lv_obj_t* canvas;
    lv_draw_buf_t can_buf;
    uint8_t* buf;
    uint16_t can_width;
    uint16_t can_height;
    uint8_t can_clr_format;*/
    int col_width;

    struct my_scr_predict_day* day;
    //lv_font_t* font48;
    lv_font_t* font24;
    lv_style_t normal_style;
    lv_style_t container_style;
    //lv_style_t container_focus_style;
    lv_obj_t* container_focus;

    struct Weather_Predict predict;
};

struct my_label_align_arg{
    lv_font_t* font;
    int width;
};
//void make_label_text_horzontal_center(lv_obj_t* obj,lv_font_t* font,const char* title,int width)
void make_label_text_horzontal_center(lv_obj_t* obj,struct my_label_align_arg* arg,const char* title)
{
    //uint32_t height=lv_font_get_line_height(font);
    uint32_t obj_h=lv_obj_get_height(obj);
    //uint32_t pad;
    //uint32_t line =1;
    lv_point_t p;
    lv_txt_get_size(&p,title,arg->font,0,0,LV_COORD_MAX,LV_TEXT_FLAG_EXPAND);
    //uint32_t cwidth=lv_obj_get_content_width(obj);
    lv_label_set_text(obj,title);
    //printf("cwidth=%d\n",cwidth);
    //pad=;
    lv_obj_set_style_pad_left(obj,(arg->width-p.x)/2,0);
    lv_obj_set_style_pad_top(obj,(obj_h-p.y)/2,0);
}

void on_delete_scr_predict(struct my_lvgl_screen* scr)
{
    struct my_scr_predict* data=(struct my_scr_predict*)scr->userData;
    //lv_obj_delete_async(data->canvas);
    struct my_scr_predict_day* day;
    for(int i=0;i<data->predict.detail_cnt;i++)
    {
        day=data->day+i;
        lv_obj_del_async(day->container);
        lv_obj_del_async(day->mmdd);
        lv_obj_del_async(day->weekday);
        lv_obj_del_async(day->day_icon);
        lv_obj_del_async(day->day_info);
        lv_obj_del_async(day->day_wind_direct);
        lv_obj_del_async(day->day_wind_power);
        lv_obj_del_async(day->day_temp);
        lv_obj_del_async(day->night_temp);
        lv_obj_del_async(day->night_icon);
        lv_obj_del_async(day->night_info);
        lv_obj_del_async(day->night_wind_direct);
        lv_obj_del_async(day->night_wind_power);
        if(day->vertical_line)
            lv_obj_del_async(day->vertical_line);
    }
    free(data->day);
    del_weather_predict(&data->predict);
    //lv_binfont_destroy(data->font48);
    lv_freetype_font_delete(data->font24);
    //free(data->buf);
}
struct tm* conv_str_date(const char* ymd)
{
    char tmp[8];
    strncpy(tmp,ymd,4);
    tmp[4]=0;
    struct tm tm;
    memset(&tm,0,sizeof(struct tm));
    tm.tm_year=atoi(tmp)-1900;
    strncpy(tmp,ymd+5,2);
    tmp[2]=0;
    tm.tm_mon=atoi(tmp)-1;
    strncpy(tmp,ymd+8,2);
    tmp[2]=0;
    tm.tm_mday=atoi(tmp);
    time_t t=mktime(&tm);
    //printf("%s,%d-%d-week \n",ymd,tm.tm_mon,tm.tm_mday);
    return localtime(&t);
    //struct tm* loctm=localtime(&t);
    //printf("%d-%d-week %d\n",loctm->tm_mon,loctm->tm_mday,loctm->tm_wday);
}
void on_update_scr_predict(struct my_lvgl_screen* scr)
{
    struct my_scr_predict* data=(struct my_scr_predict*)scr->userData;
    char buf[64];
    struct Weather_Predict_Day* day;
    struct my_scr_predict_day* uiday=data->day;
    const uint8_t column_cnt=data->predict.detail_cnt;
    struct tm *tm;
    struct my_label_align_arg arg={data->font24,data->col_width};
    for(uint8_t i=0;i<column_cnt;i++,uiday++)
    {
        day=data->predict.detail+i;
        int32_t container_width=lv_obj_get_width(uiday->container);
        //printf("container width:%d\n",lv_obj_get_content_width(uiday->container));
        tm=conv_str_date(day->date);
        //strptime_l(day->date,"%Y-%m-%d",&tm);
        sprintf(buf,"%02d/%02d",tm->tm_mon+1,tm->tm_mday);
        //lv_label_set_text(uiday->mmdd,buf);

        //lv_label_set_text(uiday->weekday,getWeekName(tm->tm_wday));
        
        //lv_obj_set_width(uiday->mmdd,container_width);
        //lv_obj_set_width(uiday->weekday,container_width);
        
        make_label_text_horzontal_center(uiday->mmdd,&arg,buf);
        make_label_text_horzontal_center(uiday->weekday,&arg,getWeekName(tm->tm_wday));

        sprintf(buf,"A/mnt/UDISK/icon/%s.png",day->day.weather.img);
        lv_image_set_src(uiday->day_icon,buf);
        make_label_text_horzontal_center(uiday->day_info,&arg,day->day.weather.info);
        make_label_text_horzontal_center(uiday->day_wind_direct,&arg,day->day.wind.direct);
        make_label_text_horzontal_center(uiday->day_wind_power,&arg,day->day.wind.power);
        sprintf(buf,"%s℃",day->day.weather.temperature);
        make_label_text_horzontal_center(uiday->day_temp,&arg,buf);

        sprintf(buf,"%s℃",day->night.weather.temperature);
        //day->night.weather.temperature
        make_label_text_horzontal_center(uiday->night_temp,&arg,buf);
        sprintf(buf,"A/mnt/UDISK/icon/%s.png",day->night.weather.img);
        lv_image_set_src(uiday->night_icon,buf);

        make_label_text_horzontal_center(uiday->night_info,&arg,day->night.weather.info);
        make_label_text_horzontal_center(uiday->night_wind_direct,&arg,day->night.wind.direct);
        make_label_text_horzontal_center(uiday->night_wind_power,&arg,day->night.wind.power);
        //make_label_text_horzontal_center(uiday->mmdd,data->font24,buf,data->col_width);
        //conv_str_date(day->date);
    }
    /*lv_layer_t layer;
    lv_canvas_init_layer(data->canvas,&layer);
    
    lv_draw_line_dsc_t dsc;
    lv_draw_label_dsc_t dsc24;
    lv_draw_label_dsc_t dsc48;
    lv_draw_line_dsc_init(&dsc);
    lv_draw_label_dsc_init(&dsc24);
    lv_draw_label_dsc_init(&dsc48);
    
    dsc.color=lv_color_hex3(0x000);
    dsc24.color=dsc.color;
    dsc24.font=data->font24;
    dsc24.blend_mode=LV_BLEND_MODE_ADDITIVE;
    dsc24.align=LV_TEXT_ALIGN_CENTER;
    lv_area_t area;
    const uint8_t column_cnt=7;
    uint16_t item_width=data->can_width/column_cnt;
    struct Weather_Predict_Day* day;
    struct tm tm;
    dsc.width=1;
    dsc.p1.y=0;
    dsc.p2.y=data->can_height;
    for(uint8_t i=0;i<column_cnt-1;i++)
    {
        dsc.p1.x=(i+1)*item_width;
        dsc.p2.x=dsc.p1.x;
        lv_draw_line(&layer,&dsc);
    }
    char buf[64];
    for(uint8_t i=0;i<column_cnt;i++)
    {
        day=data->predict.detail+i;
        area.x1=i*item_width;
        area.x2=(i+1)*item_width;

        area.y1=0;
        area.y2=40;
        //strptime_l(day->date,"%Y-%m-%d %H:%M",&tm);
        //sprintf(buf,"%02d/%02d",tm.tm_mon+1,tm.tm_mday);
        dsc24.text=day->date;
        lv_draw_label(&layer,&dsc24,&area);
    }
    lv_canvas_finish_layer(data->canvas,&layer);*/
}
/*
void init_canvas(struct my_scr_predict* predict,lv_obj_t* scr)
{
    predict->can_width=790;
    predict->can_height=440;
    predict->can_clr_format=LV_COLOR_FORMAT_ARGB8888;
    uint32_t buf_len=_LV_DRAW_BUF_SIZE(predict->can_width, predict->can_height, predict->can_clr_format);
    predict->buf=(uint8_t*)malloc(buf_len);
    
    //LV_DRAW_BUF_DEFINE_STATIC
    predict->can_buf.header.magic=LV_IMAGE_HEADER_MAGIC;
    predict->can_buf.header.cf=predict->can_clr_format;
    predict->can_buf.header.flags=LV_IMAGE_FLAGS_MODIFIABLE;
    predict->can_buf.header.w=predict->can_width;
    predict->can_buf.header.h=predict->can_height;
    predict->can_buf.header.stride=_LV_DRAW_BUF_STRIDE(predict->can_width, predict->can_clr_format);
    predict->can_buf.header.reserved_2=0;

    predict->can_buf.data_size=buf_len;
    predict->can_buf.data=predict->buf;
    predict->can_buf.unaligned_data=predict->buf;

    lv_image_header_t * header = &(predict->can_buf.header);
    lv_draw_buf_init(&predict->can_buf, header->w, header->h, header->cf, 
    header->stride,predict->buf, buf_len);
    lv_draw_buf_set_flag(&predict->can_buf, LV_IMAGE_FLAGS_MODIFIABLE);

    predict->canvas=lv_canvas_create(scr);
    lv_canvas_set_draw_buf(predict->canvas,&predict->can_buf);
    lv_obj_set_pos(predict->canvas,5,5);
    lv_obj_set_size(predict->canvas,predict->can_width,predict->can_height);

    lv_canvas_fill_bg(predict->canvas,lv_color_hex3(0x18c),LV_OPA_COVER);
    
}
*/

void init_container_style(lv_style_t* style)
{
    lv_style_init(style);
    lv_style_set_margin_left(style,0);
    lv_style_set_margin_right(style,0);
    lv_style_set_margin_top(style,2);
    lv_style_set_margin_bottom(style,0);
    lv_style_set_pad_left(style,0);
    lv_style_set_pad_top(style,5);
    lv_style_set_pad_right(style,0);
    lv_style_set_pad_bottom(style,0);
    //lv_style_set_bg_color(style,lv_color_hex3(0xfff));
    //lv_style_set_bg_main_opa(style,LV_OPA_0);
    lv_style_set_border_width(style,0);
    
}
void init_style_font(struct my_scr_predict* data)
{
    //data->font48= lv_binfont_create("A/home/wang/lv_port_pc_v3/bin/STXIHEI48.bin");
	data->font24= lv_freetype_font_create(
			 "/mnt/UDISK/HYSongYunLangHeiW-1.ttf",
			 LV_FREETYPE_FONT_RENDER_MODE_BITMAP,20,LV_FREETYPE_FONT_STYLE_NORMAL);

    lv_style_init(&data->normal_style);
    //lv_style_set_border_color(&data->normal_style,lv_color_hex3(0x0));
    //lv_style_set_border_width(&data->normal_style,1);
    lv_style_set_text_font(&data->normal_style, data->font24);
    lv_style_set_margin_left(&data->normal_style,0);
    lv_style_set_margin_right(&data->normal_style,0);


    init_container_style(&data->container_style);
    /*lv_style_init(&data->container_style);
    lv_style_set_margin_left(&data->container_style,0);
    lv_style_set_margin_right(&data->container_style,0);
    lv_style_set_margin_top(&data->container_style,2);
    lv_style_set_margin_bottom(&data->container_style,0);
    lv_style_set_pad_left(&data->container_style,0);
    lv_style_set_pad_top(&data->container_style,5);
    lv_style_set_pad_right(&data->container_style,0);
    lv_style_set_pad_bottom(&data->container_style,0);
    lv_style_set_bg_color(&data->container_style,lv_color_hex3(0xfff));
    //lv_style_set_bg_main_opa(&data->container_style,LV_OPA_0);
    lv_style_set_border_width(&data->container_style,0);*/
    //lv_obj_set_style_bg_opa(day->container,LV_OPA_50,0);

    //lv_obj_set_style_text_font(day->weekday,data->font24,0);
    //init_container_style(&data->container_focus_style);
}


struct create_obj_arg{
    lv_obj_t* parent;
    lv_style_t* style;
    int width;
    int top;
};

//lv_obj_t* create_predict_label(lv_obj_t* parent,lv_style_t* style,int top,int width,int height)
lv_obj_t* create_predict_label(struct create_obj_arg *arg,int height)
{
    lv_obj_t* lbl=lv_label_create(arg->parent);
    lv_obj_set_pos(lbl,0,arg->top);
    lv_obj_add_style(lbl,arg->style,0);
    lv_obj_set_size(lbl,arg->width,height);
    return lbl;
}
//lv_obj_t* create_predict_image(lv_obj_t* parent,lv_style_t* style,int top,int width,int height)
lv_obj_t* create_predict_image(struct create_obj_arg *arg,int height)
{
    lv_obj_t* lbl=lv_image_create(arg->parent);
    lv_obj_set_pos(lbl,0,arg->top);
    lv_obj_add_style(lbl,arg->style,0);
    lv_obj_set_size(lbl,arg->width,height);
    return lbl;
}

void on_container_cb(lv_event_t* e)
{
    struct my_scr_predict* data=(struct my_scr_predict*)e->user_data;
    if(data->container_focus)
        lv_obj_set_style_bg_opa(data->container_focus,LV_OPA_0,0);
    data->container_focus=lv_event_get_target_obj(e);
    lv_obj_set_style_bg_color(data->container_focus,lv_color_hex3(0x06a),0);
    lv_obj_set_style_bg_opa(data->container_focus,LV_OPA_50,0);
}

struct my_lvgl_screen*  create_scr_predict(const struct Weather_Predict* old)
{
    struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_predict* data=(struct my_scr_predict*)malloc(sizeof(struct my_scr_predict));
    data->container_focus=NULL;
    ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;
    init_style_font(data);
    lv_obj_set_style_bg_color(ret->screen,lv_color_hex3(0x18d),0);
    
    cpy_weather_predict(&data->predict,old);
    data->day=(struct my_scr_predict_day*)malloc(sizeof(struct my_scr_predict_day)*old->detail_cnt);
    struct my_scr_predict_day* day=data->day;
    const uint8_t column_cnt=7;
    uint16_t item_width=1014/column_cnt;
    //uint32_t top;
    const uint32_t lbl_height=36,img_height=56;

    //内部obj设置的稍小一些
    data->col_width=item_width;//-4;
    struct create_obj_arg arg={NULL,&data->normal_style,data->col_width,0};
    for(int i=0;i<old->detail_cnt;i++,day++)
    {
        arg.top=0;
        day->container=lv_obj_create(ret->screen);
        //设置圆角矩形的半径，这里设置为0，=方角
        lv_obj_set_style_radius(day->container,0,LV_PART_MAIN);
        lv_obj_add_event_cb(day->container,on_container_cb,LV_EVENT_CLICKED,data);
        //背景透明
        lv_obj_set_style_bg_opa(day->container,LV_OPA_0,0);
        //lv_obj_set_style_pad_right(day->container,0,0);
        //lv_obj_set_style_pad_left(day->container,0,0);
        lv_obj_add_style(day->container,&data->container_style,0);
        lv_obj_set_pos(day->container,5+i*(item_width),0);
        lv_obj_set_size(day->container,(item_width),500);
        arg.parent=day->container;
        
        day->mmdd=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->weekday=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;

        day->day_icon=create_predict_image(&arg,img_height);
        arg.top+=img_height;

        day->day_info=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;

        day->day_wind_direct=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->day_wind_power=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->day_temp=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->night_temp=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
//
        day->night_icon=create_predict_image(&arg,img_height);
        arg.top+=img_height;
        day->night_info=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->night_wind_direct=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        day->night_wind_power=create_predict_label(&arg,lbl_height);
        arg.top+=lbl_height;
        ///
        if(i!=0)
        {
            day->vertical_line=lv_line_create(ret->screen);
            day->vertical_line_points[0].y=5;
            day->vertical_line_points[1].y=460;
            day->vertical_line_points[0].x=5+i*(item_width);
            day->vertical_line_points[1].x=day->vertical_line_points[0].x;
            lv_line_set_points(day->vertical_line,day->vertical_line_points,2);
            lv_obj_set_pos(day->vertical_line,0,0);
        }
        else
        {
            day->vertical_line=NULL;
        }
        /*day->mmdd=lv_label_create(day->container);
        lv_obj_set_pos(day->mmdd,0,0);
        lv_obj_add_style(day->mmdd,&data->normal_style,0);
        lv_obj_set_size(day->mmdd,data->col_width,40);
        day->weekday=lv_label_create(day->container);
        lv_obj_set_pos(day->weekday,0,40);
        //lv_obj_set_align(day->weekday,LV_ALIGN_CENTER);
        //
        /*lv_obj_set_style_margin_left(day->weekday,0,0);
        lv_obj_set_style_margin_right(day->weekday,0,0);
        lv_obj_set_style_margin_left(day->mmdd,0,0);
        lv_obj_set_style_margin_right(day->mmdd,0,0);*/
        /*lv_obj_add_style(day->weekday,&data->normal_style,0);
        
        lv_obj_set_size(day->weekday,data->col_width,40);*/
        //lv_obj_refresh_self_size(day->weekday);
    }

    //init_canvas(data,ret->screen);
    lv_obj_set_pos(ret->btn_back,5,520);
    lv_obj_set_size(ret->btn_back,100,60);

    ret->delete_cb=on_delete_scr_predict;
	ret->update_cb=on_update_scr_predict;
	ret->userData=data;
	return ret;
}


