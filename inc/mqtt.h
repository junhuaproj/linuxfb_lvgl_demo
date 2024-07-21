/*
 * mqtt.h
 * git clone https://github.com/eclipse/paho.mqtt.c.git
 * cd paho.mqtt.c/
 * cd build
 * cmake ..
 * make
 * sudo make install
 *
 *  Created on: Jun 26, 2024
 *      Author: wang
 */

#ifndef INC_MQTT_H_
#define INC_MQTT_H_
#include "pthread.h"


struct sensor_data
{
	char dev[32];
	char ip[32];
	char datetime[24];
	float dht_temp;
	float dht_hum;
	float ds18b20;
	float bmp_ap;
	float bmp_alt;
	float bmp_temp;
    int mqtt_flag;
    char msg[128];
	struct sensor_data* next;
};

enum sensor_flag
{
    sensor_flag_dht_temp=0x01,
    sensor_flag_dht_hum=0x02,
    sensor_flag_ds18b20=0x04,
    sensor_flag_bmp_ap=0x08,
    sensor_flag_bmp_alt=0x10,
    sensor_flag_bmp_temp=0x20,
    sensor_flag_msg=0x40,
};

extern pthread_mutex_t mqttmutex;
extern struct sensor_data last_mqtt_data;

//extern char atom_pressure[];
//extern char stm32ip[];
//extern char temperature[];
//extern char altitude[];
void my_mqtt_init();
void my_mqtt_del();
void my_mqtt_publish(const char* topic,const void* msg,int len);
#endif /* INC_MQTT_H_ */
