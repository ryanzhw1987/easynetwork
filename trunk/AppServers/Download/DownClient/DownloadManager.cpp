/*
 * DownloadManager.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadManager.h"
#include "DownloadProtocol.h"
#include "IODemuxerEpoll.h"

#include <sstream>
#include <iostream>
using namespace::std;

///////////////////////////////  thread pool  //////////////////////////////////
Thread<DownloadTask*>* DownloadThreadPool::create_thread()
{
	EpollDemuxer *io_demuxer = new EpollDemuxer;
	DownloadProtocolFamily *protocol_family = new DownloadProtocolFamily;
	SocketManager *socket_manager = new SocketManager;
	DownloadThread* temp = new DownloadThread(io_demuxer, protocol_family, socket_manager);
	temp->set_idle_timeout(30000);
	return (Thread<DownloadTask*>*)temp;
}

bool DownloadThread::do_task()
{
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task",get_id(), this);
	Queue<DownloadTask*> task_queue(false);
	get_task_queue()->transform(&task_queue, false);

	DownloadTask* download_task=NULL;
	while(task_queue.pop(download_task))
	{
		SLOG_DEBUG("Thread[ID=%d,Addr=%x] download task[file=%s, start_pos=%ld, size=%d, index=%d]"
				,get_id()
				,this
				,download_task->file_name.c_str()
				,download_task->start_pos
				,download_task->size
				,download_task->task_index);
		if(send_download_task(download_task) == false)
		{
			SLOG_ERROR("send download task[file=%s, start_pos=%ld, size=%d, index=%d] failed."
					,download_task->file_name.c_str()
					,download_task->start_pos
					,download_task->size
					,download_task->task_index);
			delete download_task;
		}
		else
		{
			ostringstream temp;
			temp<<download_task->file_name<<"_"<<download_task->start_pos;
			m_downloading_task.insert(make_pair(temp.str(), download_task));
		}
	}

	return true;
}

void DownloadThread::run()
{
	SLOG_INFO("MTServerAppFramework[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("MTServerAppFramework end...");
}

////////////////// NetInterface的接口 由应用层重写 接收协议函数//////////////////
int DownloadThread::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_RESPOND_DATA:
	{
		RespondData* temp_protocol = (RespondData*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long start_pos = temp_protocol->get_start_pos();
		unsigned int size = temp_protocol->get_size();
		string data = temp_protocol->get_data();

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
			SLOG_INFO("receive RespondData[fd=%d, file=%s, index=%d, start_pos=%ld, size=%d]", socket_handle, file_name.c_str(), task->task_index, start_pos, size);
			if(task->fp == NULL)
			{
				char buf[128];
				sprintf(buf, "%s.%d", task->file_name.c_str(), task->task_index);
				task->fp = fopen(buf, "wb");
			}

			fwrite(data.c_str(), 1, data.size(), task->fp);
			task->down_size += data.size();
			if(task->down_size == task->size)
			{
				SLOG_INFO("finish download[fd=%d, file=%s, index=%d]", socket_handle, file_name.c_str(), task->task_index);
				fclose(task->fp);
				delete task;
				m_downloading_task.erase(it);
			}
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return 0;
}

int DownloadThread::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadThread::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadThread::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int DownloadThread::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}

////////////////////////////////////////////////////////
bool DownloadThread::send_download_task(DownloadTask* down_task)
{
	SocketHandle socket_handle = get_active_trans_socket("127.0.0.1", 3011);
	if(socket_handle == SOCKET_INVALID)
		return false;
	DownloadProtocolFamily *protocol_family = (DownloadProtocolFamily*)get_protocol_family();
	RequestData *temp_protocol = (RequestData *)protocol_family->create_protocol(PROTOCOL_REQUEST_DATA);
	if(temp_protocol == NULL)
		return false;
	temp_protocol->assign(down_task->file_name, down_task->start_pos, down_task->size);
	if(send_protocol(socket_handle, temp_protocol)==0) //发送请求文件大小的协议
		return true;
	else
		return false;
}
