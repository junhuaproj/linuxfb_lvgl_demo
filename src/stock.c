/*
 * stock.c
 *
 *  Created on: Jun 26, 2024
 *      Author: wang
 */

#include "stock.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

extern struct stock_info* stocks;
char* new_attribute_info(cJSON* value);

int minute_updated=0;
void main_parser_list(const char* buf,int len,void* data)
{
	if(stocks)
	{
		delete_stocks(stocks,stock_count);
		stock_count=0;
		stocks=NULL;

	}
	stock_updated=1;
	stocks=parser_stock_json(buf,len,&stock_count);
}

//struct stock_minute* stock_minute_line=NULL;
void init_minute_line(struct stock_minute* p,int min_count)
{
	memset(p,0,sizeof(struct stock_minute));
	p->min_count=min_count;
	p->minutes=(struct stock_minute_trade*)malloc(sizeof(struct stock_minute_trade)*p->min_count);
	memset(p->minutes,0,sizeof(struct stock_minute_trade)*p->min_count);
}

void clear_day_minute_line(struct stock_day_minute * p)
{
	//memset(p->minutes,0,sizeof(struct stock_minute_trade)*p->min_count);
	if(p->days)
	{
		for(int i=0;i<p->cnt;i++)
		{
			deinit_minute_line(p->days+i);
		}
		free(p->days);
		p->days=NULL;
		p->cnt=0;
	}
}

void deinit_minute_line(struct stock_minute* p)
{
	//if(stock_minute_line==NULL)
	//	return ;
	free(p->minutes);
	memset(p,0,sizeof(struct stock_minute));
	//free(stock_minute_line);
	//stock_minute_line=NULL;
}
void detail_parser_minute2(cJSON* arr,struct stock_minute* minute)
{
	int size=cJSON_GetArraySize(arr);
	cJSON* row,*cell;
	int index;
	//clear_minute_line();

	if(size==0)
		return ;
	if(size<240)
		init_minute_line(minute,240);
	else
		init_minute_line(minute,size);
	row=cJSON_GetArrayItem(arr,0);
	cell=cJSON_GetArrayItem(row,0);
	minute->preClose=atof(cJSON_GetStringValue(cell));
	cell=cJSON_GetArrayItem(row,1);
	minute->open=atof(cJSON_GetStringValue(cell));
	cell=cJSON_GetArrayItem(row,2);
	minute->high=atof(cJSON_GetStringValue(cell));
	cell=cJSON_GetArrayItem(row,3);
	minute->low=atof(cJSON_GetStringValue(cell));
	cell=cJSON_GetArrayItem(row,4);
	minute->trade_vol=atof(cJSON_GetStringValue(cell));
	for(index=1;index<size&&index<minute->min_count;index++)
	{

		row=cJSON_GetArrayItem(arr,index);
		cell=cJSON_GetArrayItem(row,0);
		//printf("index:%d,%s ",index,cJSON_GetStringValue(cell));
		strcpy(minute->minutes[index-1].hhmm,cJSON_GetStringValue(cell));
		cell=cJSON_GetArrayItem(row,1);

		minute->minutes[index-1].price=atof(cJSON_GetStringValue(cell));
		cell=cJSON_GetArrayItem(row,3);
		minute->minutes[index-1].vol=atof(cJSON_GetStringValue(cell));
		//printf("index:%d,%.2f",index,stock_minute_line->minutes[index-1].price);
	}
}
void detail_parser_minute(struct stock_day_minute* days,const char* buf,int len,void* data,int dayType)
{
	cJSON* root=cJSON_ParseWithLength(buf,len);
	if(root)
	{
		if(cJSON_IsArray(root))
		{
			if(dayType==0)
			{
				clear_day_minute_line(days);
				days->cnt=1;
				days->days=(struct stock_minute*)malloc(sizeof(struct stock_minute)*1);
				//init_minute_line(days->days);
				detail_parser_minute2(root,days->days);
			}
			else if(dayType==1)
			{
				int count=cJSON_GetArraySize(root);
				cJSON* day;
				clear_day_minute_line(days);
				days->cnt=count;
				days->days=(struct stock_minute*)malloc(sizeof(struct stock_minute)*days->cnt);
				cJSON* prop;
				for(int i=0;i<count;i++)
				{
					day=cJSON_GetArrayItem(root,i);
					prop=cJSON_GetObjectItem(day,"date");
					//init_minute_line(days->days+i);
					prop=cJSON_GetObjectItem(day,"minutes");
					detail_parser_minute2(prop,days->days+i);
				}
			}
			minute_updated=1;
		}
		
		cJSON_Delete(root);
	}
}

char* new_attribute_info(cJSON* value)
{
	char* cV=cJSON_GetStringValue(value);
	char* ret=(char*)malloc(strlen(cV)+1);
	strcpy(ret,cV);
	return ret;
}

void delete_stock(struct stock_info* s)
{
	int i=0;
	for(i=0;i<s->count;i++)
	{
		free(s->values[i]);
	}
	free(s->values);
	s->values=NULL;
	/*if(s->v01)free(s->v01);
	if(s->v02)free(s->v02);
	if(s->v03)free(s->v03);
	if(s->v04)free(s->v04);
	if(s->v05)free(s->v05);
	if(s->v06)free(s->v06);
	if(s->v07)free(s->v07);
	if(s->v08)free(s->v08);
	if(s->v09)free(s->v09);
	if(s->v10)free(s->v10);
	if(s->v11)free(s->v11);
	if(s->v12)free(s->v12);*/
}





void delete_stocks(struct stock_info* s,int count)
{
	int i=0;
	for(i=0;i<count;i++)
	{
		delete_stock(s+i);
	}
	free(s);
}

struct stock_info* parser_stock_json(const char* buf,int len,int* count)
{
	struct stock_info* ret=NULL;
	cJSON* root=cJSON_ParseWithLength(buf,len);
	//cJSON* stock;
	cJSON* prop;
	int size,i=0,col,sizeCol;
	if(root==NULL)
	{
		goto last;
	}
	if(!cJSON_IsObject(root))
	{
		printf("no obj\n");
		goto last;
	}
	cJSON *current_element = root->child;

	size=0;
    while ((current_element != NULL) && (current_element->string != NULL))
    {
    	
		size++;
		current_element = current_element->next;
    }

	//return NULL;
	//size=cJSON_GetArraySize(root);
	if(size==0)
		goto last;
	*count=size;
	ret=(struct stock_info*)malloc(sizeof(struct stock_info)*size);
	current_element = root->child;
	while ((current_element != NULL) && (current_element->string != NULL))
	{
		//stock=cJSON_GetArrayItem(root,i);
		printf("%s\n",current_element->string);
		sizeCol=cJSON_GetArraySize(current_element);
		ret[i].count=sizeCol;
		ret[i].values=(char**)malloc(sizeof(char*)*sizeCol);
		for(col=0;col<sizeCol;col++)
		{
			prop=cJSON_GetArrayItem(current_element,col);
			if(prop==NULL)
			{
				ret[i].values[col]=NULL;
			}
			else
			{
				ret[i].values[col]=new_attribute_info(prop);
			}
		}
		current_element = current_element->next;
		i++;
	}
last:
	if(root)
		cJSON_Delete(root);
	return ret;
}
