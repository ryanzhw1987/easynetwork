/*
 * IOBuffer.cpp
 *
 *  Created on: 2012-9-8
 *      Author: LiuYongjin
 */

#include "IOBuffer.h"
#include <string.h>
#include <stdlib.h>
#include "slog.h"

#define MAX_BUFFER_SIZE 10*1024*1024   //缓冲区最大10M

IOBuffer::IOBuffer(unsigned int size/*=BUFFER_SIZE*/)
{
	m_buffer = NULL;
	m_buffer_size = 0;

	m_buffer = (char *)calloc(1, size);
	if(m_buffer != NULL)
		m_buffer_size = size;

	m_data = m_buffer;
	m_data_size = 0;
}

IOBuffer::~IOBuffer()
{
	if(m_buffer != NULL)
		free(m_buffer);
	m_buffer = m_data = NULL;
	m_buffer_size = m_data_size = 0;
}

//获取一个大小为size的缓冲用于写, 成功返回可写的缓冲区, 失败返回NULL,表示没有足够内存
char* IOBuffer::get_write_buffer(unsigned int size)
{
	unsigned int total_free  = m_buffer_size-m_data_size; //缓冲区总的剩余空间
	unsigned int offset = m_data-m_buffer;           //缓冲区前端剩余空间
	unsigned int last_free   = total_free-offset;              //缓冲区后端剩余空间
	if(last_free >= size) //缓冲区后端的空间足够
		return m_data+m_data_size;

	//移动数据到缓冲去开始处
	memcpy(m_buffer, m_data, m_data_size);
	m_data = m_buffer;

	//总的剩余空间足够
	if(total_free >= size)
		return m_data+m_data_size;

	//不够空间
	int new_size = m_buffer_size;
	if(new_size < BUFFER_SIZE)
		new_size = BUFFER_SIZE;
	while(new_size-m_data_size<size && new_size<MAX_BUFFER_SIZE)  //按原空间2倍扩展
		new_size <<= 1;
	if(new_size >= MAX_BUFFER_SIZE)
		return NULL;

	char *temp = (char *)realloc(m_buffer, new_size);
	if(temp != NULL)
	{
		m_buffer = temp;
		m_buffer_size = new_size;
		m_data = m_buffer;
		return m_data+m_data_size;
	}
	return NULL;
}

//设置本次写入多少数据, 成功返回true, 失败返回false
bool IOBuffer::set_write_size(unsigned int size)
{
	if(m_buffer_size-m_data_size < size)  //本次写入的长度比剩余空间还多, 错误
		return false;
	m_data_size += size;
	return true;
}

bool IOBuffer::write_rollback(unsigned int size)
{
	if(size > m_data_size)
		return false;
	m_data_size -= size;
	return true;
}
//获取用于读的缓冲区, size返回可读取缓冲区的大小.如果没有数据可读, 返回NULL.
char* IOBuffer::get_read_buffer(unsigned int *data_size)
{
	if(m_data_size>0)
	{
		*data_size = m_data_size;
		return m_data;
	}
	*data_size = 0;
	return NULL;
}

//设置本次读取多少数据, 成功返回true, 失败返回false
bool IOBuffer::set_read_size(unsigned int size)
{
	if(size > m_data_size) //读的长度比数据长度还大
		return false;
	m_data += size;
	m_data_size -= size;
	return true;
}

