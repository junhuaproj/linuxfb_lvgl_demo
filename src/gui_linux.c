/*
 * gui_linux.c
 *
 *  Created on: Jul 1, 2024
 *      Author: wang
 */

#include "gui.h"
#include <string.h>
#include "linux_stat.h"

struct my_scr_linux
{
	lv_obj_t* scale_mem;
	lv_obj_t* scale_mem_needle;
	lv_obj_t* lbl_mem;
	lv_obj_t* scale_swap;
	lv_obj_t* scale_swap_needle;
	lv_obj_t* lbl_swap;
	lv_obj_t* scale_cpu;
	lv_obj_t* scale_cpu_needle;
	lv_obj_t* lbl_cpu;
	lv_obj_t* lbl_status;
	lv_obj_t* tabview;
	lv_obj_t* scale_storage;

	lv_obj_t* table_info;
	struct linux_cpu_stat cpu_stat;
	int cpu_stat_valid;
	int to_update
};

extern lv_style_t style;

void on_delete_scr_linux(struct my_lvgl_screen* scr)
{
	struct my_scr_linux* data=(struct my_scr_linux*)scr->userData;
	scr->userData=NULL;
	lv_obj_del_async(data->scale_mem);
	lv_obj_del_async(data->scale_swap);
	lv_obj_del_async(data->scale_mem_needle);
	lv_obj_del_async(data->scale_swap_needle);
	lv_obj_del_async(data->lbl_status);
	lv_obj_del_async(data->table_info);
	free(data->cpu_stat.cpus);
	free(data);
}

