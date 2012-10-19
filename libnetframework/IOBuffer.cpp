/*
 * IOBuffer.cpp
 *
 *  Created on: 2012-9-8
 *      Author: LiuYongjin
 */

#include "IOBuffer.h"
#include <string.h>
#include <stdlib.h>

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

////////////////////////////////////////////////////////////
//打开大小为size的缓冲区用于写.
//成功:返回缓存取到指针; 失败:返回NULL;
char *IOBuffer::write_open(unsigned int size)
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

//关闭写缓冲区;
//成功:返回true, 设置成功写入的字节数;
//失败:返回false,缓冲区数据没有任何改变;
bool IOBuffer::write_close(unsigned int write_size)
{
	if(m_buffer_size-m_data_size < write_size)  //本次写入的长度比剩余空间还多, 错误
		return false;
	m_data_size += write_size;
	return true;
}

//打开缓冲区用于读
//成功:返回缓冲区到指针,设置size为可读缓冲区中数据的大小
//失败:返回NULL
const char *IOBuffer::read_open(unsigned int &size)
{
	size = m_data_size;
	if(m_data_size>0)
		return m_data;
	else
		return NULL;
}

//关闭读缓冲区;
//成功:返回true,设置成功读出的字节数
//失败:返回false,缓冲区数据没有任何改变;
bool IOBuffer::read_close(unsigned int read_size)
{
	if(read_size > m_data_size) //读的长度比数据长度还大
		return false;
	m_data += read_size;
	m_data_size -= read_size;
	return true;
}

