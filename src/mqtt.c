/*
 * mqtt.c
 *
 *  Created on: Jun 26, 2024
 *      Author: wang
 */



#include "mqtt.h"
#include "MQTTClient.h"
#include "cJSON.h"
pthread_mutex_t mqttmutex;

MQTTClient client;
MQTTClient_deliveryToken deliveryToken;

MQTTClient_connectOptions connOpts=MQTTClient_connectOptions_initializer;

struct sensor_data* pmqtt_data=NULL;

struct sensor_data last_mqtt_data;

//char atom_pressure[32]={0};
//char stm32ip[32]={0};
//char temperature[32]={0};
//char altitude[32]={0};

//const char* atom_header="atomspheric pressure:";
//const char* ip_topic="/client/ip";
//const char* temp_header="bmp temp:";
//const char* altitude_header="bmp altitude:";


void delivered(void* ctx,MQTTClient_deliveryToken token)
{
	deliveryToken=token;
}

void conn_disconnect(void* ctx,char* cause)
{
	printf("disconnect %s\n",cause);
}

void add_sensor_to_que(struct sensor_data* dest,struct sensor_data* cur)
{
	while(dest->next)
	{
		dest=dest->next;
	}
	dest->next=cur;
}
int parser_mqtt_json(MQTTClient_message* msg,void* ctx)
{
	//struct sensor_data* data=(struct sensor_data*)malloc(sizeof(struct sensor_data));
	//memset(data,0,sizeof(struct sensor_data));
	cJSON* root=cJSON_ParseWithLength(msg->payload,msg->payloadlen);
	if(root==NULL)return -1;
	int ret=-2;
	struct sensor_data* data=&last_mqtt_data;
	data->mqtt_flag=0;
	cJSON* obj=cJSON_GetObjectItem(root,"dev");
	if(obj)
	{
		strcpy(data->dev,cJSON_GetStringValue(obj));
	}
	obj=cJSON_GetObjectItem(root,"ip");
	if(obj)
	{
		strcpy(data->ip,cJSON_GetStringValue(obj));
		printf("%s\n",data->ip);
	}
	obj=cJSON_GetObjectItem(root,"time");
	if(obj)
	{
		if(cJSON_IsString(obj))
			strcpy(data->datetime,cJSON_GetStringValue(obj));
		else
		{
			time_t t=cJSON_GetNumberValue(obj);
			struct tm* tm=localtime(&t);
			strftime(data->datetime,32,"%Y-%m-%d %H:%M:%S",tm);
		}
	}
	obj=cJSON_GetObjectItem(root,"ds18b20");
	if(obj)
	{
		data->ds18b20=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_ds18b20;
	}
	obj=cJSON_GetObjectItem(root,"dht_temp");
	if(obj)
	{
		data->dht_temp=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_dht_temp;
	}
	obj=cJSON_GetObjectItem(root,"dht_hum");
	if(obj)
	{
		data->dht_hum=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_dht_hum;
	}
	obj=cJSON_GetObjectItem(root,"bmp_temp");
	if(obj)
	{
		data->bmp_temp=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_bmp_temp;
	}
	obj=cJSON_GetObjectItem(root,"bmp_ap");
	if(obj)
	{
		data->bmp_ap=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_bmp_ap;
	}
	obj=cJSON_GetObjectItem(root,"bmp_alt");
	if(obj)
	{
		data->bmp_alt=cJSON_GetNumberValue(obj);
		data->mqtt_flag|=sensor_flag_bmp_alt;
	}
	obj=cJSON_GetObjectItem(root,"msg");
	if(obj)
	{
		strcpy(data->msg,cJSON_GetStringValue(obj));
		data->mqtt_flag|=sensor_flag_msg;
	}

	cJSON_Delete(root);
	printf("ip:%s\n",data->ip);
	/*if(pmqtt_data==NULL)
		pmqtt_data=data;
	else
		add_sensor_to_que(pmqtt_data,data);
		*/
	return ret;
}

