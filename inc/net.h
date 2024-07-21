/*
 * net.h
 *
 *  Created on: Jun 25, 2024
 *      Author: wang
 */

#ifndef NET_H_
#define NET_H_

#include "stock.h"
#include "thread.h"
//typedef void (*net_response_cb)(const char* buf,int len,void* data);
void main_parser_list(const char* buf,int len,void* data);
//void detail_parser_minute(const char* buf,int len,void* data);
void detail_parser_minute(struct stock_day_minute* days,const char* buf,int len,void* data,int dayType);
//void request_url(const char* host,const char* port,const char* path,net_response_cb,void* data);
void request_url(const char* host,const char* port,const char* path,struct resp_buf* data);

#endif /* NET_H_ */
