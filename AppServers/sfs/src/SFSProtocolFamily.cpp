/*
 * SFSProtocolFamily.cpp
 *
 *  Created on: 2012-11-7
 *      Author: LiuYongJin
 */

#include "SFSProtocolFamily.h"

Protocol* SFSProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
	Protocol *protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_FILE_INFO_REQ:
		protocol = new ProtocolFileInfoReq;
		break;
	case PROTOCOL_FILE_INFO:
		protocol = new ProtocolFileInfo;
		break;
	case PROTOCOL_FILE_REQ:
		protocol = new ProtocolFileReq;
		break;
	case PROTOCOL_FILE:
		protocol = new ProtocolFile;
		break;
	case PROTOCOL_FILE_SAVE_RESULT:
		protocol = new ProtocolFileSaveResult;
		break;
	case PROTOCOL_CHUNK_PING:
		protocol = new ProtocolChunkPing;
		break;
	case PROTOCOL_CHUNK_PING_RESP:
		protocol = new ProtocolChunkPingResp;
		break;
	}

	return protocol;
}

void SFSProtocolFamily::destroy_protocol(Protocol *protocol)
{
	if(protocol != NULL)
		delete protocol;
}

//////////////////////////////  0. FileInfoReq Protocol  //////////////////////////////
bool ProtocolFileInfoReq::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////fid
	ENCODE_STRING(m_fid);
	////query chunk path
	ENCODE_CHAR(m_query_chunkpath);

	return true;
}

bool ProtocolFileInfoReq::decode_body(const char *buf, int size)
{
	int len = 0;
	////fid
	DECODE_STRING(m_fid);
	////query chunk path
	DECODE_CHAR(m_query_chunkpath);
	return true;
}

//////////////////////////////  1. FileInfo Protocol  //////////////////////////
bool ProtocolFileInfo::encode_body(ByteBuffer *byte_buffer)
{
	////result
	ENCODE_INT(m_result);
	if(m_result == 0) return true;

	////fid
	ENCODE_STRING(m_fileinfo.fid);
	////file name
	ENCODE_STRING(m_fileinfo.name);
	////file size
	ENCODE_INT64(m_fileinfo.size);
	////chunk info
	int count = m_fileinfo.path_list.size();
	ENCODE_INT(count);
	vector<ChunkPath>::iterator it;
	for(it=m_fileinfo.path_list.begin(); it!=m_fileinfo.path_list.end(); ++it)
	{
		//chunk id
		ENCODE_STRING(it->id);
		//chunk addr
		ENCODE_STRING(it->addr);
		//chunk port
		ENCODE_INT(it->port);
		//index
		ENCODE_INT(it->index);
		//chunk offset
		ENCODE_INT64(it->offset);
	}

	return true;
}

bool ProtocolFileInfo::decode_body(const char *buf, int size)
{
	////result
	DECODE_INT(m_result);
	if(m_result == 0) return true;

	////fid
	DECODE_STRING(m_fileinfo.fid);
	////file name
	DECODE_STRING(m_fileinfo.name);
	////file size
	DECODE_INT64(m_fileinfo.size);
	////chunk info
	int count = 0;
	DECODE_INT(count);
	while(count > 0)
	{
		--count;
		ChunkPath chunkpath;
		//chunk id
		DECODE_STRING(chunkpath.id);
		//chunk addr
		DECODE_STRING(chunkpath.addr);
		//chunk port
		DECODE_INT(chunkpath.port);
		//index
		DECODE_INT(chunkpath.index);
		//chunk offset
		DECODE_INT64(chunkpath.offset);
		m_fileinfo.add_path(chunkpath);
	}

	return true;
}

//////////////////////////////  2. FileInfoSaveResult Protocol  //////////////////////////
bool ProtocolFileInfoSaveResult::encode_body(ByteBuffer *byte_buffer)
{
	////result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_fid);

	return true;
}

