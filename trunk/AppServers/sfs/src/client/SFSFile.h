/*
 * SFSClient.h
 *
 *  Created on: 2012-11-8
 *      Author: LiuYongJin
 */

#ifndef _LIB_SFS_CLIENT_H_20121108
#define _LIB_SFS_CLIENT_H_20121108

#include "ByteBuffer.h"
#include <stdint.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

#include "SFSProtocolFamily.h"
namespace SFS
{

class FileInfo
{
public:
	int result;
	string fid;
	uint64_t size;
	vector<ChunkInfo> chunkinfo;
};

class File
{
public:
	File(string &master_addr, int master_port, int n_replica);

	bool file_info(string &fid, FileInfo &file_info);
	//从sfs读取fid文件的数据保存到out_buf中
	bool retrieve(string &fid, ByteBuffer *out_buf);
	//从sfs读取fid文件的数据保存到out_file文件中
	bool retrieve(string &fid, string &out_file);
	//存储local_file文件到sfs
	bool store(string &local_file);
private:
	string m_master_addr;
	int    m_master_port;
	int    m_n_replica;
	SFSProtocolFamily m_protocol_family;
private:
	Protocol* query_master(Protocol *protocol);
};

}
#endif //_LIB_SFS_CLIENT_H_20121108


