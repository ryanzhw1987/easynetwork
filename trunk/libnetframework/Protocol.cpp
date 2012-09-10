/*
 * Protocol.cpp
 *
 *  Created on: 2012-9-8
 *      Author: LiuYongjin
 */
#include "Protocol.h"
#include <assert.h>

int Protocol::encode(IOBuffer *io_buffer)
{
	if(io_buffer==NULL || m_header==NULL)
		return -1;

	//1. 分配一个header_size大小的空间,暂时不写入数据
	int header_size = get_header_size();
	if(header_size <= 0)
		return -1;

	//头部的偏移位置
	unsigned int header_offset = io_buffer->get_data_size();

	//写入头部数据(没有真正写入数据,只是为头部预留空间)
	if(io_buffer->get_write_buffer(header_size) == NULL) //没有内存
		return -1;
	if(io_buffer->set_write_size(header_size) == false)
		return -1;

	//2. 编码协议体
	bool result;
	int body_size = encode_body(io_buffer);
	if(body_size == -1)  //失败, 回滚分配的header空间
	{
		result = io_buffer->write_rollback(header_size);
		assert(result == true);
		return -1;
	}
	//encode_body的时候可能导致数据迁移,需要重新定位header_buffer的位置.
	char *header_buffer = io_buffer->seek(header_offset);
	assert(header_buffer != NULL);

	//3. 编码协议头(真正写入头部数据)
	m_header->set_body_size(body_size);
	if(m_header->encode(header_buffer, header_size) != 0)	//头部编码失败,回滚协议体和协议体占用的空间
	{
		result = io_buffer->write_rollback(header_size+body_size);
		assert(result == true);
		return -1;
	}
	return 0;
}
