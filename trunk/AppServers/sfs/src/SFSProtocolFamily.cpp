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
	}

	return protocol;
}

void SFSProtocolFamily::destroy_protocol(Protocol *protocol)
{
	if(protocol != NULL)
		delete protocol;
}

//////////////////////////////  FileInfo Protocol  //////////////////////////////
bool ProtocolFileInfo::encode_body(ByteBuffer *byte_buffer)
{
	if(m_fid.empty())
		return false;
	char query_chunkinfo = m_query_chunkinfo?1:0;
	byte_buffer->append(query_chunkinfo);
	return byte_buffer->append(m_fid.c_str(), m_fid.size());
}

bool ProtocolFileInfo::decode_body(const char *buf, int size)
{
	m_query_chunkinfo = buf[0]==1?true:false;
	++buf;--size;

	m_fid.assign(buf, size);
	return true;
}

//////////////////////////////  FileInfoResp Protocol  //////////////////////////
bool ProtocolFileInfoResp::encode_body(ByteBuffer *byte_buffer)
{
	char result = (char)m_result;
	//1. result
	if(!byte_buffer->append(result))
		return false;
	//2. len of fid
	int fid_len = m_fid.size();
	if(!byte_buffer->append((const char*)&fid_len, sizeof(fid_len)))
		return false;
	//3. fid
	if(!byte_buffer->append(m_fid.c_str(), fid_len))
		return false;

	if(m_result != 0)
	{
		//4. filesize
		if(!byte_buffer->append((const char*)&m_filesize, sizeof(m_filesize)))
			return false;

		//5. chunk info
		if(m_chunkinfo.empty())
		{
			SLOG_ERROR("chunkinfo is empty.");
			return false;
		}
		vector<ChunkInfo>::iterator it;
		for(it=m_chunkinfo.begin(); it!=m_chunkinfo.end(); ++it)
		{
			ChunkInfo &chunk_info = *it;
			int len;

			//1. path
			len = chunk_info.path.size();
			if(!byte_buffer->append((const char*)&len, sizeof(len)))
				return false;
			if(!chunk_info.path.empty() && !byte_buffer->append(chunk_info.path.c_str()))
				return false;
			//2. chunk addr len
			len = chunk_info.chunk_addr.size();
			if(!byte_buffer->append((const char*)&len, sizeof(len)))
				return false;
			//3. chunk addr
			if(!chunk_info.chunk_addr.empty() && !byte_buffer->append(chunk_info.chunk_addr.c_str()))
				return false;
			//4. chunk port
			if(!byte_buffer->append((const char*)&chunk_info.port, sizeof(chunk_info.port)))
				return false;
		}
	}

	return true;
}

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
bool ProtocolFileInfoResp::decode_body(const char *buf, int size)
{
	//1. result
	m_result = (int)buf[0];
	++buf; --size;
	//2. len of fid
	int len = *(int*)buf;
	buf += sizeof(len); size -= sizeof(len);
	//3. fid
	m_fid.assign(buf, len);
	buf += len; size -= len;

	if(m_result != 0)
	{
		//4. filesize
		m_filesize = *(uint64_t*)buf;
		buf+=sizeof(m_filesize); size-=sizeof(m_filesize);

		//5. chunkinfo
		ChunkInfo chunkinfo;
		while(size > 0)
		{
			//path
			if(size < sizeof(len))
				return false;
			len = *(int*)buf;
			buf += sizeof(len); size -= sizeof(len);

			if(size<len || len<0)
				return false;
			chunkinfo.path.assign(buf, len);
			buf+=len; size-=len;

			//chunk addr
			if(size <= 0)
				return false;
			len = *(int*)buf;
			buf+=sizeof(len); size-= sizeof(len);

			if(size<len || len<0)
				return false;
			chunkinfo.chunk_addr.assign(buf, len);
			buf+=len; size-=len;

			//port
			if(size <= 0)
				return false;
			chunkinfo.port = *(int*)buf;
			buf+=sizeof(chunkinfo.port); size-=sizeof(chunkinfo.port);

			m_chunkinfo.push_back(chunkinfo);
		}
	}

	return true;
}

//////////////////////////////  ChunkPing Protocol  //////////////////////////
bool ProtocolChunkPing::encode_body(ByteBuffer *byte_buffer)
{
	//1.chunk port
	if(!byte_buffer->append((const char*)&m_chunk_port, sizeof(m_chunk_port)))
		return false;
	//2. disk space
	if(!byte_buffer->append((const char*)&m_disk_space, sizeof(m_disk_space)))
		return false;
	//3. disk used
	if(!byte_buffer->append((const char*)&m_disk_used, sizeof(m_disk_used)))
		return false;
	//4. chunk id
	int len = m_chunk_id.size();
	if(!byte_buffer->append((const char*)&len, sizeof(len)))
		return false;
	if(!m_chunk_id.empty() && !byte_buffer->append(m_chunk_id.c_str(), len))
		return false;
	return true;
}

bool ProtocolChunkPing::decode_body(const char *buf, int size)
{
	//1. chunk port
	if(size < sizeof(m_chunk_port))
		return false;
	m_chunk_port = *(int*)buf;
	buf+=sizeof(m_chunk_port); size-=sizeof(m_chunk_port);
	//2. disk space
	if(size < sizeof(m_disk_space))
		return false;
	m_disk_space = *(uint64_t*)buf;
	buf+=sizeof(m_disk_space); size-=sizeof(m_disk_space);
	//3. disk used
	if(size < sizeof(m_disk_used))
		return false;
	m_disk_used = *(uint64_t*)buf;
	buf+=sizeof(m_disk_used); size-=sizeof(m_disk_used);

	//4. chunk id
	int len = *(int *)buf;
	buf+=sizeof(len); size-=sizeof(len);
	if(size < len)
		return false;
	if(len > 0)
		m_chunk_id.assign(buf, len);

	return true;
}

//////////////////////////////  ChunkPingResp Protocol  //////////////////////////
bool ProtocolChunkPingResp::encode_body(ByteBuffer *byte_buffer)
{
	//1. result
	if(!byte_buffer->append((const char*)&m_result, sizeof(m_result)))
		return false;
	return true;
}

bool ProtocolChunkPingResp::decode_body(const char *buf, int size)
{
	if(size < sizeof(m_result))
		return false;
	m_result = *(int*)buf;
	buf+=sizeof(m_result); size-= sizeof(m_result);

	return true;
}
