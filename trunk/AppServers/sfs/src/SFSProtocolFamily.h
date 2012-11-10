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


#define PROTOCOL_FILE_INFO              0    //client请求master文件信息
#define PROTOCOL_FILE_INFO_RESP         1    //master回复client文件信息
#define PROTOCOL_CHUNK_PING             2    //chunk请求master保存chunk信息
#define PROTOCOL_CHUNK_PING_RESP        3    //mastet回复chunk保存信息结果
#define PROTOCOL_CHUNK_REPORT           4    //chunk请求master保存信息
#define PROTOCOL_CHUNK_REPORT_RESP      5    //master回复chunk保存结果
#define PROTOCOL_STORE                  6    //client请求chunk存储文件
#define PROTOCOL_STORE_RESP             7    //chunk回复client存储结果
#define PROTOCOL_RETRIEVE               8    //client请求chunk获取文件
#define PROTOCOL_RETRIEVE_RESP          9    //chunk回复client文件数据


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

//////////////////////////////  FileInfo Protocol  //////////////////////////////
//查询master获取FID对应的文件信息
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
	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}

	//query chunk info
	void set_query_chunkinfo(bool query_chunkinfo){m_query_chunkinfo = query_chunkinfo?1:0;}
	bool get_query_chunkinfo(){return m_query_chunkinfo==0?false:true;}
private:
	string m_fid;           //文件的fid
	char m_query_chunkinfo; //如果没有gpath,是否返回chunkinfo
};

//////////////////////////////  FileInfoResp Protocol  //////////////////////////////
//master回复获取文件信息
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
	//result: 0(失败,其他字段无效),1(成功),2(chunk的地址有效,请求协议设置了query_chunkinfo)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}

	//file name
	void set_file_name(const string &filename){m_filename = filename;}
	const string& get_file_name(){return m_filename;}

	//file size
	void set_filesize(uint64_t filesize){m_filesize = filesize;}
	uint64_t get_fielsize(){return m_filesize;}

	//chunk info
	void add_chunkinfo(ChunkInfo &chunk_info){m_chunkinfo.push_back(chunk_info);	}
	const vector<ChunkInfo>& get_chunkinfo(){return m_chunkinfo;}
private:
	int m_result;	//查询结果
	string m_fid;
	string m_filename;
	uint64_t m_filesize;
	vector<ChunkInfo> m_chunkinfo;
};

//////////////////////////////  ChunkPing Protocol  //////////////////////////////
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
	ProtocolChunkPing()
		:m_chunk_port(-1)
		,m_disk_space(0)
		,m_disk_used(0)
	{}

	//chunk id
	void set_chunk_id(const string &chunk_id){m_chunk_id = chunk_id;}
	const string& get_chunk_id(){return m_chunk_id;}

	//chunk地址
	void set_chunk_addr(const string &chunk_addr){m_chunk_addr = chunk_addr;}
	const string& get_chunk_addr(){return m_chunk_addr;}

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
	string m_chunk_addr;    //chunk的地址
	int m_chunk_port;       //chunk的端口
	uint64_t m_disk_space;  //磁盘空间
	uint64_t m_disk_used;   //磁盘已用空间
};

//////////////////////////////  ChunkPingResp Protocol  //////////////////////////////
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

//////////////////////////////  ChunkReport Protocol  //////////////////////////////
class ProtocolChunkReport:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkPingResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	ProtocolChunkReport():m_filesize(0){}

	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}

	//chunk id
	void set_chunk_id(const string &chunk_id){m_chunk_id = chunk_id;}
	const string& get_chunk_id(){return m_chunk_id;}

	//file name
	void set_file_name(const string &filename){m_filename = filename;}
	const string& get_file_name(){return m_filename;}

	//file size
	void set_file_size(uint64_t filesize){m_filesize = filesize;}
	uint64_t get_file_size(){return m_filesize;}
private:
	string m_chunk_id;
	string m_fid;
	string m_filename;
	uint64_t m_filesize;
};

//////////////////////////////  ChunkReportResp Protocol  //////////////////////////////
class ProtocolChunkReportResp:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkReportResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//result:0(失败), 1(成功)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}

	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}
private:
	int m_result;
	string m_fid;
};

//////////////////////////////  ChunkReportResp Protocol  //////////////////////////////
class ProtocolStore:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkReportResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	ProtocolStore()
		:m_filesize(0)
		,m_segindex(-1)
		,m_segsize(0)
		,m_segfinished(0)
		,m_data(NULL)
	{}

	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}
	//file name
	void set_file_name(const string &filename){m_filename = filename;}
	const string& get_file_name(){return m_filename;}
	//file size
	void set_file_size(uint64_t filesize){m_filesize = filesize;}
	uint64_t get_file_size(){return m_filesize;}
	//seg index
	void set_seg_index(int segindex){m_segindex = segindex;}
	uint64_t get_seg_index(){return m_segindex;}
	//seg size
	void set_seg_size(int segsize){m_segsize = segsize;}
	uint64_t get_seg_size(){return m_segsize;}
	//seg finisned
	void set_seg_finished(bool finished){m_segfinished = finished?1:0;}
	bool get_seg_finished(){return m_segfinished==0?false:true;}

	//data
	const char* get_data(){return m_data;}
private:
	string m_fid;
	string m_filename;
	uint64_t m_filesize;
	int m_segindex;
	int m_segsize;
	char m_segfinished;
	const char *m_data;
};

//////////////////////////////  ChunkReportResp Protocol  //////////////////////////////
class ProtocolStoreResp:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkReportResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//result:0(失败), 1(成功)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}
	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}
	//chunk path
	void set_chunk_path(const string &chunk_path){m_chunk_path = chunk_path;}
	const string& get_chunk_path(){return m_chunk_path;}
private:
	int m_result;
	string m_fid;
	string m_chunk_path;
};

//////////////////////////////  Retrieve Protocol  //////////////////////////////
class ProtocolRetrieve:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkReportResp Protocol";}
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

//////////////////////////////  RetrieveResp Protocol  //////////////////////////////
class ProtocolRetrieveResp:public Protocol
{
public://实现protocol的接口
	//协议的描述信息
	const char* details(){return "ChunkReportResp Protocol";}
	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	//result:0(失败), 1(成功)
	void set_result(int result){m_result = result;}
	int get_result(){return m_result;}
	//fid
	void set_fid(const string &fid){m_fid = fid;}
	const string& get_fid(){return m_fid;}
	//file size
	void set_file_size(uint64_t filesize){m_filesize = filesize;}
	uint64_t get_file_size(){return m_filesize;}
	//seg index
	void set_seg_index(int segindex){m_segindex = segindex;}
	uint64_t get_seg_index(){return m_segindex;}
	//seg size
	void set_seg_size(int segsize){m_segsize = segsize;}
	uint64_t get_seg_size(){return m_segsize;}
	//seg finisned
	void set_seg_finished(bool finished){m_segfinished = finished?1:0;}
	bool get_seg_finished(){return m_segfinished==0?false:true;}

	//data
	const char* get_data(){return m_data;}
private:
	int m_result;
	string m_fid;
	uint64_t m_filesize;
	int m_segindex;
	int m_segsize;
	char m_segfinished;
	const char *m_data;
};
#endif //_SFS_PROTOCOL_FAMILY_H_