/*static void set_table_info_cpu_row_str(struct linux_cpu_info* header,int cnt,int16_t row,lv_obj_t* table)
{
	int col=1;
	while(header)
	{
		lv_table_set_cell_value(table,row,col,header->vendor_id);
		header=header->next;
		col++;
	}
}
static void set_table_info_cpu_row_int(struct linux_cpu_info* header,int cnt,int16_t row,lv_obj_t* table)
{
	int col=1;
	while(header)
	{
		lv_table_set_cell_value_fmt(table,row,col,"%d",header->vendor_id);
		header=header->next;
		col++;
	}
}*/
static void set_table_info_cpu(struct linux_cpu_info* p,int cnt,lv_obj_t* table)
{
	int col=1;
	lv_table_set_column_count(table,cnt+1);
	lv_table_set_row_count(table,2);
	lv_table_set_cell_value(table,0,0,"processor");
	lv_table_set_cell_value(table,1,0,"model_name");
	
	/*lv_table_set_row_count(table,11);
	lv_table_set_cell_value(table,0,0,"processor");
	lv_table_set_cell_value(table,1,0,"vendor_id");
	lv_table_set_cell_value(table,2,0,"cpu_family");
	lv_table_set_cell_value(table,3,0,"model");
	lv_table_set_cell_value(table,4,0,"model_name");
	lv_table_set_cell_value(table,5,0,"cpu_MHz");
	lv_table_set_cell_value(table,6,0,"cache_size");
	lv_table_set_cell_value(table,7,0,"physical_id");
	lv_table_set_cell_value(table,8,0,"siblings");
	lv_table_set_cell_value(table,9,0,"core_id");
	lv_table_set_cell_value(table,10,0,"cpu_cores");*/

	while(p)
	{
		lv_table_set_cell_value_fmt(table,0,col,"%d",p->processor);
		
		lv_table_set_cell_value(table,1,col,p->model_name);
		

		/*lv_table_set_cell_value_fmt(table,0,col,"%d",p->processor);
		lv_table_set_cell_value(table,1,col,p->vendor_id);
		lv_table_set_cell_value(table,2,col,p->cpu_family);
		lv_table_set_cell_value(table,3,col,p->model);
		lv_table_set_cell_value(table,4,col,p->model_name);
		lv_table_set_cell_value(table,5,col,p->cpu_MHz);
		lv_table_set_cell_value(table,6,col,p->cache_size);
		lv_table_set_cell_value_fmt(table,7,col,"%d",p->physical_id);
		lv_table_set_cell_value_fmt(table,8,col,"%d",p->siblings);
		lv_table_set_cell_value_fmt(table,9,col,"%d",p->core_id);
		lv_table_set_cell_value_fmt(table,10,col,"%d",p->cpu_cores);*/
		p=p->next;
		col++;
	}
	//lv_table_set_cell_value(table,0,1,p->vendor_id);
	
}
void on_update_scr_linux(struct my_lvgl_screen* scr)
{
	struct my_scr_linux* data=(struct my_scr_linux*)scr->userData;
	if(data->to_update<0)
	{
		struct linux_mem mem;
		if(linux_read_mem_info(&mem)==0)
		{
			//lv_linemeter_set_value(data->scale_mem,100*(mem.mem_free+mem.mem_available)/mem.mem_total);
			printf("mem:free=%d,avalidable=%d,total=%d\n",mem.mem_free,mem.mem_available,mem.mem_total);
			lv_scale_set_line_needle_value(data->scale_mem,data->scale_mem_needle,50,100*(mem.mem_free+mem.mem_available)/mem.mem_total);
			lv_scale_set_line_needle_value(data->scale_swap,data->scale_swap_needle,50,100*(mem.swap_total-mem.swap_free)/mem.swap_total);
			//lv_linemeter_set_value(data->scale_swap,100*(mem.swap_free)/mem.swap_total);
		}
		struct linux_cpu_info* cpu_info=read_cpu_infos();
		int cpu_cnt=0;
		if(cpu_info)
		{
			struct linux_cpu_info* p=cpu_info;
			while(p)
			{
				printf("disp: cpu %d\n",p->processor);
				p=p->next;
				cpu_cnt++;
			}
			set_table_info_cpu(cpu_info,cpu_cnt,data->table_info);
			del_cpu_info(cpu_info);
		}
		struct linux_cpu_stat cpu_stat;
		memset(&cpu_stat,0,sizeof(struct linux_cpu_stat));
		cpu_stat.cpu_cnt=cpu_cnt;
		cpu_stat.cpus=(struct linux_cpu_stat*)malloc(sizeof(struct linux_cpu_stat)*cpu_cnt);
		memset(cpu_stat.cpus,0,sizeof(struct linux_cpu_stat)*cpu_cnt);
		linux_read_cpu_stat(&cpu_stat);
		unsigned int prevUsed=0,prevIdle=0,curUsed,curIdle;
		if(data->cpu_stat_valid==0)
		{
			memcpy(&data->cpu_stat,&cpu_stat,sizeof(struct linux_cpu_stat));
			data->cpu_stat.cpus=(struct linux_cpu_stat*)malloc(sizeof(struct linux_cpu_stat)*cpu_cnt);
			data->cpu_stat_valid=1;
		}
		else
		{
			struct linux_cpu_stat* pOldCpus=data->cpu_stat.cpus;
			prevUsed=linux_get_cpu_busy(&data->cpu_stat);
			prevIdle=data->cpu_stat.idle;
			memcpy(&data->cpu_stat,&cpu_stat,sizeof(struct linux_cpu_stat));
			data->cpu_stat.cpus=pOldCpus;
		}
		memcpy(data->cpu_stat.cpus,cpu_stat.cpus,sizeof(struct linux_cpu_stat)*data->cpu_stat.cpu_cnt);
		//print_cpu(&cpu_stat);
		curUsed=linux_get_cpu_busy(&cpu_stat);
		curIdle=cpu_stat.idle;
		
		printf("cpu %u,%u,%u,%u,\n",curUsed,prevUsed,curIdle,prevIdle);
		//printf("cpu %u\n",100*(curUsed-prevUsed)/((curIdle-prevIdle)+(curUsed-prevUsed)));
		lv_scale_set_line_needle_value(data->scale_cpu,data->scale_cpu_needle,50,100*(curUsed-prevUsed)/((curIdle-prevIdle)+(curUsed-prevUsed)));
		for(int i=0;i<cpu_cnt;i++)
		{
			print_cpu(cpu_stat.cpus+i);
		}
		free(cpu_stat.cpus);
		//linux_storage_size();
		data->to_update=100;
	}
	else
		data->to_update--;
	update_status(data->lbl_status);
}

