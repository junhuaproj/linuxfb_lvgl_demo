
#include "thread.h"
#include "gui.h"


void* my_thread_cb(void* p)
{
    struct my_thread* data=(struct my_thread*)p;
	int wait_update=20;
    struct my_lvgl_screen* scr=data->scr;
	while(scr->userData)
	{
		usleep(100000);
		wait_update--;
		//printf("update thread %d...\n",wait_update);
		if(wait_update<=0)
		{
			
			wait_update=data->loop(scr->userData,&data->mutex);
            if(wait_update==0)break;
		}
	}
	printf("thread exit line:%d.%d\n",__LINE__,data->id);
	pthread_exit(NULL);
	return NULL;
}

void my_thread_init(struct my_thread* arg)
{
    memset(arg,0,sizeof(struct my_thread));
    pthread_mutex_init(&arg->mutex,NULL);
}
void my_wait_thread_exit(struct my_thread* arg)
{
    pthread_join(arg->id,NULL);
}


int my_create_thread(struct my_thread* arg)//pthread_t* thread,struct my_lvgl_screen* scr,const pthread_attr_t* attr)
{
    return pthread_create(&arg->id,arg->attr,my_thread_cb,arg);
}

void net_response_buf_cb(const char* buf,int len,void* data)
{
    struct resp_buf* pBuf=(struct resp_buf*)data;
    pBuf->len=len;
    pBuf->buf=(char*)malloc(len);
    memcpy(pBuf->buf,buf,len);
}
