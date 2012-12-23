/*
 * memorymgr.c
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 */

#include "memorymgr.h"
#include <pthread.h>

#define SLAB_COUNT 100
struct _memory_slab_
{
	pthread_mutex_t *lock;
	int element_size;  //元素大小
	int total_size;    //已经分配的总大小
	void *free_list;

};

static int memoryslab_init(MemorySlab *mem_slab, int element_size, int thread_safe)
{
	if(mem_slab == NULL)
		return -1;
	mem_slab->element_size = element_size;
	mem_slab->free_list = NULL;
	mem_slab->slab = malloc(element_size*SLAB_COUNT);

	mem_slab->lock = NULL;
	if(thread_safe != 0)
	{
		mem_slab->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		if(mem_slab->lock == NULL)
			return -1;
		pthread_mutex_init(mem_slab->lock, NULL);
	}
}

MemorySlab* memoryslab_create(int element_size, int thread_safe)
{
	MemorySlab *mem_slab = (MemorySlab*)malloc(sizeof(MemorySlab));
	if(mem_slab == NULL)
		return NULL;
	memoryslab_init(mem_slab, element_size, thread_safe);
	return mem_slab;
}




struct _memory_mgr_
{
	pthread_mutex_t *lock;
};

//thread_safe:0(非多线程安全),1(多线程安全)
MemoryMgr* memorymgr_init(int thread_safe)
{
	MemoryMgr *mem_mgr = (MemoryMgr*)malloc(sizeof(MemoryMgr));
	if(mem_mgr == NULL)
		return NULL;
	mem_mgr->lock = NULL;
	if(thread_safe != 0)
	{
		mem_mgr->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		if(mem_mgr->lock == NULL)
		{
			free(mem_mgr);
			return NULL;
		}
		pthread_mutex_init(mem_mgr->lock, NULL);
	}


	return mem_mgr;
}

void memorymgr_uninit(MemoryMgr *memory_mgr);
//分配size字节的内存块
void* memorymgr_malloc(MemoryMgr *memory_mgr, unsigned int size);
//释放内存块
void memorymgr_free(MemoryMgr *memory_mgr, void *ptr);
