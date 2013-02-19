/*
 * Array.c
 *
 *  Created on: 2013-2-19
 *      Author: LiuYongjin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Array.h"

#define ARRAY_INIT_CAPACITY       64
#define ARRAY_CLEAN_THRESHOLD     512

struct _array_
{
	char *base;
	uint32_t elem_size;       //element size
	uint32_t elem_count;
	uint32_t capacity;
};

Array* array_create(uint32_t elem_size)
{
	Array* temp = (Array*)malloc(sizeof(Array));
	if(temp == NULL)
		return NULL;

	temp->elem_size = elem_size;
	temp->elem_count = 0;
	temp->capacity = ARRAY_INIT_CAPACITY;

	temp->base = (char*)malloc(temp->capacity*temp->elem_size);
	if(temp->base == NULL)
	{
		free(temp);
		temp = NULL;
	}

	return temp;
}

uint32_t array_element_size(Array *array)
{
	return array->elem_size;
}

uint32_t array_size(Array *array)
{
	return array->elem_count;
}

uint32_t array_capacity(Array *array)
{
	return array->capacity;
}

uint32_t array_empty(Array *array)
{
	return array->elem_count>0?0:1;
}

void* array_add(Array *array)
{
	if(array->elem_count == array->capacity)
	{
		uint32_t cap = array->capacity << 1;
		void *base = realloc((void*)array->base, cap*array->elem_size);
		if(base == NULL)
			return NULL;
		array->base = (char*)base;
		array->capacity = cap;
	}

	char *temp = array->base + array->elem_count*array->elem_size;
	++array->elem_count;
	return (void*)temp;
}

void array_remove(Array *array, void *element)
{
	if(array->elem_count == 0)
		return ;
	--array->elem_count;
	uint32_t index = ((char*)element-array->base)/array->elem_size;
	if(index < array->elem_count)
		memmove(element, array->base+array->elem_count*array->elem_size, array->elem_size);
}

void array_clean(Array *array)
{
	if(array->capacity >= ARRAY_CLEAN_THRESHOLD)
	{
		array->capacity = ARRAY_INIT_CAPACITY;
		array->base = (char*)realloc((void*)array->base, array->capacity*array->elem_size);
	}

	array->elem_count = 0;
}

void* array_element(Array *array, uint32_t index)
{
	if(index >= array->elem_count)
		return NULL;
	return (void*)(array->base + index*array->elem_size);
}
