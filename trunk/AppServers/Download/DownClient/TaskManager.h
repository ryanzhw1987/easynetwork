/*
 * TaskManager.h
 *
 *  Created on: 2012-9-17
 *      Author: LiuYongjin
 */

#ifndef APP_DOWNLOAD_TASK_MANAGER_H_
#define APP_DOWNLOAD_TASK_MANAGER_H_

#include <string>
using std::string;

#include "PipeThread.h"
#include "NetInterface.h"
#include "DownloadManager.h"
#include "DownloadProtocol.h"

class TaskManager: public NetInterface, public PipeThread<string>
{
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现接口:注册管道事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);
protected:
	//////////////////由应用层重写 创建IODemuxer//////////////////
	virtual IODemuxer* create_io_demuxer();
	//////////////////由应用层重写 销毁IODemuxer//////////////////
	virtual void delete_io_demuxer(IODemuxer* io_demuxer);
	//////////////////由应用层重写 创建SocketManager//////////////
	virtual SocketManager* create_socket_manager();
	//////////////////由应用层重写 销毁SocketManager//////////////
	virtual void delete_socket_manager(SocketManager* socket_manager);
	//////////////////由应用层重写 创建具体的协议族//////////////
	virtual ProtocolFamily* create_protocol_family();
	//////////////////由应用层重写 销毁协议族////////////////////
	virtual void delete_protocol_family(ProtocolFamily* protocol_family);

	//////////////////由应用层重写 接收协议函数//////////////////
	bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	bool on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	bool on_socket_handle_timeout(SocketHandle socket_handle);

////////////////////////////////////////////////
public:
	TaskManager():m_download_pool(NULL){}

	void set_download_pool(DownloadThreadPool* download_pool)
	{
		m_download_pool = download_pool;
	}
protected:
	bool send_get_filesize_task(string &file_name);
	bool download_task(const string &file_name, unsigned long long file_size);
private:
	DownloadThreadPool *m_download_pool;

};

#endif //APP_DOWNLOAD_TASK_MANAGER_H_

