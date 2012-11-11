/*
 * SFSProtocolFamily.cpp
 *
 *  Created on: 2012-11-7
 *      Author: LiuYongJin
 */

#include "SFSProtocolFamily.h"

/////////////////////////////////////////////////////////////
//编码字符
#define ENCODE_CHAR(c) do{ \
	if(!byte_buffer->append(c)) \
		return false; \
}while(0)
//解码字符
#define DECODE_CHAR(c) do{ \
	if(size < sizeof(c)) return false; \
	c = buf[0]; ++buf; --size; \
}while(0)

//编码整数
#define ENCODE_INT(i) do{ \
	if(!byte_buffer->append((const char*)&i, sizeof(i))) \
		return false; \
}while(0)
//解码整数
#define DECODE_INT(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

//编码64位整数
#define ENCODE_INT64(i) ENCODE_INT(i)
//解码整数
#define DECODE_INT64(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int64_t*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

//编码字符串
#define ENCODE_STRING(str) do{\
	len = str.size(); \
	if(!byte_buffer->append((const char*)&len, sizeof(len))) \
		return false; \
	if(len > 0 && !byte_buffer->append(str.c_str())) \
		return false; \
}while(0)
//解码字符串
#define DECODE_STRING(str) do{\
	DECODE_INT(len); \
	if(len<0 || size<len) return false; \
	if(len > 0) \
		str.assign(buf, len); buf+=len; size-=len; \
}while(0)

/////////////////////////////////////////////////////////////

Protocol* SFSProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
	Protocol *protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_FILE_INFO:
		protocol = new ProtocolFileInfo;
		break;
	case PROTOCOL_FILE_INFO_RESP:
		protocol = new ProtocolFileInfoResp;
		break;
	case PROTOCOL_CHUNK_PING:
		protocol = new ProtocolChunkPing;
		break;
	case PROTOCOL_CHUNK_PING_RESP:
		protocol = new ProtocolChunkPingResp;
		break;
	case PROTOCOL_STORE:
		protocol = new ProtocolStore;
		break;
	case PROTOCOL_STORE_RESP:
		protocol = new ProtocolStoreResp;
		break;
	case PROTOCOL_RETRIEVE:
		protocol = new ProtocolRetrieve;
		break;
	case PROTOCOL_RETRIEVE_RESP:
		protocol = new ProtocolRetrieveResp;
		break;
	}

	return protocol;
}

void SFSProtocolFamily::destroy_protocol(Protocol *protocol)
{
	if(protocol != NULL)
		delete protocol;
}

//////////////////////////////  1. FileInfo Protocol  //////////////////////////////
bool ProtocolFileInfo::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////fid
	ENCODE_STRING(m_fid);
	////query chunkinfo
	ENCODE_CHAR(m_query_chunkinfo);

	return true;
}

bool ProtocolFileInfo::decode_body(const char *buf, int size)
{
	int len = 0;
	////fid
	DECODE_STRING(m_fid);
	////query chunkinfo
	DECODE_CHAR(m_query_chunkinfo);
	return true;
}

//////////////////////////////  2. FileInfoResp Protocol  //////////////////////////
bool ProtocolFileInfoResp::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_fid);
	////file name
	ENCODE_STRING(m_filename);
	////file size
	ENCODE_INT64(m_filesize);
	////chunk info
	len = m_chunkinfo.size();
	ENCODE_INT(len);
	vector<ChunkInfo>::iterator it;
	for(it=m_chunkinfo.begin(); it!=m_chunkinfo.end(); ++it)
	{
		ChunkInfo &chunk_info = *it;
		//chunk path
		ENCODE_STRING(chunk_info.path);
		//chunk addr len
		ENCODE_STRING(chunk_info.chunk_addr);
		//chunk port
		ENCODE_INT(chunk_info.port);
	}

	return true;
}

bool ProtocolFileInfoResp::decode_body(const char *buf, int size)
{
	int len = 0;
	////result
	DECODE_INT(m_result);
	////fid
	DECODE_STRING(m_fid);
	////file name
	DECODE_STRING(m_filename);
	////filesize
	DECODE_INT64(m_filesize);

	//chunk info
	DECODE_INT(len);
	if(len > 0 )
	{	////chunkinfo
		ChunkInfo chunkinfo;
		while(size > 0)
		{
			//chunk path
			DECODE_STRING(chunkinfo.path);
			//chunk addr
			DECODE_STRING(chunkinfo.chunk_addr);
			//port
			DECODE_INT(chunkinfo.port);

			m_chunkinfo.push_back(chunkinfo);
		}
	}

	return true;
}

//////////////////////////////  3. ChunkPing Protocol  //////////////////////////
bool ProtocolChunkPing::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////chunk id
	ENCODE_STRING(m_chunk_id);
	////chunk addr
	ENCODE_STRING(m_chunk_addr);
	////chunk port
	ENCODE_INT(m_chunk_port);
	//// disk space
	ENCODE_INT64(m_disk_space);
	////disk used
	ENCODE_INT64(m_disk_used);

	return true;
}

