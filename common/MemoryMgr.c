/*
 * MemoryMgr.c
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 */

#include "MemoryMgr.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LOCK(lock) do{if(lock!=NULL) pthread_mutex_lock(lock);}while(0)
#define UNLOCK(lock) do{if(lock!=NULL) pthread_mutex_unlock(lock);}while(0)
#define ELEMENT_COUNT 100    //每个slab包含的元素个数
#define CLASS_NUMBER  6      //slab分类总数,每类的slab元素大小依次为4,8,16,32,64,128...
static uint32_t _class_id(int size)
{
	uint32_t class_id = 0;
	if(size<=0)
		size = 1;
	for(size=(size-1)>>2; size>0; size=size>>1)
		++class_id;
	return class_id;
}

////////////////////////////  memory slab  ////////////////////////////
struct _memory_slab_
{
	uint32_t class_id;
	uint32_t element_size;  //元素大小
	void *free_list;        //空闲链表
	void *slab;             //slab列表
	void *cur;              //当前slab中可用内存起始地址
	uint64_t total_size;    //slab分配到的有效内存大小
	pthread_mutex_t *lock;
};

#define ELEMENT_SIZE(size) (size+sizeof(uint32_t))   //class_id占用额外的4个字节
#define SLAB_SIZE(element_size) (element_size*ELEMENT_COUNT)
#define SLAB_BASE(slab) (slab+sizeof(void*))
#define SLAB_END(slab, element_size) (slab+sizeof(void*)+SLAB_SIZE(element_size))
#define SLAB_NEXT(slab) (*(void**)slab)
#define SLAB_FREE(slab) (free(slab))
static void* SLAB_MALLOC(int element_size)
{
	int size = sizeof(void*)+SLAB_SIZE(element_size);
	void *slab = malloc(size);
	if(slab != NULL)
		*(void**)slab = NULL;
	return slab;
}

static int _memoryslab_init(MemorySlab *mem_slab, int element_size, int thread_safe)
{
	mem_slab->class_id = _class_id(element_size);
	mem_slab->element_size = ELEMENT_SIZE(element_size);
	mem_slab->free_list = NULL;
	//create slab
	void *slab = SLAB_MALLOC(mem_slab->element_size);
	if(slab == NULL)
		return -1;
	mem_slab->slab = slab;
	mem_slab->cur = SLAB_BASE(slab);
	mem_slab->total_size = SLAB_SIZE(mem_slab->element_size);
	//create lock
	mem_slab->lock = NULL;
	if(thread_safe != 0)
	{
		mem_slab->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		if(mem_slab->lock == NULL)
		{
			SLAB_FREE(mem_slab->slab);
			return -1;
		}
		pthread_mutex_init(mem_slab->lock, NULL);
	}
	return 0;
}

static void _memoryslab_uninit(MemorySlab *mem_slab)
{
	while(mem_slab->slab != NULL)
	{
		void *slab = mem_slab->slab;
		mem_slab->slab = SLAB_NEXT(slab);
		free(slab);
	}
	if(mem_slab->lock)
	{
		pthread_mutex_destroy(mem_slab->lock);
		free(mem_slab->lock);
	}
}

MemorySlab* memoryslab_create(int element_size, int thread_safe)
{
	MemorySlab *mem_slab = (MemorySlab*)malloc(sizeof(MemorySlab));
	if(mem_slab == NULL)
		return NULL;
	if(_memoryslab_init(mem_slab, element_size, thread_safe) != 0)
	{
		free(mem_slab);
		return NULL;
	}
	return mem_slab;
}

void memoryslab_destroy(MemorySlab *mem_slab)
{
	if(mem_slab == NULL)
		return;
	_memoryslab_uninit(mem_slab);
	free(mem_slab);
}

void* memoryslab_malloc(MemorySlab *mem_slab)
{
	void *ptr = NULL;
	if(mem_slab == NULL)
		return NULL;
	LOCK(mem_slab->lock);
	if(mem_slab->free_list != NULL)  //get memory from free_list
	{
		ptr = mem_slab->free_list;
		mem_slab->free_list = *(void**)ptr;  //next
	}
	else//get memory from slab
	{
		if(mem_slab->cur+mem_slab->element_size > SLAB_END(mem_slab->slab, mem_slab->element_size))  //no enough
		{
			void *slab = SLAB_MALLOC(mem_slab->element_size);
			if(slab != NULL)
			{
				*(void**)slab = mem_slab->slab;
				mem_slab->slab = slab;
				mem_slab->cur = SLAB_BASE(slab);
				mem_slab->total_size += SLAB_SIZE(mem_slab->element_size);
			}
		}

		if(mem_slab->cur+mem_slab->element_size <= SLAB_END(mem_slab->slab, mem_slab->element_size))
		{
			*(uint32_t*)mem_slab->cur = mem_slab->class_id;
			ptr = mem_slab->cur+sizeof(uint32_t);
			mem_slab->cur += mem_slab->element_size;
		}
	}
	UNLOCK(mem_slab->lock);
	return ptr;
}

int memoryslab_free(MemorySlab *mem_slab, void *ptr)
{
	if(mem_slab == NULL)
		return -1;
	if(ptr == NULL)
		return 0;
	//check class_id
	uint32_t class_id = *(uint32_t*)(ptr-sizeof(uint32_t));
	if(class_id != mem_slab->class_id)
		return -1;
	//add to free list
	LOCK(mem_slab->lock);
	*(void**)ptr = mem_slab->free_list;
	mem_slab->free_list = ptr;
	UNLOCK(mem_slab->lock);
	return 0;
}

////////////////////////////// memory manager ////////////////////////////
struct _memory_mgr_
{
	MemorySlab slabs[CLASS_NUMBER];
};

MemoryMgr* memorymgr_init(int thread_safe)
{
	MemoryMgr *mem_mgr = (MemoryMgr*)malloc(sizeof(MemoryMgr));
	if(mem_mgr == NULL)
		return NULL;
	int element_size, class_id;
	for(class_id=0,element_size=4; class_id<CLASS_NUMBER; ++class_id,element_size=element_size<<1)
		_memoryslab_init(&mem_mgr->slabs[class_id], element_size, thread_safe);

	return mem_mgr;
}

void memorymgr_uninit(MemoryMgr *memory_mgr)
{
	if(memory_mgr == NULL)
		return ;
	int class_id;
	for(class_id=0; class_id<CLASS_NUMBER; ++class_id)
		_memoryslab_uninit(&memory_mgr->slabs[class_id]);
	free(memory_mgr);
}

//分配size字节的内存块
void* memorymgr_malloc(MemoryMgr *memory_mgr, int size)
{
	uint32_t class_id = _class_id(size);
	if(memory_mgr==NULL || class_id>=CLASS_NUMBER)
		return malloc(size);
	else
		return memoryslab_malloc(&memory_mgr->slabs[class_id]);
}

//释放内存块
int memorymgr_free(MemoryMgr *memory_mgr, void *ptr)
{
	if(memory_mgr == NULL)
		return -1;
	if(ptr == NULL)
		return 0;
	uint32_t class_id = *(uint32_t*)(ptr-sizeof(uint32_t));
	if(class_id<0 || class_id>=CLASS_NUMBER)
	{
		free(ptr);
		return 0;
	}
	else
		return memoryslab_free(&memory_mgr->slabs[class_id], ptr);
}
