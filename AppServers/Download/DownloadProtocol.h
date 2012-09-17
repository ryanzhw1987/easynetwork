/*
 * DownloadProtocol.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_SERVER_DOWNLOAD_PROTOCOL_H_
#define APP_SERVER_OWNLOAD_PROTOCOL_H_

#include "ProtocolDefault.h"

const ProtocolType PROTOCOL_REQUEST_SIZE = 1;   //请求
const ProtocolType PROTOCOL_REQUEST_DATA = 2;   //请求
const ProtocolType PROTOCOL_RESPOND_SIZE = 3;   //回复
const ProtocolType PROTOCOL_RESPOND_DATA = 4;   //回复

//请求文件大小
class RequestSize: public DefaultProtocol
{
	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	int encode_body(IOBuffer *io_buffer);
	//解码包体.成功返回0,否则返回-1;
	int decode_body(const char* buf, int buf_size);
public:
	RequestSize():DefaultProtocol(PROTOCOL_REQUEST_SIZE){}
	RequestSize(string &filename):DefaultProtocol(PROTOCOL_REQUEST_SIZE)
	{
		m_file_name = filename;
	}
	void assign(const string &file_name){m_file_name = file_name;}
	const string& get_file_name(){return m_file_name;}
private:
	string m_file_name;
};

//回复文件大小
class RespondSize: public DefaultProtocol
{
	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	int encode_body(IOBuffer *io_buffer);
	//解码包体.成功返回0,否则返回-1;
	int decode_body(const char* buf, int buf_size);
public:
	RespondSize():DefaultProtocol(PROTOCOL_RESPOND_SIZE){}
	RespondSize(string &filename, unsigned long long file_size):DefaultProtocol(PROTOCOL_RESPOND_SIZE)
	{
		m_file_size = file_size;
		m_file_name = filename;
	}

	void assign(const string &file_name, unsigned long long file_size)
	{
		m_file_name = file_name;
		m_file_size = file_size;
	}

	const string& get_file_name(){return m_file_name;}
	unsigned long long get_file_size(){return m_file_size;}
private:
	unsigned long long m_file_size;
	string m_file_name;
};

//请求数据协议
class RequestData: public DefaultProtocol
{
public:
	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	int encode_body(IOBuffer *io_buffer);
	//解码包体.成功返回0,否则返回-1;
	int decode_body(const char* buf, int buf_size);
public:
	RequestData():DefaultProtocol(PROTOCOL_REQUEST_DATA){}
	RequestData(string &filename, unsigned long long start_pos, unsigned int size):DefaultProtocol(PROTOCOL_REQUEST_DATA)
	{
		m_start_pos = start_pos;
		m_size = size;
		m_file_name = filename;
	}

	void assign(const string &file_name, unsigned long long start_pos, unsigned int size)
	{
		m_file_name = file_name;
		m_start_pos = start_pos;
		m_size = size;
	}

	unsigned long long get_start_pos(){return m_start_pos;}
	unsigned int get_size(){return m_size;}
	const string& get_file_name(){return m_file_name;}
private:
	unsigned long long m_start_pos;
	unsigned int m_size;
	string m_file_name;
};

//回复数据协议
class RespondData: public DefaultProtocol
{
public:
	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	int encode_body(IOBuffer *io_buffer);
	//解码包体.成功返回0,否则返回-1;
	int decode_body(const char* buf, int buf_size);
public:
	RespondData():DefaultProtocol(PROTOCOL_RESPOND_DATA){}
	RespondData(string &filename, unsigned long long start_pos, unsigned int size, string &data):DefaultProtocol(PROTOCOL_RESPOND_DATA)
	{
		m_start_pos = start_pos;
		m_size = size;
		m_file_name = filename;
		m_data = data;
	}

	void assign(const string &file_name, unsigned long long start_pos, unsigned int size)
	{
		m_file_name = file_name;
		m_start_pos = start_pos;
		m_size = size;
	}

	void assign(string &data)
	{
		m_data = data;
	}

	const string& get_file_name(){return m_file_name;}
	const string& get_data(){return m_data;}
	unsigned long long get_start_pos(){return m_start_pos;}
	unsigned int get_size(){return m_size;}

private:
	string m_file_name;
	string m_data;
	unsigned long long m_start_pos;
	unsigned int m_size;
};

class DownloadProtocolFamily:public DefaultProtocolFamily
{
public:
	Protocol* create_protocol(ProtocolType protocol_type, bool new_header=true);
	int destroy_protocol(Protocol* protocol);
private:
	//memory cache: StringProtocol
	MemCache<RequestSize> m_request_size_memcache;
	MemCache<RespondSize> m_respond_size_memcache;
	MemCache<RequestData> m_request_data_memcache;
	MemCache<RespondData> m_respond_data_memcache;
};

#endif //DOWNLOAD_PROTOCOL_H_



