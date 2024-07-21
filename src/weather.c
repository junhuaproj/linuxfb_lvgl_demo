
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "weather.h"
#include "cJSON.h"

const char* weather_date_format="yyyy-MM-dd";
char* get_buf_cjson(const cJSON* value);
int weather_parser(const char* buf,int len,struct Weather* weather);
int weather_parser_obj(cJSON* root,struct Weather* weather);
//"./weather.json"
/*int weather_parser_file(const char* filePath,struct Weather* weather)
{
    struct stat curstat;
    memset(&curstat,0,sizeof(struct stat));
    if(0!=stat(filePath,&curstat))
    {
        //no exist;
        printf("size:%d",curstat.st_size);
        return -1;
    }
    FILE* f=fopen(filePath,"r");
    if(f)
    {
        int ret=0;
        int len=curstat.st_size;
        char* buf=(char*)malloc(len);
        fread(buf,1,len,f);
        fclose(f);
        ret=weather_parser(buf,len,weather);
        free(buf);
        return ret;
    }
    return -1;
}*/

int weather_parser_file(const char* filePath,struct Weather* weather)
{
    int len=0;
    char* buf=read_file_content(filePath,&len);
    
    if(buf)
    {
        int ret=0;
        
        ret=weather_parser(buf,len,weather);
        free(buf);
        return ret;
    }
    return -1;
}

int weather_parser_src(const char* buf,int len,struct Weather* weather)
{
    cJSON* root=cJSON_ParseWithLength(buf,len);
	if(root)
	{
		if(cJSON_IsObject(root))
		{
            cJSON* data=cJSON_GetObjectItem(root,"data");
            if(data&&cJSON_IsObject(data))
			    weather_parser_obj(data,weather);
            else
            {
                cJSON_Delete(root);
                return -1;
            }
		}
		cJSON_Delete(root);
        return 1;
	}
    return -1;
}
int weather_parser(const char* buf,int len,struct Weather* weather)
{
    cJSON* root=cJSON_ParseWithLength(buf,len);
	if(root)
	{
		if(cJSON_IsObject(root))
		{
            cJSON* data=cJSON_GetObjectItem(root,"data");
            if(data&&cJSON_IsObject(data))
			    weather_parser_obj(data,weather);
            else
			    weather_parser_obj(root,weather);
		}
		cJSON_Delete(root);
        return 1;
	}
    return -1;
}
int weather_parser_station(const cJSON* obj,struct Weather_Station* station)
{
    station->city=get_buf_cjson(cJSON_GetObjectItem(obj,"city"));
    station->code=get_buf_cjson(cJSON_GetObjectItem(obj,"code"));
    station->province=get_buf_cjson(cJSON_GetObjectItem(obj,"province"));
    return 1;
}

int weather_parser_weather(const cJSON* obj,struct Weather_Weather* weather)
{
    const cJSON* prop=cJSON_GetObjectItem(obj,"airpressure");
    weather->airpressure=cJSON_GetNumberValue(prop);

    prop=cJSON_GetObjectItem(obj,"feelst");
    weather->feelst=cJSON_GetNumberValue(prop);

    prop=cJSON_GetObjectItem(obj,"humidity");
    weather->humidity=cJSON_GetNumberValue(prop);
    prop=cJSON_GetObjectItem(obj,"rain");
    weather->rain=cJSON_GetNumberValue(prop);
    prop=cJSON_GetObjectItem(obj,"temperature");
    weather->temperature=cJSON_GetNumberValue(prop);

    prop=cJSON_GetObjectItem(obj,"temperatureDiff");
    weather->temperatureDiff=cJSON_GetNumberValue(prop);

    weather->img=get_buf_cjson(cJSON_GetObjectItem(obj,"img"));
    weather->info=get_buf_cjson(cJSON_GetObjectItem(obj,"info"));
    return 1;
}


int weather_parser_wind(const cJSON* obj,struct Weather_Wind* wind)
{
    const cJSON* prop=cJSON_GetObjectItem(obj,"degree");
    wind->degree=cJSON_GetNumberValue(prop);

    prop=cJSON_GetObjectItem(obj,"speed");
    wind->speed=cJSON_GetNumberValue(prop);;
    wind->direct=get_buf_cjson(cJSON_GetObjectItem(obj,"direct"));
    wind->power=get_buf_cjson(cJSON_GetObjectItem(obj,"power"));
    return 1;
}

