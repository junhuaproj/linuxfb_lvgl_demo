/*
 * linux_stat.c
 *
 *  Created on: Jul 1, 2024
 *      Author: wang
 */



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#include "linux_stat.h"
#define CPU_BUF_LEN	256
struct key_value
{
	char* key;
	char* value;
};
#define KEY_VALUE_LEN	sizeof(struct key_value)

void del_key_value(struct key_value* value)
{
	if(value->key)
		free(value->key);
	if(value->value)
		free(value->value);
}
char* get_new_buf(const char* value,int len)
{
	if(len<0)
		len=strlen(value);
	char* p=(char*)malloc(len+1);
	strncpy(p,value,len);
	p[len]=0;
	return p;
}
void set_key_value(struct key_value* value,const char* s,int len)
{
	if(value->value)
		free(value->value);
	
	value->value=get_new_buf(s,len);
}
void set_key_key(struct key_value* value,const char* s,int len)
{
	if(value->key)
		free(value->key);
	
	value->key=get_new_buf(s,len);
}

void set_buf_value(char* dest,int max_dest_len,const char* src)
{
	int src_len=strlen(src);
	if(src_len<max_dest_len)
		strcpy(dest,src);
	else
	{
		strncpy(dest,src,max_dest_len-1);
		dest[max_dest_len-1]=0;
	}
}

void sscan_cpu_info(const char* buf,int buf_len,struct linux_cpu_stat *cpu)
{
	char value[20];
	int cur_len=0,pos=0,value_index=0;
	memset(value,0,20);
	while(pos<buf_len&&buf[pos])
	{
		if(isblank(buf[pos])&&cur_len<19)
		{
			value[cur_len]=0;
			//printf("value:%s\n",value);

			if(value[0])
			{
				switch(value_index)
				{
				case 0:
					strcpy(cpu->name,value);
					//printf("cpu:%s\n",cpu->name);
					break;
				case 1:
					cpu->user=atoi(value);break;
				case 2:
					cpu->nice=atoi(value);break;
				case 3:
					cpu->system=atoi(value);break;
				case 4:
					cpu->idle=atoi(value);break;
				case 5:
					cpu->lowait=atoi(value);break;
				case 6:
					cpu->irq=atoi(value);break;
				case 7:
					cpu->softirq=atoi(value);break;
				}
				value_index++;
			}
			cur_len=0;
			
			memset(value,0,20);
		}
		else
		{
			value[cur_len]=buf[pos];
			cur_len++;
		}
		pos++;

	}

	/*sscanf(buf,"%s %u %u %u %u %u %u %u",cpu->name,
			cpu->user,cpu->nice,cpu->system,
			cpu->idle,cpu->lowait,
			cpu->irq,cpu->softirq);*/
}
unsigned int linux_get_cpu_busy(const struct linux_cpu_stat* cpu)
{
	return cpu->irq+cpu->lowait
	+cpu->nice+cpu->softirq+cpu->system+cpu->user;
}
void print_cpu(const struct linux_cpu_stat *cpu)
{
	printf("%s  %u %u %u %u %u %u %u\n",cpu->name,
			cpu->user,cpu->nice,cpu->system,
			cpu->idle,cpu->lowait,
			cpu->irq,cpu->softirq);
}
int linux_read_cpu_stat(struct linux_cpu_stat* cpu_stat)
{
	int ret=-1;
	FILE* fd=fopen("/proc/stat","r");
	if(fd!=NULL)
	{
		char buf[CPU_BUF_LEN];
		//printf("open ok\n");
		int cpu_idx=-1;
		while(fgets(buf,CPU_BUF_LEN,fd)&&cpu_idx<cpu_stat->cpu_cnt)
		{
			//printf("read:%s\n",buf);
			if(strncmp(buf,"cpu",3)==0)
			{
				if(cpu_idx<0)
				{
					sscan_cpu_info(buf,CPU_BUF_LEN,cpu_stat);
					cpu_idx=0;
				}
				else
				{
					sscan_cpu_info(buf,CPU_BUF_LEN,cpu_stat->cpus+cpu_idx);
					cpu_idx++;
				}
				//print_cpu(&cpu_stat);
			}
			else break;
		}
		fclose(fd);
		ret=0;
	}
	return ret;
}
//////////////////////////////////

