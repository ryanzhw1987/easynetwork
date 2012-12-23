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

MemorySlab* memoryslab_create(int element_size, int thread_safe);



//thread_safe:0(非多线程安全),1(多线程安全)
MemoryMgr* memorymgr_init(int thread_safe);
void memorymgr_uninit(MemoryMgr *memory_mgr);
//分配size字节的内存块
void* memorymgr_malloc(MemoryMgr *memory_mgr, unsigned int size);
//释放内存块
void memorymgr_free(MemoryMgr *memory_mgr, void *ptr);

#ifdef __cplusplus
}
#endif

#endif //_MEMORY_MANAGER_H_

