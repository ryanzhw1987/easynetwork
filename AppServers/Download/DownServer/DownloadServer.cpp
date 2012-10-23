/*
 * DownloadServer.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */
#include "DownloadServer.h"
#include "IODemuxerEpoll.h"
#include "DownloadProtocol.h"
#include "slog.h"

#include <stdio.h>

//////////////////由应用层重写 接收协议函数//////////////////
void DownloadServer::run_thread()
{
	SLOG_INFO("MTServerAppFramework[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("MTServerAppFramework end...");
}

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* DownloadServer::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void DownloadServer::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* DownloadServer::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void DownloadServer::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* DownloadServer::create_protocol_family()
{
	return new DownloadProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void DownloadServer::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

/////////////////////////////////////  实现 NetInterface的接口  ///////////////////////
bool DownloadServer::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DownloadProtocolFamily* protocol_family = (DownloadProtocolFamily*)get_protocol_family();
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_REQUEST_SIZE:
	{
		RequestSize *temp_protocol = (RequestSize*)protocol;
		const string file_name = temp_protocol->get_file_name();
		SLOG_INFO("receive <RequestSize:file=%s>", file_name.c_str());

		string path="/data/";
		path += file_name;

		//get file size
		unsigned long long file_size=0;
		FILE *fp = fopen(path.c_str(), "r");
		if(fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fclose(fp);
		}

		RespondSize* resp_protocol = (RespondSize*)protocol_family->create_protocol(PROTOCOL_RESPOND_SIZE);
		if(resp_protocol)
		{
			resp_protocol->assign(file_name, file_size);
			send_protocol(socket_handle, resp_protocol);
		}
		else
			SLOG_ERROR("create RespondSize protocol failed.");

		break;
	}
	case PROTOCOL_REQUEST_DATA:
	{
		RequestData *temp_protocol = (RequestData*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long start_pos = temp_protocol->get_start_pos();
		unsigned int size = temp_protocol->get_size();
		SLOG_INFO("receive <RequestData: file=%s, start_pos=%ld, size=%d>", file_name.c_str(), start_pos, size);

		string path="/data/";
		path += file_name;
		FILE *fp = fopen(path.c_str(), "r");
		if(fp != NULL)
		{
			RespondData* resp_protocol = (RespondData*)protocol_family->create_protocol(PROTOCOL_RESPOND_DATA);
			resp_protocol->assign(file_name, start_pos, size);

			DefaultProtocolHeader *header = (DefaultProtocolHeader *)resp_protocol->get_protocol_header();
			int header_length = header->get_header_length();
			ByteBuffer *byte_buffer = new ByteBuffer;
			//1. 预留协议头空间
			byte_buffer->get_append_buffer(header_length);
			byte_buffer->set_append_size(header_length);
			//2. 编码协议体数据
			resp_protocol->encode_body(byte_buffer);
			//3. 添加数据
			char *data_buffer = byte_buffer->get_append_buffer(size);
			fseek(fp, start_pos, SEEK_SET);
			fread(data_buffer, 1, size, fp);
			fclose(fp);
			byte_buffer->set_append_size(size);
			//4. 编码协议头
			int body_length = byte_buffer->size()-header_length;
			char *header_buffer = byte_buffer->get_data(header_length);
			header->encode(header_buffer, body_length);
			//5. 发送协议
			resp_protocol->attach_raw_data(byte_buffer);
			send_protocol(socket_handle, resp_protocol);
		}
		else
		{
			SLOG_ERROR("can't open file=%s", file_name.c_str());
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		return false;
	}

	return true;
}

bool DownloadServer::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool DownloadServer::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool DownloadServer::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool DownloadServer::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return true;
}


///////////////////////////////  thread pool  //////////////////////////////////
Thread<SocketHandle>* DownloadThreadPool::create_thread()
{
	DownloadServer* temp = new DownloadServer();
	temp->start_instance();
	temp->set_idle_timeout(30000);
	return (Thread<SocketHandle>*)temp;
}

///////////////////////////////  Timer Handler /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_INFO("timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);

	return HANDLE_OK;
}