enum cpu_info_type{
	cpu_info_none=0,
	cpu_info_processor=0x0001,
	//cpu_info_vendor=0x0002,
	//cpu_info_family=0x0004,
	//cpu_info_model=0x0008,
	cpu_info_model_name=0x0010,
	//cpu_info_mhz=0x0020,
	//cpu_info_cache=0x0040,
	//cpu_info_physical=0x0080,
	//cpu_info_siblings=0x0100,
	//cpu_info_core_id=0x0200,
	//cpu_info_cpu_cores=0x0400,
	cpu_info_all=0x11,//0x07ff,
};
struct linux_cpu_info* create_linux_cpu_info()
{
	struct linux_cpu_info* p=(struct linux_cpu_info*)malloc(sizeof(struct linux_cpu_info));
	memset(p,0,sizeof(struct linux_cpu_info));
	return  p;
}

void del_cpu_info(struct linux_cpu_info* p)
{
	struct linux_cpu_info* head=p,*next;
	while(head)
	{
		next=head->next;
		printf("free cpu %d\n",head->processor);
		free(head);
		head=next;
	}
}

int cpu_info_parser(const char* buf,struct key_value* value)
{
	const char* sep=strchr(buf,':');
	//char name[24],value[64];
	const char* nameend=strchr(buf,'\t');
	int len;
	if(sep==NULL||nameend==NULL)return -1;
	len=nameend-buf;
	set_key_key(value,buf,len);
	//strncpy(name,buf,len);
	//name[len]=0;
	len=strlen(sep+2);
	//if(len>63)
	//	len=63;
	//strncpy(value,sep+2,len);
	//value[len]=0;
	set_key_value(value,sep+2,len);
	//printf("%s:%s\n",value->key,value->value);
	return 1;
}
enum cpu_info_type cpu_key_value_to_info(const struct key_value* value,struct linux_cpu_info* info)
{
	if(strcmp(value->key,"processor")==0)
	{
		info->processor=atoi(value->value);
		return cpu_info_processor;
	}
	/*else if(strcmp(value->key,"vendor_id")==0)
	{
		set_buf_value(info->vendor_id,32,value->value);
		return cpu_info_vendor;
	}
	else if(strcmp(value->key,"cpu family")==0)
	{
		set_buf_value(info->cpu_family,8,value->value);
		return cpu_info_family;
	}
	else if(strcmp(value->key,"model")==0)
	{
		set_buf_value(info->model,8,value->value);
		return cpu_info_model;
	}*/
	else if(strcmp(value->key,"model name")==0)
	{
		set_buf_value(info->model_name,64,value->value);
		return cpu_info_model_name;
	}
	/*else if(strcmp(value->key,"cpu MHz")==0)
	{
		set_buf_value(info->cpu_MHz,16,value->value);
		return cpu_info_mhz;
	}
	else if(strcmp(value->key,"cache size")==0)
	{
		set_buf_value(info->cache_size,16,value->value);
		return cpu_info_cache;
	}
	else if(strcmp(value->key,"physical id")==0)
	{
		info->physical_id=atoi(value->value);
		return cpu_info_physical;
	}
	else if(strcmp(value->key,"siblings")==0)
	{
		info->siblings=atoi(value->value);
		return cpu_info_siblings;
	}
	else if(strcmp(value->key,"core id")==0)
	{
		info->core_id=atoi(value->value);
		return cpu_info_core_id;
	}
	else if(strcmp(value->key,"cpu cores")==0)
	{
		info->cpu_cores=atoi(value->value);
		return cpu_info_cpu_cores;
	}*/
	else
	{
		//printf("cpu error %s\n",value->key);
		return cpu_info_none;
	}
}

struct linux_cpu_info* read_cpu_info(FILE* fd)
{
	char buf[CPU_BUF_LEN];
	int line;
	char* end;
	struct key_value value;
	struct linux_cpu_info* pInfo=create_linux_cpu_info();
	unsigned int attributes=0;
	memset(&value,0,KEY_VALUE_LEN);
	printf("%s\n",__FUNCTION__);
	
