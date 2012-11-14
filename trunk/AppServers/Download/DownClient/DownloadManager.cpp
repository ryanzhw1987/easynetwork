/*
 * DownloadManager.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadManager.h"
#include "IODemuxerEpoll.h"
#include "DownloadProtocol.h"

#include <sstream>
#include <iostream>
using namespace::std;

void DownloadThread::run_thread()
{
	SLOG_INFO("ConnectThread[ID=%d] is running...", get_thread_id());
	start_server();
	SLOG_INFO("ConnectThread end...");
}

bool DownloadThread::on_notify_add_task()
{
	SLOG_INFO("Thread[ID=%d,Addr=%x] on_notify_add_task", get_thread_id(), this);
	return send_download_task();
}

bool DownloadThread::register_notify_handler(int read_pipe, EVENT_TYPE event_type, EventHandler* event_handler)
{
	IODemuxer* io_demuxer = get_io_demuxer();
	return io_demuxer->register_event(read_pipe,event_type,-1,event_handler)==0?true:false;
}

/////////////////////////////////// NetInterface 方法 ////////////////////////////
bool DownloadThread::start_server()
{
	//Init NetInterface
	init_net_interface();
	set_thread_ready();

	//// Add Your Codes Here
	////////////////////////

	SLOG_INFO("Start download server.");
	get_io_demuxer()->run_loop();

	return true;
}

ProtocolFamily* DownloadThread::create_protocol_family()
{
	return new DownloadProtocolFamily;
}

void DownloadThread::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

bool DownloadThread::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_RESPOND_DATA:
	{
		RespondData* temp_protocol = (RespondData*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long start_pos = temp_protocol->get_start_pos();
		unsigned int size = temp_protocol->get_size();
		const char *data = temp_protocol->get_data();

		ostringstream temp;
		temp<<file_name<<"_"<<start_pos;
		DownloadMap::iterator it = m_downloading_task.find(temp.str());
		if(it == m_downloading_task.end())
		{
			SLOG_WARN("receive RespondData[file=%s, start=%ld], but can't not find task", file_name.c_str(), start_pos);
		}
		else
		{
			DownloadTask* task = it->second;
			SLOG_INFO("receive RespondData[ID=%d, fd=%d, file=%s, index=%d, start_pos=%ld, size=%d]", get_thread_id(), socket_handle, file_name.c_str(), task->task_index, task->start_pos, task->size);
			if(task->fp == NULL)
			{
				char buf[128];
				sprintf(buf, "./download_data/%s.%d", task->file_name.c_str(), task->task_index);
				task->fp = fopen(buf, "wb");
			}

			fwrite(data, 1, size, task->fp);
			task->down_size += size;
			if(task->down_size == task->size)
			{
				SLOG_INFO("finish download[ID=%d, fd=%d, file=%s, index=%d]", get_thread_id(), socket_handle, file_name.c_str(), task->task_index);
				fclose(task->fp);
				delete task;
				m_downloading_task.erase(it);
				m_is_downloading = false;
				send_download_task(socket_handle);
			}
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return true;
}

bool DownloadThread::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool DownloadThread::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool DownloadThread::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool DownloadThread::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return true;
}

bool DownloadThread::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle accpet. fd=%d", socket_handle);
	return true;
}

/////////////////////////////////////////   业务逻辑   ///////////////////////////////////////
bool DownloadThread::send_download_task(SocketHandle socket_handle)
{
	if(m_is_downloading)  //一次只执行一个下载任务
		return false;

	DownloadTask* download_task = NULL;
	if(get_task(download_task) && send_download_task(socket_handle, download_task))
		m_is_downloading = true;
	return true;
}

////////////////////////////////////////////////////////
bool DownloadThread::send_download_task(SocketHandle socket_handle, DownloadTask* download_task)
{
	if(socket_handle == SOCKET_INVALID)
		socket_handle = get_active_trans_socket("127.0.0.1", 3011);
	if(socket_handle == SOCKET_INVALID)
		return false;
	DownloadProtocolFamily *protocol_family = (DownloadProtocolFamily*)get_protocol_family();
	RequestData *temp_protocol = (RequestData *)protocol_family->create_protocol(PROTOCOL_REQUEST_DATA);
	if(temp_protocol == NULL)
		return false;
	temp_protocol->assign(download_task->file_name, download_task->start_pos, download_task->size);
	if(send_protocol(socket_handle, temp_protocol)) //发送请求文件大小的协议
	{
		SLOG_DEBUG("Thread[ID=%d,Addr=%x] send download task[file=%s, start_pos=%ld, size=%d, index=%d]"
					,get_thread_id()
					,this
					,download_task->file_name.c_str()
					,download_task->start_pos
					,download_task->size
					,download_task->task_index);
		ostringstream temp;
		temp<<download_task->file_name<<"_"<<download_task->start_pos;
		m_downloading_task.insert(make_pair(temp.str(), download_task));
		return true;
	}
	else
		return false;
}

///////////////////////////////  thread pool  //////////////////////////////////
Thread<DownloadTask*>* DownloadWorkerPool::create_thread()
{
	DownloadThread* temp = new DownloadThread;
	temp->set_idle_timeout(30000);
	return (Thread<DownloadTask*>*)temp;
}
