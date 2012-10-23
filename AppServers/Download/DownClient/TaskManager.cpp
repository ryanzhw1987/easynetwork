/*
 * TaskManager.cpp
 *
 *  Created on: 2012-9-17
 *      Author: LiuYongjin
 */

#include "TaskManager.h"
#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "DownloadProtocol.h"

void TaskManager::run_thread()
{
	SLOG_INFO("ConnectThread[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("ConnectThread end...");
}

bool TaskManager::on_notify_add_task()
{
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task",get_id(), this);
	string file_name;
	while(get_task(file_name))
	{
		SLOG_DEBUG("Thread[ID=%d, Addr=%x] receive task=%s", get_id(), this, file_name.c_str());
		if(send_get_filesize_task(file_name) == false)
			SLOG_ERROR("sent get_file_size protocol failed. file_name=%s", file_name.c_str());
	}

	return true;
}

bool TaskManager::register_notify_handler(int read_pipe, EVENT_TYPE event_type, EventHandler* event_handler)
{
	IODemuxer* io_demuxer = get_io_demuxer();
	return io_demuxer->register_event(read_pipe,event_type,-1,event_handler)==0?true:false;
}

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* TaskManager::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void TaskManager::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* TaskManager::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void TaskManager::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* TaskManager::create_protocol_family()
{
	return new DownloadProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void TaskManager::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

/////////////////////////////////////  实现 NetInterface的接口  ///////////////////////
bool TaskManager::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_RESPOND_SIZE:
	{
		RespondSize *temp_protocol = (RespondSize*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long file_size = temp_protocol->get_file_size();
		SLOG_INFO("receive <RespondSize:file=%s, size=%ld>", file_name.c_str(), file_size);

		//下载文件
		if(file_size > 0)
			download_task(file_name, file_size);
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return true;
}

bool TaskManager::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool TaskManager::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool TaskManager::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool TaskManager::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////
bool TaskManager::send_get_filesize_task(string &file_name)
{
	SocketHandle socket_handle = get_active_trans_socket("127.0.0.1", 3011);
	if(socket_handle == SOCKET_INVALID)
		return false;
	DownloadProtocolFamily *protocol_family = (DownloadProtocolFamily*)get_protocol_family();
	RequestSize *temp_protocol = (RequestSize *)protocol_family->create_protocol(PROTOCOL_REQUEST_SIZE);
	if(temp_protocol == NULL)
		return false;
	temp_protocol->assign(file_name);

	if(send_protocol(socket_handle, temp_protocol)==0) //发送请求文件大小的协议
		return true;
	else
		return false;
}

bool TaskManager::download_task(const string &file_name, unsigned long long file_size)
{
	int i;
	unsigned long long start_pos = 0;
	unsigned long long split_size = 1024*1024;

	for(i=0; start_pos+split_size<=file_size; ++i,start_pos+=split_size)
	{
		DownloadTask *task = new DownloadTask;
		task->file_name = file_name;
		task->start_pos = start_pos;
		task->size = split_size;
		task->task_index = i;
		task->fp = NULL;
		task->down_size = 0;
		m_download_pool->add_task(task);
	}

	if(start_pos < file_size) //最后一个分片
	{
		DownloadTask *task = new DownloadTask;
		task->file_name = file_name;
		task->start_pos = start_pos;
		task->size = file_size-start_pos;
		task->task_index = i;
		task->fp = NULL;
		task->down_size = 0;
		m_download_pool->add_task(task);
	}
	return true;
}