bool ProtocolFileInfoSaveResult::decode_body(const char *buf, int size)
{
	////result
	DECODE_INT(m_result);
	////fid
	DECODE_STRING(m_fid);

	return true;
}

//////////////////////////////  3. FileReq Protocol  //////////////////////////
bool ProtocolFileReq::encode_body(ByteBuffer *byte_buffer)
{
	int len =0;
	////chunk path
	ENCODE_STRING(m_chunk_path);

	return true;
}

bool ProtocolFileReq::decode_body(const char *buf, int size)
{
	int len = 0;
	//fid
	DECODE_STRING(m_chunk_path);

	return true;
}

//////////////////////////////  4. File Protocol  //////////////////////////
bool ProtocolFile::encode_body(ByteBuffer *byte_buffer)
{
	////result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_file_seg.fid);
	////name
	ENCODE_STRING(m_file_seg.name);
	////file size
	ENCODE_INT64(m_file_seg.filesize);
	////seg offset
	ENCODE_INT64(m_file_seg.offset);
	////seg index
	ENCODE_INT(m_file_seg.index);
	////seg size
	ENCODE_INT(m_file_seg.size);

	return true;
}

bool ProtocolFile::decode_body(const char *buf, int size)
{
	////result
	DECODE_INT(m_result);
	if(m_result == 1) return true;
	////fid
	DECODE_STRING(m_file_seg.fid);
	////name
	DECODE_STRING(m_file_seg.name);
	////file size
	DECODE_INT64(m_file_seg.filesize);
	////seg offset
	DECODE_INT64(m_file_seg.offset);
	////seg index
	DECODE_INT(m_file_seg.index);
	////seg size
	DECODE_INT(m_file_seg.size);
	////data
	if(size<m_file_seg.size || m_file_seg.size<=0) return false;
	m_file_seg.data = buf;

	return true;
}

//////////////////////////////  5. FileSaveResult Protocol  //////////////////////////
bool ProtocolFileSaveResult::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_file_seg.fid);
	////seg index
	ENCODE_INT(m_file_seg.index);

	return true;
}

bool ProtocolFileSaveResult::decode_body(const char *buf, int size)
{
	int len = 0;
	//result
	DECODE_INT(m_result);
	////fid
	DECODE_STRING(m_file_seg.fid);
	////seg index
	DECODE_INT(m_file_seg.index);

	return true;
}

//////////////////////////////  6. ChunkPing Protocol  //////////////////////////
bool ProtocolChunkPing::encode_body(ByteBuffer *byte_buffer)
{
	////chunk id
	ENCODE_STRING(m_chunk_info.id);
	////chunk addr
	ENCODE_STRING(m_chunk_info.addr);
	////chunk port
	ENCODE_INT(m_chunk_info.port);
	//// disk space
	ENCODE_INT64(m_chunk_info.disk_space);
	////disk used
	ENCODE_INT64(m_chunk_info.disk_used);

	return true;
}

bool ProtocolChunkPing::decode_body(const char *buf, int size)
{
	////chunk id
	DECODE_STRING(m_chunk_info.id);
	////chunk addr
	DECODE_STRING(m_chunk_info.addr);
	////chunk port
	DECODE_INT(m_chunk_info.port);
	////disk space
	DECODE_INT64(m_chunk_info.disk_space);
	////disk used
	DECODE_INT64(m_chunk_info.disk_used);

	return true;
}

//////////////////////////////  7. ChunkPingResp Protocol  //////////////////////////
bool ProtocolChunkPingResp::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////result
	ENCODE_INT(m_result);
	////chunk id
	ENCODE_STRING(m_chunk_id);

	return true;
}

bool ProtocolChunkPingResp::decode_body(const char *buf, int size)
{
	int len = 0;
	////result
	DECODE_INT(m_result);
	////chunk id
	DECODE_STRING(m_chunk_id);

	return true;
}

