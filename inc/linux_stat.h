/*
 * linuxsys.h
 *
 *  Created on: Jul 1, 2024
 *      Author: wang
 */

#ifndef INC_LINUX_STAT_H_
#define INC_LINUX_STAT_H_

struct linux_mem
{
	unsigned int mem_total;
	unsigned int mem_free;
	unsigned int mem_available;
	unsigned int swap_total;
	unsigned int swap_free;
};


struct linux_cpu_info
{
	int processor;
	char vendor_id[32];
	char cpu_family[8];
	char model[8];
	char model_name[64];
	char cpu_MHz[16];
	char cache_size[16];
	int physical_id;
	int siblings;
	int core_id;
	int cpu_cores;
	struct linux_cpu_info* next;
};

struct linux_cpu_stat
{
	char name[16];
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
	unsigned int lowait;
	unsigned int irq;
	unsigned int softirq;
	int cpu_cnt;
	struct linux_cpu_stat* cpus;
};
//storage size,unit:MB
struct linux_storage_info
{
	unsigned int total;
	unsigned int used;
	unsigned int free;
};
struct linux_cpu_info* create_linux_cpu_info();
void del_cpu_info(struct linux_cpu_info* p);

unsigned int linux_get_cpu_busy(const struct linux_cpu_stat*);
int linux_read_cpu_stat(struct linux_cpu_stat* cpu_stat);
void print_cpu(const struct linux_cpu_stat *cpu);
struct linux_cpu_info* read_cpu_infos();
int linux_read_mem_info(struct linux_mem* mem);

int linux_storage_size(struct linux_storage_info*);
#endif /* INC_LINUX_STAT_H_ */