bool ProtocolChunkPing::decode_body(const char *buf, int size)
{
	int len = 0;
	////chunk id
	DECODE_STRING(m_chunk_id);
	////chunk addr
	DECODE_STRING(m_chunk_addr);
	////chunk port
	DECODE_INT(m_chunk_port);
	////disk space
	DECODE_INT64(m_disk_space);
	////disk used
	DECODE_INT64(m_disk_used);

	return true;
}

//////////////////////////////  4. ChunkPingResp Protocol  //////////////////////////
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

//////////////////////////////  5. ChunkReport Protocol  //////////////////////////
bool ProtocolChunkReport::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////fid
	ENCODE_STRING(m_fid);
	////chunk id
	ENCODE_STRING(m_chunk_id);
	////file name
	ENCODE_STRING(m_filename);
	////file size
	ENCODE_INT64(m_filesize);

	return true;
}

bool ProtocolChunkReport::decode_body(const char *buf, int size)
{
	int len = 0;
	////fid
	DECODE_STRING(m_fid);
	////file name
	DECODE_STRING(m_chunk_id);
	////file name
	DECODE_STRING(m_filename);
	////file size
	DECODE_INT64(m_filesize);

	return true;
}

//////////////////////////////  6. ChunkReportResp Protocol  //////////////////////////
bool ProtocolChunkReportResp::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_fid);

	return true;
}

bool ProtocolChunkReportResp::decode_body(const char *buf, int size)
{
	int len = 0;
	////result
	DECODE_INT(m_result);
	////fid
	DECODE_STRING(m_fid);

	return true;
}

//////////////////////////////  7. Store Protocol  //////////////////////////
bool ProtocolStore::encode_body(ByteBuffer *byte_buffer)
{
	int len =0;
	////fid
	ENCODE_STRING(m_fid);
	////file name
	ENCODE_STRING(m_filename);
	////file size
	ENCODE_INT64(m_filesize);
	////seg offset
	ENCODE_INT64(m_segoffset);
	////seg index
	ENCODE_INT(m_segindex);
	////seg size
	ENCODE_INT(m_segsize);
	////seg finished
	ENCODE_CHAR(m_segfinished);

	return true;
}

bool ProtocolStore::decode_body(const char *buf, int size)
{
	int len = 0;
	//fid
	DECODE_STRING(m_fid);
	//file name
	DECODE_STRING(m_filename);
	//file size
	DECODE_INT64(m_filesize);
	////seg offset
	DECODE_INT64(m_segoffset);
	//seg index
	DECODE_INT(m_segindex);
	//seg size
	DECODE_INT(m_segsize);
	//seg finished
	DECODE_CHAR(m_segfinished);
	//data
	if(size<m_segsize || m_segsize<=0) return false;
	m_data = buf;

	return true;
}

//////////////////////////////  8. StoreResp Protocol  //////////////////////////
bool ProtocolStoreResp::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//result
	ENCODE_INT(m_result);
	////fid
	ENCODE_STRING(m_fid);
	////chunk path
	ENCODE_STRING(m_chunk_path);

	return true;
}

bool ProtocolStoreResp::decode_body(const char *buf, int size)
{
	int len = 0;
	//result
	DECODE_INT(m_result);
	////fid
	DECODE_STRING(m_fid);
	////chunk path
	DECODE_STRING(m_chunk_path);

	return true;
}

//////////////////////////////  9. Retrieve Protocol  //////////////////////////
bool ProtocolRetrieve::encode_body(ByteBuffer *byte_buffer)
{
	int len =0;
	////chunk path
	ENCODE_STRING(m_chunk_path);

	return true;
}

bool ProtocolRetrieve::decode_body(const char *buf, int size)
{
	int len = 0;
	//fid
	DECODE_STRING(m_chunk_path);

	return true;
}

//////////////////////////////  10. RetrieveResp Protocol  //////////////////////////
bool ProtocolRetrieveResp::encode_body(ByteBuffer *byte_buffer)
{
	int len =0;
	////fid
	ENCODE_STRING(m_fid);
	////file size
	ENCODE_INT64(m_filesize);
	////seg index
	ENCODE_INT(m_segindex);
	////seg size
	ENCODE_INT(m_segsize);
	////seg finished
	ENCODE_CHAR(m_segfinished);

	return true;
}

bool ProtocolRetrieveResp::decode_body(const char *buf, int size)
{
	int len = 0;
	//fid
	DECODE_STRING(m_fid);
	//file size
	DECODE_INT64(m_filesize);
	//seg index
	DECODE_INT(m_segindex);
	//seg size
	DECODE_INT(m_segsize);
	//seg finished
	DECODE_CHAR(m_segfinished);
	//data
	if(size<m_segsize || m_segsize<=0) return false;
	m_data = buf;

	return true;
}
