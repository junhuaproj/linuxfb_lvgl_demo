/*
 * stock.h
 *
 *  Created on: Jun 25, 2024
 *      Author: wang
 */

#ifndef INC_STOCK_H_
#define INC_STOCK_H_


struct stock_info
{
	char** values;
	int count;
	/*char* v01;
	char* v02;
	char* v03;
	char* v04;
	char* v05;
	char* v06;
	char* v07;
	char* v08;
	char* v09;
	char* v10;
	char* v11;
	char* v12;*/
};

struct stock_minute_trade
{
	char hhmm[8];
	float price;
	int vol;
};
struct stock_minute{
	float preClose;
	float open;
	float high;
	float low;
	float trade_vol;
	int min_count;
	struct stock_minute_trade* minutes;
};

struct stock_day_minute
{
	int cnt;
	struct stock_minute* days;
};


extern int minute_updated;
//extern struct stock_minute* stock_minute_line;
extern struct stock_info* stocks;
extern int stock_updated;
extern int stock_count;
//void init_minute_line();
//void deinit_minute_line();
void init_minute_line(struct stock_minute* p,int min_count);
void deinit_minute_line(struct stock_minute* p);
void clear_day_minute_line(struct stock_day_minute * p);


void delete_stock(struct stock_info*);
void delete_stocks(struct stock_info* s,int count);
//void parser_stock_json(const char* buf,int len);
struct stock_info* parser_stock_json(const char* buf,int len,int* count);
typedef void (*net_response_cb)(const char* buf,int len,void* data);

#endif /* INC_STOCK_H_ */
