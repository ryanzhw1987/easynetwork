/*
 * test_all.cpp
 *
 *  Created on: 2012-12-23
 *      Author: LiuYongjin
 */

#include <stdio.h>
#include <assert.h>

/////heapsort test /////
#include "heapsort.h"
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


////////////////////////////  main  ////////////////////////////
int main()
{
	test_heapsort();
	return 0;
}
