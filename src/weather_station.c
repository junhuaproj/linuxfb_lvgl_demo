#include "weather_station.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include <sys/stat.h>
#include "weather.h"

void del_station_key_val_arr(struct station_key_value_arr* arr)
{
    if(arr->val)
    {
        int i;
        for(i=0;i<arr->cnt;i++)
            clear_station_key_value(arr->val+i);
        free(arr->val);

    }
    memset(arr,0,sizeof(struct station_key_value_arr));
}

void clear_station_key_value(struct station_key_value* key)
{
    if(key->code)
        free(key->code);
    if(key->name)
        free(key->name);
#ifdef _STATION_URL_
    if(key->url)
        free(key->url);
#endif
    //memset(key,0,STATION_VALUE_LEN);
}

typedef void (*parser_weather_station_cb)(cJSON* obj,struct station_key_value* val);

static void parser_weather_province(cJSON* obj,struct station_key_value* val)
{
    cJSON* attr=cJSON_GetObjectItem(obj,"code");
    val->code=get_buf(cJSON_GetStringValue(attr),-1);
    attr=cJSON_GetObjectItem(obj,"name");
    val->name=get_buf(cJSON_GetStringValue(attr),-1);
#ifdef _STATION_URL_    
    attr=cJSON_GetObjectItem(obj,"url");
    val->url=get_buf(cJSON_GetStringValue(attr),-1);
#endif
}

static void parser_weather_city(cJSON* obj,struct station_key_value* val)
{
    cJSON* attr=cJSON_GetObjectItem(obj,"code");
    val->code=get_buf(cJSON_GetStringValue(attr),-1);
    attr=cJSON_GetObjectItem(obj,"city");
    val->name=get_buf(cJSON_GetStringValue(attr),-1);
#ifdef _STATION_URL_    
    attr=cJSON_GetObjectItem(obj,"url");
    val->url=get_buf(cJSON_GetStringValue(attr),-1);
#endif
}


int load_weather_key_val_arr(const char* buf,int len,struct station_key_value_arr* arr,parser_weather_station_cb cb)
{
    cJSON* root=cJSON_ParseWithLength(buf,len);
    int ret=-1,i;
    if(root==NULL)
    {
        printf("json error\n");
        return -1;
    }
    if(!cJSON_IsArray(root))
        goto last;
    arr->cnt=cJSON_GetArraySize(root);
    arr->val=(struct station_key_value*)malloc(sizeof(struct station_key_value)*arr->cnt);
    for(i=0;i<arr->cnt;i++)
    {
        cb(cJSON_GetArrayItem(root,i),arr->val+i);
    }
    ret=1;
last:
    if(root)
        cJSON_Delete(root);
    return ret;
}

char* read_file_content(const char* file_path,int* len)
{
    struct stat curstat;
    memset(&curstat,0,sizeof(struct stat));
    printf("read file content...\n");
    if(0!=stat(file_path,&curstat))
    {
        printf("stat %s\n",file_path);
        //no exist;
        //printf("size:%d",curstat.st_size);
        return NULL;
    }
    printf("read file len %d\n",curstat.st_size);
    FILE* f=fopen(file_path,"r");
    if(f)
    {
        int ret=0;
        *len=curstat.st_size;
        char* buf=(char*)malloc(*len);
        fread(buf,1,*len,f);
        fclose(f);
        return buf;
    }
    return NULL;
}
int load_weather_station_province_buf(struct station_key_value_arr* arr,const char* buf,int len)
{
    return load_weather_key_val_arr(buf,len,arr,parser_weather_province);
}
int load_weather_station_city_buf(struct station_key_value_arr* arr,const char* buf,int len)
{
    return load_weather_key_val_arr(buf,len,arr,parser_weather_city);
}
int load_weather_station_city(struct station_key_value_arr* arr,const char* file_path)
{
    int len=0;
    char* buf=read_file_content(file_path,&len);
    if(buf)
    {
        int ret=0;
        ret=load_weather_key_val_arr(buf,len,arr,parser_weather_city);
        free(buf);
        return ret;
    }
    return -1;
}
int load_weather_station_province(struct station_key_value_arr* arr,const char* file_path)
{
    int len=0;
    char* buf=read_file_content(file_path,&len);
    if(buf)
    {
        int ret=0;
        ret=load_weather_key_val_arr(buf,len,arr,parser_weather_province);
        free(buf);
        return ret;
    }
    /*struct stat curstat;
    memset(&curstat,0,sizeof(struct stat));
    if(0!=stat(file_path,&curstat))
    {
        //no exist;
        printf("size:%d",curstat.st_size);
        return -1;
    }
    FILE* f=fopen(file_path,"r");
    if(f)
    {
        int ret=0;
        int len=curstat.st_size;
        char* buf=(char*)malloc(len);
        fread(buf,1,len,f);
        fclose(f);
        ret=load_weather_key_val_arr(buf,len,arr,parser_weather_province);
        free(buf);
        return ret;
    }*/
    return -1;
}

int save_to_file(const char* buf,int len,const char* file_path)
{
    FILE* f=fopen(file_path,"w");
    if(f)
    {
        printf("write ...\n");
        fwrite(buf,1,len,f);
        fclose(f);
        printf("write ok\n");
        return 1;
    }
    return -1;
}
int load_station(struct station_key_value* station,const char* file_path)
{
    int len=0;
    char* buf=read_file_content(file_path,&len);
    if(buf)
    {
        cJSON* root=cJSON_ParseWithLength(buf,len);
        
        int ret=-1;
        if(root){
            cJSON* attr=cJSON_GetObjectItem(root,"code");
            station->code=get_buf(cJSON_GetStringValue(attr),-1);
            attr=cJSON_GetObjectItem(root,"name");
            station->name=get_buf(cJSON_GetStringValue(attr),-1);
            ret=1;
            cJSON_Delete(root);
        }
        free(buf);
        return ret;
    }
    return -1;
}
int save_station(struct station_key_value* station,const char* file_path)
{
    cJSON* root=cJSON_CreateObject();
    cJSON_AddStringToObject(root,"code",station->code);
    cJSON_AddStringToObject(root,"name",station->name);
    char* buf=cJSON_Print(root);
    save_to_file(buf,strlen(buf),file_path);
    free(buf);
    cJSON_Delete(root);
}
