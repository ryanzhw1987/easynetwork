/*
 * HeapSort.cpp
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongJin
 */

#include "HeapSort.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INIT_CAPACITY 128

struct _heap_
{
	int capacity;    //堆当前容量
	int size;        //堆元素个数
	void **elements; //堆元素数组
	ElementCompareFunc cmp_func;
	ElementDestroyFunc des_func;
};

static int _default_cmp(void *a, void *b)
{
	if(a < b)
		return -1;
	else if(a==b)
		return 0;
	else
		return 1;
}

static int _expand_capacity(Heap *heap)
{
	if(heap == NULL)
		return -1;
	int new_capacity = heap->capacity * 2;
	void ** new_ptr =(void **)realloc((void*)heap->elements, sizeof(void*)*new_capacity);
	if(new_ptr == NULL)
		return -1;
	heap->elements = new_ptr;
	heap->capacity = new_capacity;
	return 0;
}


//创建堆
Heap* heapsort_create(ElementCompareFunc cmp_func, ElementDestroyFunc des_func)
{
	Heap *heap = (Heap*)malloc(sizeof(Heap));
	if(heap == NULL)
		return NULL;
	heap->elements = (void **)malloc(sizeof(void*)*INIT_CAPACITY);
	if(heap->elements == NULL)
	{
		free(heap);
		return NULL;
	}
	heap->capacity = INIT_CAPACITY;
	heap->size = 0;
	heap->cmp_func = (cmp_func!=NULL?cmp_func:_default_cmp);
	heap->des_func = des_func;

	return heap;
}

//销毁堆
void heapsort_destroy(Heap *heap)
{
	if(heap == NULL)
		return;
	if(heap->elements != NULL)
	{
		if(heap->des_func != NULL)
			while(--heap->size >= 0)
				heap->des_func(heap->elements[heap->size]);
		free(heap->elements);
	}
	free(heap);
}

//堆元素个数
int heapsort_count(Heap *heap)
{
	return heap==NULL?0:heap->size;
}

//插入元素,成功返回0,失败返回-1
int heapsort_insert(Heap *heap, void *element)
{
	if(heap == NULL)
		return -1;
	if(heap->size == heap->capacity)
		if(_expand_capacity(heap) != 0)
			return -1;

	//自底向上调整
	int curpos = heap->size;
	int p = (curpos-1)/2;
	while(curpos>0 && p >= 0)
	{
		if(heap->cmp_func(heap->elements[p], element) != 1)
			break;
		heap->elements[curpos] = heap->elements[p];
		curpos = p;
		p = (curpos-1)/2;
	}
	heap->elements[curpos] = element;
	++heap->size;

	return 0;
}

//获取堆顶元素
void* heapsort_top(Heap *heap)
{
	if(heap==NULL || heap->size<=0)
		return NULL;
	else
		return heap->elements[0];
}

//删除堆顶元素
void heapsort_pop(Heap *heap)
{
	if(heap==NULL || heap->size<=0)
		return;
	if(heap->des_func != NULL)
		heap->des_func(heap->elements[0]);
	--heap->size;
	if(heap->size == 0)
		return;

	void *element = heap->elements[heap->size];
	int curpos = 0;
	int child = curpos*2+1;
	while(child < heap->size)
	{
		if(child+1<heap->size && heap->cmp_func(heap->elements[child+1], heap->elements[child])==-1)
			++child;
		if(heap->cmp_func(element, heap->elements[child])==-1)
			break;
		heap->elements[curpos] = heap->elements[child];
		curpos = child;
		child = curpos*2+1;
	}
	heap->elements[curpos] = element;
}

//清除堆
void heapsort_clear(Heap *heap)
{
	if(heap == NULL)
		return;
	if(heap->des_func != NULL)
		while(--heap->size >= 0)
			heap->des_func(heap->elements[heap->size]);
	heap->size = 0;
}
