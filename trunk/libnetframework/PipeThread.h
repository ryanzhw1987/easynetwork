/*
 * PipeThread.h
 *
 *  Created on: 2012-9-17
 *      Author: LiuYongjin
 */

#ifndef _LIB_PIPE_THREAD_H_
#define _LIB_PIPE_THREAD_H_

#include "Thread.h"
#include "IODemuxer.h"
#include "EventHandler.h"
#include "slog.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

////////////////////////////////////////////////////////////
/////                 PipeThread                       /////
/////             通过管道触发消息事件                 /////
////////////////////////////////////////////////////////////
template <class T>
class PipeThread: public Thread<T>
{
public:
	PipeThread(bool detachable=true, unsigned int stack_size=0, int id=0);
	virtual ~PipeThread();
private:
	int m_pipe[2];
	void close_pipe();
	bool m_register_handler;	//是否需要注册管道事件

	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////                  PipeHandler                  /////////
	/////////  响应PipeThread的管道读事件，调用PipeThread   /////////
	////////   的on_notify_add_task接口通知线程处理任务     /////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	friend class PipeHandler;
	class PipeHandler:public EventHandler
	{
	public:
		PipeHandler(PipeThread<T> *thread):m_thread(thread){}
		HANDLE_RESULT on_readable(int fd) //实现EventHandler的接口
		{
			SLOG_DEBUG("Thread[ID=%d, Addr=%x] pipe fd=%d readable", m_thread->get_thread_id(), m_thread, fd);
			//接收消息,把管道到数据全部读取出来,很快,一般只循环一次;
			//如果链接发得太快,导致很多消息堵塞呢?...
			char buf[100];
			while(read(fd, buf, 100) > 0)
				;
			m_thread->on_notify_add_task();
			return HANDLE_OK;
		}
	private:
		PipeThread<T> *m_thread;
	};
	PipeHandler m_pipe_handler;

////////////////// Thread线程接口 ////////////////
protected:
	//实现Thread接口:发送添加任务事件
	bool notify_add_task();

	////线程实际运行的入口
	//virtual void run_thread()=0;
	////响应添加任务事件
	//virtual bool on_notify_add_task()=0;
	//新增加的接口:注册管道事件,当写入管道时,通知线程调用on_notify_add_task来响应事件
	virtual bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler)=0;
};

template <class T>
PipeThread<T>::PipeThread(bool detachable, unsigned int stack_size, int id)
								:Thread<T>(detachable, stack_size, id)
								 ,m_pipe_handler(this)
								 ,m_register_handler(true)
{
	int flags;
	if (pipe(m_pipe))
	{
		SLOG_ERROR("create pipe errer when creating connect thread");
		return ;
	}
	if((flags=fcntl(m_pipe[0], F_GETFL, 0)) == -1)
	{
		close_pipe();
		SLOG_ERROR("fcntl pipe errer when register_notify_handler");
		return ;
	}
	if(fcntl(m_pipe[0], F_SETFL, flags|O_NONBLOCK) == -1)
	{
		close_pipe();
		SLOG_ERROR("fcntl pipe no block failed. errno=%d", errno);
		return ;
	}
}

template <class T>
PipeThread<T>::~PipeThread()
{
	close_pipe();
}

template <class T>
bool PipeThread<T>::notify_add_task()
{
	if(m_register_handler)	//先注册管道可读事件
	{
		if(register_notify_handler(m_pipe[0], EVENT_READ|EVENT_PERSIST, &m_pipe_handler))
			m_register_handler = false;
		else
			return false;
	}

	//往管道写数据,通知connect thread
	if(write(m_pipe[1], "", 1) != 1)
		SLOG_WARN("notify connect thread to accept a new connect failed.");
	return true;
}

template <class T>
void PipeThread<T>::close_pipe()
{
	close(m_pipe[0]);
	close(m_pipe[1]);
	m_pipe[0] = m_pipe[1] = -1;
}

#endif //_LIB_PIPE_THREAD_H_



