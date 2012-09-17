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

template<class T>
class PipeHandler:public EventHandler
{
public: //实现EventHandler的接口
	virtual HANDLE_RESULT on_readable(int fd)
	{
		//接收消息,把管道到数据全部读取出来
		//很快,一般只循环一次;链接发得太快,导致很多消息堵塞...但是没有关系
		char buf[100];
		while(read(fd, buf, 100) > 0)
			;
		SLOG_DEBUG("Thread[ID=%d, Addr=%x] pipe fd=%d reable able", m_thread->get_id(), m_thread, fd);
		m_thread->do_task(); //处理任务
		return HANDLE_OK;
	}
public:
	PipeHandler(Thread<T> *thread):m_thread(thread){}
private:
	Thread<T> *m_thread;
};

template <class T>
class PipeThread: public Thread<T>
{
public:
	PipeThread(IODemuxer *io_demuxer, bool detachable=true, unsigned int stack_size=0, int id=0);
protected://实现Thread的接口
	bool notify_add_task();
private:
	int m_pipe[2];
	PipeHandler<T> m_pipe_handler;
};

template <class T>
PipeThread<T>::PipeThread(IODemuxer *io_demuxer, bool detachable, unsigned int stack_size, int id)
					:Thread<T>(detachable, stack_size, id)
					 ,m_pipe_handler(this)
{
	if (pipe(m_pipe))
	{
		SLOG_ERROR("create pipe errer when creating connect thread");
		assert(0);
	}

	int flags = fcntl(m_pipe[0], F_GETFL, 0);
	if(flags != -1 )
	{
		flags |= O_NONBLOCK;
		if(fcntl(m_pipe[0], F_SETFL, flags) == -1)
			SLOG_ERROR("set pipe[0] no block failed. errno=%d", errno);
	}

	//注册管道读事件
	if(io_demuxer->register_event(m_pipe[0], EVENT_READ|EVENT_PERSIST, -1, &m_pipe_handler) == -1)
		SLOG_ERROR("register pipe=%d read event failed",m_pipe[0]);
}

template <class T>
bool PipeThread<T>::notify_add_task()
{
	//往管道写数据,通知connect thread
	if(write(m_pipe[1], "", 1) != 1)
		SLOG_WARN("notify connect thread to accept a new connect failed.");
	return true;
}

#endif //_LIB_PIPE_THREAD_H_



