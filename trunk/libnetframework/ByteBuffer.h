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

//注意: 通过ByteBuffer的方法(比如get_append_buffer)获得的buffer指针,最好不要长久保存使用.
//因为ByteBuffer内部可能重新分配,导致保存的指针失效!!!!
//比如下面使用是危险的,因为调用append方法可能导致内存重新分配.
//ByteBuffer byte_buffer;
//char *temp_buffer = byte_buffer.get_append_buffer(1024);
//byte_buffer.append("test",4);
//memcpy(temp_buffer, "this operation is dangerous!!!", 30);
class ByteBuffer
{
public:
	//初始化一个空间大小为capacity的buffer
	ByteBuffer(int capacity=INIT_SIZE);
	~ByteBuffer();
	//返回buffer的空间大小
	int capacity(){return m_capacity;}
	//返回buffer中数据的大小
	int size(){return m_size;}
	//获取从偏移offset开始的大小为size的有效数据(size=-1时表示offset后所有的有效数据)
	char* get_data(int size=-1, int offset=0);
	//从有效数据缓冲区结尾开始获取一个大小为size的buffer(供调用者直接使用)
	//对获取到的buffer,最多只能写入size个字节的数据;
	//如果往获取到的buffer中写入数据,而没有调用set_append_size,则写入无效,不改变数据的大小(可能改变buffer的capacity)
	char* get_append_buffer(int size);
	//设置实际添加到结尾的数据长度
	//必须先调用get_append_buffer
	void set_append_size(int append_size);
	//将长度为size的buf添加到有效数据缓冲区的末尾.成功返回true,失败返回false
	bool append_buffer(char *buf, int size);
	//添加以'\0\为结尾的字符串string到buffer的末尾(不包括string的'\0')
	bool append_string(char *string);
	//在结尾添加count个字符c
	bool append_char(char c, int count=1);
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
