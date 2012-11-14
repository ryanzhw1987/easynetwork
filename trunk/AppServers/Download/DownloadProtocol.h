/*
 * DownloadProtocol.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_SERVER_DOWNLOAD_PROTOCOL_H_
#define APP_SERVER_DOWNLOAD_PROTOCOL_H_

#include "DefaultProtocolFamily.h"
#include "MemManager.h"
#include <stdint.h>

#include <string>
using std::string;

#define PROTOCOL_REQUEST_SIZE 1    //请求
#define PROTOCOL_REQUEST_DATA 2   //请求
#define PROTOCOL_RESPOND_SIZE 3   //回复
#define PROTOCOL_RESPOND_DATA 4   //回复

//请求文件大小
class RequestSize: public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "RequestSize";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	RequestSize(){}
	RequestSize(string &filename)
	{
		m_file_name = filename;
	}
	void assign(const string &file_name){m_file_name = file_name;}
	const string& get_file_name(){return m_file_name;}
private:
	string m_file_name;
};

//回复文件大小
class RespondSize: public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "RespondSize";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	RespondSize():m_file_size(0){}
	RespondSize(string &filename, uint64_t file_size)
	{
		m_file_size = file_size;
		m_file_name = filename;
	}

	void assign(const string &file_name, uint64_t file_size)
	{
		m_file_name = file_name;
		m_file_size = file_size;
	}

	const string& get_file_name(){return m_file_name;}
	uint64_t get_file_size(){return m_file_size;}
private:
	uint64_t m_file_size;
	string m_file_name;
};

//请求数据协议
class RequestData: public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "RequestData";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	RequestData():m_start_pos(0), m_size(0){}
	RequestData(string &filename, uint64_t start_pos, unsigned int size)
	{
		m_start_pos = start_pos;
		m_size = size;
		m_file_name = filename;
	}

	void assign(const string &file_name, uint64_t start_pos, unsigned int size)
	{
		m_file_name = file_name;
		m_start_pos = start_pos;
		m_size = size;
	}

	uint64_t get_start_pos(){return m_start_pos;}
	unsigned int get_size(){return m_size;}
	const string& get_file_name(){return m_file_name;}
private:
	uint64_t m_start_pos;
	unsigned int m_size;
	string m_file_name;
};

//回复数据协议
class RespondData: public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "RespondData";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	RespondData():m_start_pos(0), m_size(0){}
	RespondData(string &filename, uint64_t start_pos, unsigned int size)
	{
		m_start_pos = start_pos;
		m_size = size;
		m_file_name = filename;
	}

	void assign(const string &file_name, uint64_t start_pos, unsigned int size)
	{
		m_file_name = file_name;
		m_start_pos = start_pos;
		m_size = size;
	}

	const string& get_file_name(){return m_file_name;}
	uint64_t get_start_pos(){return m_start_pos;}
	unsigned int get_size(){return m_size;}
	const char* get_data(){return m_data;}
private:
	string m_file_name;
	uint64_t m_start_pos;
	unsigned int m_size;
	const char *m_data;
};

class DownloadProtocolFamily:public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
private:
	//memory cache:
	MemCache<RequestSize> m_request_size_memcache;
	MemCache<RespondSize> m_respond_size_memcache;
	MemCache<RequestData> m_request_data_memcache;
	MemCache<RespondData> m_respond_data_memcache;
};

#endif //DOWNLOAD_PROTOCOL_H_



