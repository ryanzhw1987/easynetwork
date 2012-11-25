/*
 * SFSClient.h
 *
 *  Created on: 2012-11-8
 *      Author: LiuYongJin
 */

#ifndef _LIB_SFS_CLIENT_H_20121108
#define _LIB_SFS_CLIENT_H_20121108

#include "Socket.h"
#include "ByteBuffer.h"
#include "SFSProtocolFamily.h"

#include <stdint.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace SFS
{

class File
{
public:
	File(string &master_addr, int master_port, int n_replica);

	//获取fid的文件信息
	//query_chunkpath: 当没有文件信息的时候是否请求分配chunk path
	//返回值:
	//0:失败(没有文件信息/分配chunk path失败)
	//1:有文件信息
	//2:没有文件信息但分配chunk path成功
	int get_file_info(FileInfo &file_info, string &fid, bool query_chunkpath=false);

	//从sfs读取fid文件保存到local_file中
	bool get_file(string &fid, string &local_file);

	//将文件local_file保存到sfs系统
	//失败返回false; 成功返回true,fileinfo表示保存后的文件信息
	bool save_file(FileInfo &fileinfo, string &local_file);
private:
	string m_master_addr;
	int    m_master_port;
	int    m_n_replica;
	SFSProtocolFamily m_protocol_family;
private:
	Protocol* query_master(Protocol *protocol);
	bool send_file_to_chunk(string &local_file, string &fid, string &chunk_addr, int chunk_port);
	bool send_file_protocol_to_chunk(TransSocket* trans_socket, ProtocolFile *protocol_store, ByteBuffer *byte_buffer, int fd);
};

}
#endif //_LIB_SFS_CLIENT_H_20121108