int weather_parser_real(const cJSON* obj,struct Weather_Real* real)
{
    real->publish_time=get_buf_cjson(cJSON_GetObjectItem(obj,"publish_time"));
    cJSON* value=cJSON_GetObjectItem(obj,"station");
    if(cJSON_IsObject(value))
    {
        weather_parser_station(value,&real->station);
    }
    value=cJSON_GetObjectItem(obj,"weather");
    if(cJSON_IsObject(value))
    {
        weather_parser_weather(value,&real->weather);
    }
    value=cJSON_GetObjectItem(obj,"wind");
    if(cJSON_IsObject(value))
    {
        weather_parser_wind(value,&real->wind);
    }
    return 1;
}


int weather_parser_predict_day_weather(const cJSON* obj, struct Weather_Predict_Weather* weather)
{
    weather->img=get_buf_cjson(cJSON_GetObjectItem(obj,"img"));
    weather->info=get_buf_cjson(cJSON_GetObjectItem(obj,"info"));
    weather->temperature=get_buf_cjson(cJSON_GetObjectItem(obj,"temperature"));
    return 1;
}
int weather_parser_predict_day_wind(const cJSON* obj, struct Weather_Predict_Wind* wind)
{
    wind->direct=get_buf_cjson(cJSON_GetObjectItem(obj,"direct"));
    wind->power=get_buf_cjson(cJSON_GetObjectItem(obj,"power"));
    return 1;
}

int weather_parser_predict_day_day(const cJSON* obj, struct Weather_Predict_Weather_Predict* weather)
{
    cJSON* value=cJSON_GetObjectItem(obj,"weather");
    if(cJSON_IsObject(value))
        weather_parser_predict_day_weather(value,&weather->weather);

    value=cJSON_GetObjectItem(obj,"wind");
    if(cJSON_IsObject(value))
        weather_parser_predict_day_wind(value,&weather->wind);
    return 1;
}


int weather_parser_predict_day(const cJSON* obj, struct Weather_Predict_Day* day)
{
    const cJSON* prop=cJSON_GetObjectItem(obj,"precipitation");
    day->date=get_buf_cjson(cJSON_GetObjectItem(obj,"date"));
    day->precipitation=cJSON_GetNumberValue(prop);
    day->pt=get_buf_cjson(cJSON_GetObjectItem(obj,"pt"));
    const cJSON* value=cJSON_GetObjectItem(obj,"day");
    if(cJSON_IsObject(value))
        weather_parser_predict_day_day(value,&day->day);
    value=cJSON_GetObjectItem(obj,"night");
    if(cJSON_IsObject(value))
        weather_parser_predict_day_day(value,&day->night);
    return 1;
}

int weather_praser_predict(const cJSON* obj,struct Weather_Predict* predict)
{
    predict->publish_time=get_buf_cjson(cJSON_GetObjectItem(obj,"publish_time"));
    cJSON* value=cJSON_GetObjectItem(obj,"station");
    if(cJSON_IsObject(value))
    {
        weather_parser_station(value,&predict->station);
    }
    value=cJSON_GetObjectItem(obj,"detail");
    if(cJSON_IsArray(value))
    {
        //QJsonArray arr=value.toArray();
        predict->detail_cnt=cJSON_GetArraySize(value);
        predict->detail=(struct Weather_Predict_Day*)malloc(sizeof(struct Weather_Predict_Day)*predict->detail_cnt);
        memset(predict->detail,0,sizeof(struct Weather_Predict_Day)*predict->detail_cnt);
        for(int i=0;i<predict->detail_cnt;i++)
        {
            weather_parser_predict_day(cJSON_GetArrayItem(value,i),predict->detail+i);
        }
    }
    return 1;
}
int weather_parser_obj(cJSON* root,struct Weather* weather)
{
    cJSON* value=cJSON_GetObjectItem(root,"real");
    if(cJSON_IsObject(value))
    {
        weather_parser_real(value,&weather->real);
    }
    else return 0;
    value=cJSON_GetObjectItem(root,"predict");
    if(cJSON_IsObject(value))
    {
        weather_praser_predict(value,&weather->predict);
    }
    else return 0;
    return 1;
}


void del_weather_station(struct Weather_Station* station)
{
    if(station->city)
        free(station->city);
    if(station->code)
        free(station->code);
    if(station->province)
        free(station->province);
    memset(station,0,sizeof(struct Weather_Station));
}

void del_weather_weather(struct Weather_Weather* weather)
{
    if(weather->img)
        free(weather->img);
    if(weather->info)
        free(weather->info);
    memset(weather,0,sizeof(struct Weather_Weather));
}
void del_weather_wnd(struct Weather_Wind* wind)
{
    if(wind->direct)
        free(wind->direct);
    if(wind->power)
        free(wind->power);
    memset(wind,0,sizeof(struct Weather_Wind));
}