struct my_lvgl_screen* create_screen_linux_main()
{
	struct my_lvgl_screen* ret=create_default_screen();

	struct my_scr_linux* data=(struct my_scr_linux*)malloc(sizeof(struct my_scr_linux));
	static int32_t col_dec[]={190,190,190,100,60,LV_GRID_TEMPLATE_LAST};
	static int32_t row_dec[]={190,30,190,30,40,50,LV_GRID_TEMPLATE_LAST};
	data->to_update=0;
	ret->btn_back=create_button_back(ret->screen,ret);
	ret->create_cb=create_main_scr;
	lv_obj_set_style_grid_column_dsc_array(ret->screen,col_dec,0);
	lv_obj_set_style_grid_row_dsc_array(ret->screen,row_dec,0);
	lv_obj_set_layout(ret->screen,LV_LAYOUT_GRID);

	data->scale_mem=lv_scale_create(ret->screen);
	lv_scale_set_mode(data->scale_mem,LV_SCALE_MODE_ROUND_INNER);
	lv_obj_set_style_radius(data->scale_mem,LV_RADIUS_CIRCLE,0);
	lv_obj_set_style_clip_corner(data->scale_mem,true,0);
	lv_scale_set_range(data->scale_mem,0,100);
	lv_scale_set_angle_range(data->scale_mem,270);
	lv_obj_set_size(data->scale_mem,190,190);
	//lv_obj_set_pos(data->scale_mem,5,5);
	lv_obj_set_grid_cell(data->scale_mem,LV_GRID_ALIGN_STRETCH,0,1,LV_GRID_ALIGN_STRETCH,0,1);
	lv_scale_set_label_show(data->scale_mem,true);
	lv_scale_set_total_tick_count(data->scale_mem,51);
	lv_scale_set_major_tick_every(data->scale_mem,10);

	data->lbl_mem=lv_label_create(ret->screen);
	lv_label_set_text(data->lbl_mem,"mem");
	lv_obj_set_grid_cell(data->lbl_mem,LV_GRID_ALIGN_CENTER,0,1,LV_GRID_ALIGN_STRETCH,1,1);

	data->scale_mem_needle=lv_line_create(data->scale_mem);
	lv_obj_set_style_line_width(data->scale_mem_needle,3,LV_PART_MAIN);
	lv_obj_set_style_line_rounded(data->scale_mem_needle,true,LV_PART_MAIN);

	data->scale_swap=lv_scale_create(ret->screen);
	lv_scale_set_mode(data->scale_swap,LV_SCALE_MODE_ROUND_INNER);
	lv_obj_set_style_radius(data->scale_swap,LV_RADIUS_CIRCLE,0);
	lv_obj_set_style_clip_corner(data->scale_swap,true,0);
	lv_scale_set_range(data->scale_swap,0,100);
	lv_scale_set_angle_range(data->scale_swap,270);
	lv_obj_set_size(data->scale_swap,190,190);
	//lv_obj_set_pos(data->scale_swap,210,5);
	lv_obj_set_grid_cell(data->scale_swap,LV_GRID_ALIGN_STRETCH,1,1,LV_GRID_ALIGN_STRETCH,0,1);

	data->scale_swap_needle=lv_line_create(data->scale_swap);
	lv_obj_set_style_line_width(data->scale_swap_needle,3,LV_PART_MAIN);
	lv_obj_set_style_line_rounded(data->scale_swap_needle,true,LV_PART_MAIN);

