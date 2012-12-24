/*
 * test_all.cpp
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongjin
 */

#include <stdio.h>
#include <assert.h>

/////heapsort test /////
static void test_heapsort();
static void test_memoryslab();
static void test_memorymgr();

////////////////////////////  main  ////////////////////////////
int main()
{
	test_heapsort();
	test_memoryslab();
	test_memorymgr();
	return 0;
}

#include "HeapSort.h"
void test_heapsort()
{
	Heap *heap = heapsort_create(NULL, NULL);
	assert(heap != NULL);
	heapsort_insert(heap, (void*)5);
	heapsort_insert(heap, (void*)4);
	heapsort_insert(heap, (void*)3);
	heapsort_insert(heap, (void*)2);
	heapsort_insert(heap, (void*)1);
	heapsort_insert(heap, (void*)7);
	heapsort_insert(heap, (void*)8);
	heapsort_insert(heap, (void*)9);
	heapsort_insert(heap, (void*)10);
	printf("heap size:%d\n", heapsort_count(heap));
	while(heapsort_count(heap) > 0)
	{
		printf("%d ", (int)heapsort_top(heap));
		heapsort_pop(heap);
	}
	printf("\n");

	heapsort_destroy(heap);
}

#include "MemoryMgr.h"
void test_memoryslab()
{
	MemorySlab *mem_slab = memoryslab_create(4, 1);
	int i=0;
	while(i++<500)
	{
		void *ptr1 = memoryslab_malloc(mem_slab);
		void *ptr2 = memoryslab_malloc(mem_slab);
		printf("%0X %0X;", (unsigned int)ptr1, (unsigned int)ptr2);
		memoryslab_free(mem_slab, ptr2);
	}
	printf("\n");

	memoryslab_destroy(mem_slab);
}

void test_memorymgr()
{
	MemoryMgr *mem_mgr = memorymgr_init(1);
	int i=0;
	while(i++<500)
	{
		void *ptr1 = memorymgr_malloc(mem_mgr, 4);
		void *ptr2 = memorymgr_malloc(mem_mgr, 6);
		printf("%0X %0X;", (unsigned int)ptr1, (unsigned int)ptr2);
	}
	printf("\n");
	memorymgr_uninit(mem_mgr);
}
