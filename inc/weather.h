#ifndef WEATHER_H
#define WEATHER_H


struct Weather_Station
{
    char* city;
    char* code;
    char* province;
};

void cpy_weather_station(struct Weather_Station* dst,const struct Weather_Station* src);
void del_weather_station(struct Weather_Station* station);

struct Weather_Weather
{
    float airpressure;
    float feelst;
    float humidity;
    char* img;
    char* info;
    float rain;
    float temperature;
    float temperatureDiff;
};

void del_weather_weather(struct Weather_Weather* weather);

struct Weather_Wind
{
    float degree;
    char* direct;
    char* power;
    float speed;
};

void del_weather_wnd(struct Weather_Wind* wind);
struct Weather_Real
{
    char* publish_time;
    struct Weather_Station station;
    struct Weather_Weather weather;
    struct Weather_Wind wind;
};

void del_weather_real(struct Weather_Real* real);
struct Weather_Predict_Weather
{
    char* img;
    char* info;
    char* temperature;
};
void cpy_weather_predict_weather(struct Weather_Predict_Weather* dst,const struct Weather_Predict_Weather* src);
void del_weather_predict_weather(struct Weather_Predict_Weather* );
struct Weather_Predict_Wind
{
    char* direct;
    char* power;
};
void cpy_weather_predict_wind(struct Weather_Predict_Wind* dst,const struct Weather_Predict_Wind* src);
void del_weather_predict_wind(struct Weather_Predict_Wind* );
struct Weather_Predict_Weather_Predict
{
    struct Weather_Predict_Weather weather;
    struct Weather_Predict_Wind wind;
};
void cpy_weather_predict_weather_predict(struct Weather_Predict_Weather_Predict* dst,const struct Weather_Predict_Weather_Predict* src);
void del_weather_predict_weather_predict(struct Weather_Predict_Weather_Predict*);
struct Weather_Predict_Day
{
    char* date;
    struct Weather_Predict_Weather_Predict day;
    struct Weather_Predict_Weather_Predict night;
    float precipitation;
    char* pt;
};
void cpy_weather_predict_day(struct Weather_Predict_Day* dst,const struct Weather_Predict_Day* src);
void del_weather_predict_day(struct Weather_Predict_Day*);
struct Weather_Predict
{
    struct Weather_Station station;
    char* publish_time;
    int detail_cnt;
    struct Weather_Predict_Day* detail;
};
void del_weather_predict(struct Weather_Predict*);
struct Weather{
    struct Weather_Predict predict;
    struct Weather_Real real;
};

void cpy_weather_predict(struct Weather_Predict* dst,const struct Weather_Predict* src);
void del_weather(struct Weather* );
char* get_buf(const char* cstr,int len);



//void str_weather_station(const struct Weather_Station* station,QString& out);
//void str_weather_station(const struct Weather_Station* station,QString& out);

extern const char* weather_date_format;

int weather_parser_file(const char* filePath,struct Weather* weather);
int weather_parser(const char* buf,int len,struct Weather* weather);
/**
 * 用于直接下载的内容，外面增加一层object
 * msg
 * code
 * data:实际数据
 */
int weather_parser_src(const char* buf,int len,struct Weather* weather);

char* read_file_content(const char* file_path,int* len);
#endif // WEATHER_H