void del_weather_real(struct Weather_Real* real)
{
    if(real->publish_time)
    {
        free(real->publish_time);
        //real.publish_time=NULL;
    }
    del_weather_station(&real->station);
    del_weather_weather(&real->weather);
    del_weather_wnd(&real->wind);
    memset(real,0,sizeof(struct Weather_Real));
}
void del_weather_predict_weather(struct Weather_Predict_Weather* weather)
{
    if(weather->img)
        free(weather->img);
    if(weather->info)
        free(weather->info);
    if(weather->temperature)
        free(weather->temperature);
    memset(weather,0,sizeof(struct Weather_Predict_Weather));
}
void del_weather_predict_wind(struct Weather_Predict_Wind* wind)
{
    if(wind->direct)
        free(wind->direct);
    if(wind->power)
        free(wind->power);
    memset(wind,0,sizeof(struct Weather_Predict_Wind));
}

void del_weather_predict_weather_predict(struct Weather_Predict_Weather_Predict* predict)
{
    del_weather_predict_weather(&predict->weather);
    del_weather_predict_wind(&predict->wind);
    memset(predict,0,sizeof(struct Weather_Predict_Weather_Predict));
}
void del_weather_predict_day(struct Weather_Predict_Day* day)
{
    if(day->date)
        free(day->date);
    if(day->pt)
        free(day->pt);
    del_weather_predict_weather_predict(&day->day);
    del_weather_predict_weather_predict(&day->night);
    memset(day,0,sizeof(struct Weather_Predict_Day));
}

void del_weather_predict(struct Weather_Predict* predict)
{
    del_weather_station(&predict->station);
    if(predict->publish_time)
        free(predict->publish_time);
    if(predict->detail)
    {
        for(int i=0;i<predict->detail_cnt;i++)
        {
            del_weather_predict_day(predict->detail+i);
        }
        free(predict->detail);
    }
    memset(predict,0,sizeof(struct Weather_Predict));
}
void del_weather(struct Weather* weather)
{
    del_weather_real(&weather->real);
    del_weather_predict(&weather->predict);
}

char* get_buf(const char* cstr,int len)
{
    if(len<0)
        len=strlen(cstr);
    char* p=(char*)malloc(len+1);
    strncpy(p,cstr,len);
    p[len]=0;
    return p;
}
char* get_buf_cjson(const cJSON* value)
{
    return get_buf(cJSON_GetStringValue(value),-1);
}
void cpy_weather_predict(struct Weather_Predict* dst,const struct Weather_Predict* src)
{
    //memcpy(dst,src,sizeof(struct Weather_Predict));
    dst->detail_cnt=src->detail_cnt;
    dst->publish_time=get_buf(src->publish_time,-1);
    cpy_weather_station(&dst->station,&src->station);
    dst->detail=(struct Weather_Predict_Day*)malloc(sizeof(struct Weather_Predict_Day)*dst->detail_cnt);
    for(int i=0;i<dst->detail_cnt;i++)
    {
        cpy_weather_predict_day(dst->detail+i,src->detail+i);
    }
}
void cpy_weather_station(struct Weather_Station* dst,const struct Weather_Station* src)
{
    dst->city=get_buf(src->city,-1);
    dst->code=get_buf(src->code,-1);
    dst->province=get_buf(src->province,-1);
}

void cpy_weather_predict_weather(struct Weather_Predict_Weather* dst,const struct Weather_Predict_Weather* src)
{
    dst->img=get_buf(src->img,-1);
    dst->info=get_buf(src->info,-1);
    dst->temperature=get_buf(src->temperature,-1);
}
void cpy_weather_predict_wind(struct Weather_Predict_Wind* dst,const struct Weather_Predict_Wind* src)
{
    dst->direct=get_buf(src->direct,-1);
    dst->power=get_buf(src->power,-1);
}
void cpy_weather_predict_weather_predict(struct Weather_Predict_Weather_Predict* dst,const struct Weather_Predict_Weather_Predict* src)
{
    cpy_weather_predict_weather(&dst->weather,&src->weather);
    cpy_weather_predict_wind(&dst->wind,&src->wind);
}

void cpy_weather_predict_day(struct Weather_Predict_Day* dst,const struct Weather_Predict_Day* src)
{
    cpy_weather_predict_weather_predict(&dst->day,&src->day);
    cpy_weather_predict_weather_predict(&dst->night,&src->night);
    dst->date=get_buf(src->date,-1);
    dst->precipitation=src->precipitation;
    dst->pt=get_buf(src->pt,-1);
}