	data->lbl_swap=lv_label_create(ret->screen);
	lv_label_set_text(data->lbl_swap,"swap");
	lv_obj_set_grid_cell(data->lbl_swap,LV_GRID_ALIGN_CENTER,1,1,LV_GRID_ALIGN_STRETCH,1,1);
	//lv_obj_center(data->lbl_swap);
	//lv_obj_add_event_cb(data->linemeter_mem,slider_event_cb,LV_EVENT_VALUE_CHANGED,ret);
	////cpu
	data->scale_cpu=lv_scale_create(ret->screen);
	lv_scale_set_mode(data->scale_cpu,LV_SCALE_MODE_ROUND_INNER);
	lv_obj_set_style_radius(data->scale_cpu,LV_RADIUS_CIRCLE,0);
	lv_obj_set_style_clip_corner(data->scale_cpu,true,0);
	lv_scale_set_range(data->scale_cpu,0,100);
	lv_scale_set_angle_range(data->scale_cpu,270);
	lv_obj_set_size(data->scale_cpu,190,190);
	//lv_obj_set_pos(data->scale_swap,210,5);
	lv_obj_set_grid_cell(data->scale_cpu,LV_GRID_ALIGN_STRETCH,2,1,LV_GRID_ALIGN_STRETCH,0,1);

	data->scale_cpu_needle=lv_line_create(data->scale_cpu);
	lv_obj_set_style_line_width(data->scale_cpu_needle,3,LV_PART_MAIN);
	lv_obj_set_style_line_rounded(data->scale_cpu_needle,true,LV_PART_MAIN);

	data->lbl_cpu=lv_label_create(ret->screen);
	lv_label_set_text(data->lbl_cpu,"cpu");
	lv_obj_set_grid_cell(data->lbl_cpu,LV_GRID_ALIGN_CENTER,2,1,LV_GRID_ALIGN_STRETCH,1,1);
	////end cpu

	data->tabview=lv_tabview_create(ret->screen);
	lv_tabview_set_tab_bar_position(data->tabview,LV_DIR_LEFT);
	lv_tabview_set_tab_bar_size(data->tabview,80);
	lv_obj_t* tab_buttons=lv_tabview_get_tab_bar(data->tabview);
	lv_obj_t* tab1=lv_tabview_add_tab(data->tabview,"cpu");
	lv_obj_t* tab2=lv_tabview_add_tab(data->tabview,"storage");

	lv_obj_set_grid_cell(data->tabview,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_STRETCH,2,2);
///storage pie chart
//使用scale画扇形图
	data->scale_storage=lv_scale_create(tab2);
	lv_scale_set_angle_range(data->scale_storage,360);
	lv_scale_set_label_show(data->scale_storage,false);
	lv_scale_set_mode(data->scale_storage,LV_SCALE_MODE_ROUND_INNER);
	lv_obj_set_style_radius(data->scale_storage,LV_RADIUS_CIRCLE,0);
	lv_scale_set_range(data->scale_storage,0,360);
	lv_scale_set_angle_range(data->scale_storage,360);
	lv_scale_set_major_tick_every(data->scale_storage,360);

	lv_obj_remove_style(data->scale_storage,NULL,LV_PART_MAIN);
	lv_obj_remove_style(data->scale_storage,NULL,LV_PART_INDICATOR);
	//lv_scale_set_total_tick_count(data->scale_storage,0);

	lv_scale_section_t* section=lv_scale_add_section(data->scale_storage);
	lv_scale_section_set_range(section,75,120);
	static lv_style_t section_style;
	lv_style_init(&section_style);
	lv_style_set_arc_color(&section_style,lv_palette_darken(LV_PALETTE_RED,3));
	lv_style_set_arc_width(&section_style,80);

	lv_scale_section_set_style(section,LV_PART_MAIN,&section_style);
///

	data->table_info=lv_table_create(tab1);

	memset(&data->cpu_stat,0,sizeof(struct linux_cpu_stat));
	data->cpu_stat_valid=0;
	data->lbl_status=create_status_bar(ret->screen);
	lv_obj_set_grid_cell(data->lbl_status,LV_GRID_ALIGN_STRETCH,0,3,LV_GRID_ALIGN_STRETCH,4,1);
	lv_obj_set_grid_cell(ret->btn_back,LV_GRID_ALIGN_STRETCH,4,1,LV_GRID_ALIGN_STRETCH,4,1);

	ret->delete_cb=on_delete_scr_linux;
	ret->update_cb=on_update_scr_linux;
	ret->userData=data;
	return ret;
}