	for(line=0;line<9;line++)
	{
		//fread(buf,1,CPU_BUF_LEN,fd);
		if(fgets(buf,CPU_BUF_LEN,fd)==NULL)
		{
			printf("get eror maybe file end.%s\n",buf);
			//del_cpu_info(pInfo);
			break;
			//return NULL;
		}
		end=strchr(buf,'\n');
		if(end)
		{
			end[0]=0;
			if(cpu_info_parser(buf,&value)>0)
				attributes|=cpu_key_value_to_info(&value,pInfo);
		}
		else//if no \n,is too long
		{
			if(cpu_info_parser(buf,&value)>0)
				attributes|=cpu_key_value_to_info(&value,pInfo);
			while(fgets(buf,CPU_BUF_LEN,fd)!=NULL)
			{
				if(strchr(buf,'\n'))
					break;
			}
		}
		//printf("%s\n",buf);
		//printf("cpu attribute %03x\n",attributes);
	}

	del_key_value(&value);
	if(attributes==cpu_info_all)
		return pInfo;
	
	del_cpu_info(pInfo);
	return NULL;
}

struct linux_cpu_info* read_cpu_infos()
{
	int ret=-1;
	FILE* fd=fopen("/proc/cpuinfo","r");
	if(fd!=NULL)
	{
		struct linux_cpu_info* header=NULL,*pCur=NULL,*pNew;
		//memset(pInfo,0,sizeof(struct linux_cpu_info));

		while((pNew=read_cpu_info(fd))!=NULL)
		{
			if(header==NULL)
			{
				header=pNew;
				pCur=header;
			}
			else{
				pCur->next=pNew;
				pCur=pCur->next;
			}
		}
		fclose(fd);
		return header;
	}
	return NULL;
}

///////

int mem_info_parser(const char* buf,struct linux_mem* mem)
{
	const char* sep=strchr(buf,':');
	char name[24],value[64];
	int len;
	if(sep==NULL)return -1;
	len=sep-buf;
	strncpy(name,buf,len);
	name[len]=0;
	sep=sep+1;//
	while(sep[0]&&isblank(sep[0]))
		sep++;
	if(sep[0]==0)return -1;
	len=strlen(sep);
	if(len>63)
		len=63;
	strncpy(value,sep,len);
	value[len]=0;
	//printf("%s:%s\n",name,value);
	if(strcmp(name,"MemTotal")==0)
		mem->mem_total=atoi(value);
	else if(strcmp(name,"MemFree")==0)
		mem->mem_free=atoi(value);
	if(strcmp(name,"MemAvailable")==0)
			mem->mem_available=atoi(value);
	if(strcmp(name,"SwapTotal")==0)
			mem->swap_total=atoi(value);
	if(strcmp(name,"SwapFree")==0)
			mem->swap_free=atoi(value);
	return 0;
}
int linux_read_mem_info(struct linux_mem* mem)
{
	int ret=-1;
	FILE* fd=fopen("/proc/meminfo","r");
	if(fd!=NULL)
	{
		char buf[CPU_BUF_LEN];
		char* end;
		memset(mem,0,sizeof(struct linux_mem));
		while(fgets(buf,CPU_BUF_LEN,fd)!=NULL)
		{
			end=strchr(buf,'\n');
			if(end)
			{
				end[0]=0;
			}
			mem_info_parser(buf,mem);
		}
		fclose(fd);
		ret=0;
	}
	return ret;
}

int linux_storage_size(struct linux_storage_info* info)
{
	const char* path="/";
	struct statvfs st;
	#define MB	(1024*1024)
	if(statvfs(path,&st))
	{
		return -1;
	}
	//free
	info->free=st.f_bsize*st.f_bfree/MB;
	//all
	info->total=st.f_blocks*st.f_bsize/MB;
	//used
	info->used=(st.f_blocks-st.f_bavail)*st.f_bsize/MB;
	//printf("free=%lu,total=%lu,used=%lu\n",freeSize/MB,totalsize/MB,usedsize/MB);
	return 1;
}