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


#define PROTOCOL_FILE_INFO_REQ          0    //请求文件信息
#define PROTOCOL_FILE_INFO              1    //文件信息
#define PROTOCOL_FILE_INFO_SAVE_RESULT  2    //文件信息保存结果
#define PROTOCOL_FILE_REQ               3    //请求文件数据
#define PROTOCOL_FILE                   4    //文件数据
#define PROTOCOL_FILE_SAVE_RESULT       5    //文件保存结果
#define PROTOCOL_CHUNK_PING             6    //chunk请求master保存chunk信息
#define PROTOCOL_CHUNK_PING_RESP        7    //master回复chunk保存信息结果


///////////////////////////////////////  Protoocl Family  ///////////////////////////////////////
class SFSProtocolFamily:public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
};

///////////////////////////////////////  Protocol  ///////////////////////////////////////
//chunk 路径
class ChunkPath
{
public:
	string id;
	string addr;
	int port;
	int index;
	uint64_t offset;
};

//文件信息
class FileInfo
{
public:
	string fid;
	string name;
	uint64_t size;

	vector<ChunkPath> path_list;
	int get_path_count(){return path_list.size();}
	void add_path(ChunkPath &chunk_path){path_list.push_back(chunk_path);}
	ChunkPath& get_path(int index){return path_list[index];}
};

//chunk 信息
class ChunkInfo
{
public:
	string id;             //chunk id
	string addr;           //chunk addr
	int port;              //chunk port
	uint64_t disk_space;   //磁盘空间
	uint64_t disk_used;    //磁盘已用空间
};

//文件分片
class FileSeg
{
public:
	string fid;          //文件的fid
	string name;         //文件名
	uint64_t filesize;   //文件的大小
	uint64_t offset;     //分片偏移位置
	int index;           //分片序号
	int size;            //分片大小
	const char *data;    //分片数据
};

//////////////////////////////  0. FileInfoReq Protocol  //////////////////////////////
class ProtocolFileInfoReq:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "FileInfoReq Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}

	//query chunk info
	void set_query_chunkpath(bool query_chunkpath){m_query_chunkpath = query_chunkpath?1:0;}
	bool get_query_chunkpath(){return m_query_chunkpath==0?false:true;}
private:
	string m_fid;           //文件的fid
	char m_query_chunkpath; //如果没有文件信息,请求分配chunk
};

//////////////////////////////  1. FileInfo Protocol  //////////////////////////////
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
	//result: 0(失败)，1(文件存在,返回文件信息)，2(文件不存在,返回分配的chunk)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	//用于设置/获取文件信息
	FileInfo& get_fileinfo();
private:
	int m_result;	//查询结果
	FileInfo m_fileinfo; //当result为0时无效
};

//////////////////////////////  2. ProtocolFileInfoSaveResult Protocol  //////////////////////////////
class ProtocolFileInfoSaveResult:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "FileInfoSaveResult Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//result: 0(失败)，1(文件存在,返回文件信息)，2(文件不存在,返回分配的chunk)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	//用于设置/获取文件信息
	void set_fid(string &fid){m_fid=fid;}
	const string& get_fid(){return m_fid;}
private:
	int m_result;	//保存结果:0成功, 其他失败
	string m_fid;
};

//////////////////////////////  3. ProtocolFileReq Protocol  //////////////////////////////
class ProtocolFileReq:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "FileReq Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//chunk path
	void set_chunk_path(const string &chunk_path){m_chunk_path = chunk_path;}
	const string& get_chunk_path(){return m_chunk_path;}
private:
	string m_chunk_path;
};

//////////////////////////////  4. ProtocolFile Protocol  //////////////////////////////
class ProtocolFile:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "File Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//0(file_seg有效); 1(file_seg无效)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	FileSeg& get_file_seg(){return m_file_seg;}
private:
	int m_result;
	FileSeg m_file_seg;
};

//////////////////////////////  5. FileSaveResult Protocol  //////////////////////////////
class ProtocolFileSaveResult:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "SaveResult Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//result:0(成功), 1(失败)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}
	//file_seg
	FileSeg& get_file_seg(){return m_file_seg;}
private:
	int m_result;
	FileSeg m_file_seg;
};

//////////////////////////////  6. ChunkPing Protocol  //////////////////////////////
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
	//chunk info
	ChunkInfo& get_chunk_info();

private:
	ChunkInfo m_chunk_info;
};

//////////////////////////////  7. ChunkPingResp Protocol  //////////////////////////////
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
	//result:0(失败), 1(成功)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	//chunk id
	void set_chunk_id(const string &chunk_id){m_chunk_id = chunk_id;}
	const string& get_chunk_id(){return m_chunk_id;}
private:
	int m_result;
	string m_chunk_id;
};

#endif //_SFS_PROTOCOL_FAMILY_H_


