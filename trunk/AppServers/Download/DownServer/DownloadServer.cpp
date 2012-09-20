/*
 * DownloadServer.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */
#include "DownloadServer.h"
#include "IODemuxerEpoll.h"
#include "slog.h"

#include <stdio.h>

HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_INFO("timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);

	return HANDLE_OK;
}

//////////////////由应用层重写 接收协议函数//////////////////
void DownloadServer::run()
{
	SLOG_INFO("MTServerAppFramework[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("MTServerAppFramework end...");
}

int DownloadServer::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
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
		DownloadProtocolFamily* protocol_family = (DownloadProtocolFamily*)get_protocol_family();
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
			IOBuffer io_buffer;
			fseek(fp, start_pos, SEEK_SET);
			char* buf = io_buffer.write_open(size);
			fread(buf, 1, size, fp);
			fclose(fp);
			string data;
			data.assign(buf, size);
			io_buffer.write_close(size);

			DownloadProtocolFamily* protocol_family = (DownloadProtocolFamily*)get_protocol_family();
			RespondData* resp_protocol = (RespondData*)protocol_family->create_protocol(PROTOCOL_RESPOND_DATA);
			if(resp_protocol)
			{
				resp_protocol->assign(file_name, start_pos, size);
				resp_protocol->assign(data);
				send_protocol(socket_handle, resp_protocol);
			}
		}
		else
		{
			SLOG_ERROR("can't open file=%s", file_name.c_str());
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return 0;
}

int DownloadServer::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadServer::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadServer::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int DownloadServer::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}


///////////////////////////////  thread pool  //////////////////////////////////
Thread<SocketHandle>* DownloadThreadPool::create_thread()
{
	DownloadServer* temp = new DownloadServer();
	temp->set_idle_timeout(30000);
	return (Thread<SocketHandle>*)temp;
}
