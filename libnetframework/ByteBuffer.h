/*
 * ByteBuffer.h
 *
 *  Created on: 2012-10-22
 *      Author: LiuYongJin
 */

#ifndef _CLASS_BYTE_BUFFER_H_
#define _CLASS_BYTE_BUFFER_H_


#define INIT_SIZE 1024
#define MAX_SIZE  10*1024*1024   //缓冲区最大10M

class ByteBuffer
{
public:
	ByteBuffer(int capacity=INIT_SIZE);
	~ByteBuffer();
	int capacity(){return m_capacity;}
	int size(){return m_size;}
	//获取从偏移offset开始的大小为size的有效数据(默认获取所有的数据)
	char* get_data(int size=-1, int offset=0);
	//从有效数据缓冲区结尾开始获取一个大小为size的buffer(供调用者直接使用)
	//对获取到的buffer,最多只能写入size个字节的数据
	char* get_append_buffer(int size);
	//设置实际添加到结尾的数据长度
	void set_append_size(int append_size);
	//将长度为size的buf添加到有效数据缓冲区的末尾.成功返回true,失败返回false
	bool append_buffer(char *buf, int size);
	//在结尾添加count个字符c
	bool append_char(char c, int count);
	//容量增加size个字节(默认扩展为原来的2倍),成功返回true,失败返回false.
	bool expand(int size=-1);
	//清空全部有效数据
	void clean(){m_size = 0;}
	//清空末尾大小为size的有效数据(size超过数据长度的话, 等效于clean)
	void truncate(int size){size>m_size?m_size=0:m_size-=size;}
private:
	char *m_buffer;
	int m_capacity;
	int m_size;
};

#endif //_CLASS_BYTEBUFFER_H_
