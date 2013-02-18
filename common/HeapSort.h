/*
 * HeapSort.h
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 * Description: 堆排序模块
 */

#ifndef _HEAP_SORT_H_
#define _HEAP_SORT_H_

typedef struct _heap_ Heap;

//比较函数指针.返回值:-1(a小于b); 0(a等于b); 1(a大于b)
//如果a不大于b,则a排列在b的前面
typedef int (*ElemCompare)(void *element_a, void *element_b);
//元素销毁函数指针
typedef void (*ElemDestroy)(void *element);


#ifdef __cplusplus
extern "C"
{
#endif

Heap* heap_create(ElemCompare cmp_func, ElemDestroy des_func);   //创建堆
void heap_destroy(Heap *heap);                                   //销毁堆
int heap_count(Heap *heap);                                      //堆元素个数
int heap_insert(Heap *heap, void *element);                      //插入元素,成功返回0,失败返回-1
void* heap_top(Heap *heap);                                      //获取堆顶元素
void heap_pop(Heap *heap);                                       //删除堆顶元素
void heap_clear(Heap *heap);                                     //清除堆

#ifdef __cplusplus
}
#endif

#endif //_HEAP_SORT_H_