int msg_arrived(void* ctx,char* topic,int len,MQTTClient_message* msg)
{
	printf("msg:\n");
	//char* msgcontent=(char*)msg->payload;
	//char* line=strchr(msgcontent,'\n');
	//msg->payloadlen
	//printf(msgcontent);
	
	pthread_mutex_lock(&mqttmutex);
	parser_mqtt_json(msg,ctx);
	/*if(line)
		line[0]='\0';
	if(strcmp(topic,ip_topic)==0)
	{
		strcpy(stm32ip,(char*)msg->payload);
		//line=strchr(stm32ip,'\n');
		//if(line)
		//	line[0]='\0';
		printf("ip:%s\n",stm32ip);
	}
	else if(strncmp(atom_header,msgcontent,strlen(atom_header))==0)
	{
		strcpy(atom_pressure,msgcontent+strlen(atom_header));
		printf("pressure:%s\n",atom_pressure);
		//line=strchr(atom_pressure,'\n');
		//if(line)
		//	line[0]='\0';
	}
	else if(strncmp(temp_header,msgcontent,strlen(temp_header))==0)
	{
		strcpy(temperature,msgcontent+strlen(temp_header));
		printf("temperature:%s\n",temperature);
	}
	else if(strncmp(altitude_header,msgcontent,strlen(altitude_header))==0)
	{
		strcpy(altitude,msgcontent+strlen(altitude_header));
		printf("altitude:%s\n",altitude);
	}
*/
	//printf("\ttopic:%s\n",topic);
	pthread_mutex_unlock(&mqttmutex);
	//printf("\tmsg:%s\n",(char*)msg->payload);
	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

void my_mqtt_init()
{
	int rc;
	printf("mqtt init\n");
	memset(&last_mqtt_data,0,sizeof(struct sensor_data));
	pthread_mutex_init(&mqttmutex,NULL);
	//MQTTClient_init_options control_conn_opts = MQTTClient_init_options_initializer;
	if((rc=MQTTClient_create(&client,"tcp://192.168.1.5:1883","debian",
			MQTTCLIENT_PERSISTENCE_NONE,NULL)!=MQTTCLIENT_SUCCESS))
	{
		printf("mqtt error\n");
		return ;
	}
	if((rc=MQTTClient_setCallbacks(client,NULL,
			conn_disconnect,msg_arrived,delivered))!= MQTTCLIENT_SUCCESS)
	{
		printf("set callback error %d\n",rc);
		return ;
	}
	connOpts.keepAliveInterval=90;
	connOpts.cleansession=1;
	connOpts.username="admin";
	connOpts.password="admin";
	connOpts.connectTimeout=1;

	if((rc=MQTTClient_connect(client,&connOpts))!=MQTTCLIENT_SUCCESS)
	{
		printf("connect error %d\n",rc);
		return ;
	}
	MQTTClient_subscribe(client,"/client/ip",1);
	MQTTClient_subscribe(client,"/client/sensor",1);
	printf("init ok\n");
}

void my_mqtt_publish(const char* topic,const void* msg,int len)
{
	if(client==NULL)return ;
	int rc;
	MQTTClient_deliveryToken token;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = msg;
	pubmsg.payloadlen = len;
	pubmsg.qos = 1;
	pubmsg.retained = 0;
	if((rc=MQTTClient_publishMessage(client,topic,&pubmsg,&token))!=MQTTCLIENT_SUCCESS)
	{
		printf("publish error:%d\n",rc);
		return ;
	}

	rc = MQTTClient_waitForCompletion(client, token, 1000);
	printf("Message with delivery token %d delivered\n", token);
}

void free_mqtt_sensor_data(struct sensor_data* data)
{
	struct sensor_data* next=data,*cur;
	while(next)
	{
		cur=next;
		next=next->next;
		free(cur);
	}
}
void my_mqtt_del()
{
	if(client)
	{
		MQTTClient_destroy(&client);
		client=NULL;
		free_mqtt_sensor_data(pmqtt_data);
		pthread_mutex_destroy(&mqttmutex);
	}
}
