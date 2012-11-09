/*
 * SFSProtocolFamily.h
 *
 *  Created on: 2012-11-7
 *      Author: LiuYongJin
 */

#ifndef _SFS_PROTOCOL_FAMILY_H_
#define _SFS_PROTOCOL_FAMILY_H_

#include "DefaultProtocolFamily.h"
#include <stdint.h>
#include <string>
#include <vector>
using std::string;
using std::vector;


#define PROTOCOL_FILE_INFO        1    //client发送FILE_INFO请求到master获取文件信息
#define PROTOCOL_FILE_INFO_RESP   2    //master回复FILE_INFO请求

#define PROTOCOL_CHUNK_PING       3    //chunk发送CHUNK_PING请求到master上报自己的信息
#define PROTOCOL_CHUNK_PING_RESP  4    //mastet回复CHUNK_PING

class SFSProtocolFamily:public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
};

class ChunkInfo
{
public:
	string path;        //FID_ChunkID_Location
	string chunk_addr;  //chunk address
	int port;           //chunk port
};

//查询master获取FID对应的GPATH
class ProtocolFileInfo:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "FileInfo Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}

	void set_query_chunkinfo(bool query_chunkinfo){m_query_chunkinfo = query_chunkinfo;}
	bool get_query_chunkinfo(){return m_query_chunkinfo;}
private:
	string m_fid;           //文件的fid
	bool m_query_chunkinfo; //如果没有gpath,是否返回chunkinfo
};

//master回复GPATH
class ProtocolFileInfoResp:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "FileInfoResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//设置查询结果
	void set_result(int result){m_result = result;}
	//查询结果:0(失败,其他字段无效),1(成功),2(chunk的地址有效,请求协议设置了query_chunkinfo)
	int get_result(){return m_result;}

	//设置请求到fid
	void set_fid(const string &fid){m_fid = fid;}
	//获取请求到fid
	const string& get_fid(){return m_fid;}

	//设置文件大小
	void set_filesize(uint64_t filesize){m_filesize = filesize;}
	//获取文件大小
	uint64_t get_fielsize(){return m_filesize;}

	//添加chunk_info
	void add_chunkinfo(ChunkInfo &chunk_info){m_chunkinfo.push_back(chunk_info);	}
	//获取gpath列表
	const vector<ChunkInfo>& get_chunkinfo(){return m_chunkinfo;}

private:
	int m_result;	//查询结果
	string m_fid;
	uint64_t m_filesize;
	vector<ChunkInfo> m_chunkinfo;
};

class ProtocolChunkPing:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkPing Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//chunk id
	void set_chunk_id(const string &chunk_id){m_chunk_id = chunk_id;}
	const string& get_chunk_id(){return m_chunk_id;}

	//chunk的端口号
	void set_chunk_port(int chunk_port){m_chunk_port = chunk_port;}
	int get_chunk_port(){return m_chunk_port;}

	//磁盘空间
	void set_disk_space(uint64_t disk_space){m_disk_space = disk_space;}
	uint64_t get_disk_space(){return m_disk_space;}

	//磁盘已用空间
	void set_disk_used(uint64_t disk_used){m_disk_used = disk_used;}
	uint64_t get_disk_used(){return m_disk_used;}

private:
	string m_chunk_id;      //chunk id
	int m_chunk_port;       //chunk的端口
	uint64_t m_disk_space;  //磁盘空间
	uint64_t m_disk_used;   //磁盘已用空间

};

class ProtocolChunkPingResp:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkPingResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//结果:0(成功), 1(失败)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}
private:
	int m_result;
};

#endif //_SFS_PROTOCOL_FAMILY_H_


