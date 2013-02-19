/*
 * Array.h
 *
 *  Created on: 2013-2-19
 *      Author: LiuYongjin
 */


#ifndef _COMMON_ARRAY_H_
#define _COMMON_ARRAY_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _array_ Array;

Array* array_create(uint32_t elem_size);
uint32_t array_element_size(Array *array);
uint32_t array_size(Array *array);            //number of elements in array
uint32_t array_capacity(Array *array);        //capacity of array
uint32_t array_empty(Array *array);
void* array_element(Array *array, uint32_t index);
void* array_add(Array *array);                //get a block of memory for saving element
void array_remove(Array *array, void *elem);  //remove the element from array
void array_clean(Array *array);

#ifdef __cplusplus
}
#endif

#endif//_COMMON_ARRAY_H_
