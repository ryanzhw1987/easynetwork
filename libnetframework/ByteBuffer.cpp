/*
 * ByteBuffer.cpp
 *
 *  Created on: 2012-10-22
 *      Author: LiuYongJin
 */

#include "ByteBuffer.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

ByteBuffer::ByteBuffer(int capacity/*=INIT_SIZE*/)
{
	assert(capacity <= MAX_SIZE);
	m_capacity = capacity;
	m_buffer = (char *)calloc(1, m_capacity);
	assert(m_buffer != NULL);

	m_size = 0;
}

ByteBuffer::~ByteBuffer()
{
	if(m_buffer != NULL)
		free(m_buffer);
	m_buffer = NULL;
	m_capacity = 0;
	m_size = 0;
}

//获取从偏移offset开始的大小为size的有效数据(size=-1时表示offset后所有的有效数据)
char* ByteBuffer::get_data(int size/*=-1*/, int offset/*=0*/)
{
	if(m_size <= 0)
		return NULL;
	if(offset < 0)
		offset = 0;

	if((size<0 && offset<m_size) || offset+size <= m_size)
		return m_buffer+offset;
	return NULL;
}

//从有效数据缓冲区结尾开始获取一个大小为size的buffer(供调用者直接使用)
//对获取到的buffer,最多只能写入size个字节的数据
char* ByteBuffer::get_append_buffer(int size)
{
	if(m_size+size > m_capacity)
	{
		//不够空间
		int need_size = m_size+size;
		int new_size = m_capacity;
		while(new_size<need_size && new_size<MAX_SIZE)  //按原空间2倍扩展
			new_size <<= 1;
		if(new_size >= MAX_SIZE)
			return NULL;

		char *temp = (char *)realloc(m_buffer, new_size);
		if(temp == NULL)
			return NULL;

		m_buffer = temp;
		m_capacity = new_size;
	}

	return m_buffer+m_size;
}

//设置实际添加到结尾的数据长度
void ByteBuffer::set_append_size(int append_size)
{
	assert(m_size+append_size <= m_capacity);
	m_size += append_size;
}

//将长度为size的buf添加到有效数据缓冲区的末尾.成功返回true,失败返回false
bool ByteBuffer::append(const char *buf, int size)
{
	char *append_buf = NULL;
	if(buf == NULL)
		return false;
	append_buf = get_append_buffer(size);
	if(append_buf == NULL)
		return false;
	memcpy(append_buf, buf, size);
	set_append_size(size);
	return true;
}

//添加以'\0\为结尾的字符串string到buffer的末尾(不包括string的'\0')
bool ByteBuffer::append(const char *str)
{
	int size = strlen(str);
	if(size > 0)
		return append(str , size);
	return true;
}

//在结尾添加count个字符c
bool ByteBuffer::append(const char c, int count/*=1*/)
{
	char *append_buf = get_append_buffer(count);
	if(append_buf == NULL)
		return false;
	int i;
	for(i=0; i<count; ++i)
		append_buf[i] = c;
	set_append_size(count);
	return true;
}

//容量增加size个字节(默认扩展为原来的2倍),成功返回true,失败返回false.
bool ByteBuffer::expand(int size/*-=-1*/)
{
	int new_size = 0;
	if(size < 0)
		new_size = m_capacity<<1;
	else
		new_size = m_capacity+size;
	if(new_size > MAX_SIZE)
		return false;

	char *temp = (char *)realloc(m_buffer, new_size);
	if(temp == NULL)
		return false;

	m_buffer = temp;
	m_capacity = new_size;
	return true;
}

ByteBuffer& ByteBuffer::operator +=(const char *str)
{
	append(str);
	return *this;
}

ByteBuffer& ByteBuffer::operator +=(const char c)
{
	append(c);
	return *this;
}

ByteBuffer& ByteBuffer::operator <<(const char *str)
{
	append(str);
	return *this;
}

ByteBuffer& ByteBuffer::operator <<(const char c)
{
	append(c);
	return *this;
}
