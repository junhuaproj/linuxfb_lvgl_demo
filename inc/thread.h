#ifndef __THREAD_H__
#define __THREAD_H__
#include "pthread.h"
/**
 * return 0:exit thread,>0 循环间隔,
 *  */
typedef int (*my_thread_loop_cb)(void* p,pthread_mutex_t* mutex );

struct my_thread
{
    struct my_lvgl_screen* scr;
    pthread_mutex_t mutex;
    pthread_t id;
    const pthread_attr_t* attr;

    my_thread_loop_cb loop;
};

void my_thread_init(struct my_thread* arg);
void my_wait_thread_exit(struct my_thread* arg);
int my_create_thread(struct my_thread* arg);


struct resp_buf
{
    unsigned char* buf;
    int len;
};

void net_response_buf_cb(const char* buf,int len,void* data);

#endif