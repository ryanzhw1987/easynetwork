/*
 * memorymgr.h
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 * Description: 内存管理模块
 */

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

typedef struct _memory_slab_ MemorySlab;
typedef struct _memory_mgr_ MemoryMgr;

#ifdef __cplusplus
extern "C"
{
#endif

//slab:分配固定大小element_size;thread_safe:0(非多线程安全),1(多线程安全)
MemorySlab* memoryslab_create(int element_size, int thread_safe);
void memoryslab_destroy(MemorySlab *mem_slab);
void* memoryslab_malloc(MemorySlab *mem_slab);
int memoryslab_free(MemorySlab *mem_slab, void *ptr);

//内存管理模块:thread_safe:0(非多线程安全),1(多线程安全)
MemoryMgr* memorymgr_init(int thread_safe);
void memorymgr_uninit(MemoryMgr *memory_mgr);
void* memorymgr_malloc(MemoryMgr *memory_mgr, int size);
int memorymgr_free(MemoryMgr *memory_mgr, void *ptr);

#ifdef __cplusplus
}
#endif

#endif //_MEMORY_MANAGER_H_

