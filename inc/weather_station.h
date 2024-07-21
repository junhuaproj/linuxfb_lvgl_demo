
#ifndef __WEATHER_STATION_H__
#define __WEATHER_STATION_H__
//#define _STATION_URL_
struct station_key_value
{
    char* code;
    char* name;
#ifdef _STATION_URL_    
    char* url;
#endif
};

struct station_key_value_arr
{
    int cnt;
    struct station_key_value* val;
};

#define STATION_VALUE_LEN   sizeof(struct station_key_value)

extern const char* station_cfg;
void clear_station_key_value(struct station_key_value* key);
void del_station_key_val_arr(struct station_key_value_arr* arr);


void clear_station_key_value(struct station_key_value* key);

int load_weather_station_province(struct station_key_value_arr* arr,const char* file_path);
int load_weather_station_city(struct station_key_value_arr* arr,const char* file_path);

int load_weather_station_province_buf(struct station_key_value_arr* arr,const char* buf,int len);
int load_weather_station_city_buf(struct station_key_value_arr* arr,const char* buf,int len);

int save_to_file(const char* buf,int len,const char* file_path);
int save_station(struct station_key_value* station,const char* file_path);
int load_station(struct station_key_value* station,const char* file_path);
#endif
