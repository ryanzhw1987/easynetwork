/*
 * TransProtocol.cpp
 *
 *  Created on: 2012-11-11
 *      Author: LiuYongJin
 */

#include "TransProtocol.h"
#include "ByteBuffer.h"
#include "slog.h"
#include <assert.h>

bool TransProtocol::send_protocol(TransSocket *trans_socket, Protocol *protocol)
{
	if(trans_socket==NULL || protocol==NULL)
	{
		SLOG_ERROR("parameter error.");
		return false;
	}
	ByteBuffer byte_buffer;
	//1. 预留头部
	ProtocolHeader *header = protocol->get_protocol_header();
	assert(header != NULL);
	int header_length = header->get_header_length();
	byte_buffer.reserve(header_length);
	//2. 编码协议体
	if(!protocol->encode_body(&byte_buffer))
	{
		SLOG_ERROR("encode body error. protocol detail=%s.", protocol->details());
		return false;
	}
	//3. 编码头部
	int body_length = byte_buffer.size()-header_length;
	char *header_buffer = byte_buffer.get_data(0, header_length);
	assert(header_buffer != NULL);
	if(!header->encode(header_buffer, body_length))
	{
		SLOG_ERROR("encode header error. protocol detail=%s.", protocol->details());
		return false;
	}
	//4. 发送数据
	if(trans_socket->send_data_all(byte_buffer.get_data(), byte_buffer.size()) == TRANS_ERROR)
	{
		SLOG_ERROR("send protocol data error. protocol detail=%s.", protocol->details());
		return false;
	}
	return true;
}

bool TransProtocol::recv_protocol(TransSocket *trans_socket, Protocol *protocol)
{
	if(trans_socket==NULL || protocol==NULL)
	{
		SLOG_ERROR("parameter error.");
		return false;
	}
	ByteBuffer *byte_buffer = new ByteBuffer;
	char *buffer = NULL;
	//1. 读头部数据
	ProtocolHeader *header = protocol->get_protocol_header();
	assert(header != NULL);
	int header_length = header->get_header_length();
	buffer = byte_buffer->get_append_buffer(header_length);
	if(trans_socket->recv_data_all(buffer, header_length) == TRANS_ERROR)
	{
		SLOG_ERROR("receive header error.");
		delete byte_buffer;
		return false;
	}
	byte_buffer->set_append_size(header_length);
	//2. 解码头部
	int body_length = 0;
	if(!header->decode(buffer, body_length))
	{
		SLOG_ERROR("decode header error.");
		delete byte_buffer;
		return false;
	}
	//3. 读协议体数据
	buffer = byte_buffer->get_append_buffer(body_length);
	if(trans_socket->recv_data_all(buffer, body_length) == TRANS_ERROR)
	{
		SLOG_ERROR("receive body error.");
		delete byte_buffer;
		return false;
	}
	byte_buffer->set_append_size(body_length);
	//4. 解码协议体
	if(!protocol->decode_body(buffer, body_length))
	{
		SLOG_ERROR("decpde body error.");
		delete byte_buffer;
		return false;
	}

	return protocol->attach_raw_data(byte_buffer);
}



