
#include "stock.h"
#include "cJSON.h"
#include <curl/curl.h>
#include "net.h"

struct stock_info* stocks=NULL;
int stock_count=0;
int stock_updated=0;

size_t response_header_cb(void* ptr,size_t size,size_t nmemb,void* stream)
{
    printf("%s\n",(char*)ptr);
    //stResponse* response=(stResponse*)stream;
    return size* nmemb;
}

size_t response_boday_cb(void* ptr,size_t size,size_t nmemb,void* stream)
{
    struct resp_buf* data=(struct resp_buf*)stream;
    if(data->len)
    {
        int len=data->len;
        void *buf=data->buf;
        data->len=len+size*nmemb;
        data->buf=(char*)malloc(data->len);
        memcpy(data->buf,buf,len);
        memcpy(data->buf+len,ptr,size*nmemb);
        free(buf);
    }
    else
    {
        data->len=size*nmemb;
        data->buf=(char*)malloc(data->len);
        memcpy(data->buf,ptr,data->len);
    }
    printf("len=%d\n",size*nmemb);
    //stResponse* response=(stResponse*)stream;
    return size* nmemb;
}
void request_url(const char* host,const char* port,const char* path,struct resp_buf* data)
//void request_url(const char* host,const char* port,const char* path,net_response_cb cb,void* data)
{
    char url[256];
    CURL* curl=curl_easy_init();
    sprintf(url,"http://%s:%s%s",host,port,path);
    printf("url:%s\n",url);
    curl_easy_setopt(curl,CURLOPT_URL,url);
    //ssl
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    struct curl_slist* headers=curl_slist_append(NULL,"Content-Type:application/json");
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
    //read
    curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,response_header_cb);
    curl_easy_setopt(curl,CURLOPT_HEADERDATA,NULL);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,response_boday_cb);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,data);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,10);
    CURLcode res=curl_easy_perform(curl);
    if(res==CURLE_OK)
    {
        int response_code=0;
        curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&response_code);
        printf("request code=%d\n",response_code);
    }
    else
        printf("request error %d\n",res);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

}
