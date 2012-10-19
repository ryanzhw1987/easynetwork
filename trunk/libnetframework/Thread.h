/*
 * Thread.h
 *
 *  Created on: 2012-9-9
 *      Author: LiuYongjin
 */

#ifndef _LIB_THREAD_H_
#define _LIB_THREAD_H_

#include <pthread.h>
#include "Queue.h"
#include "slog.h"

//最小线程栈
#define MIN_STACK_SIZE 2*1024*1204

////////////////////////////////////////////////////////
/////              Thread线程模板                  /////
/////          T表示线程需要处理到任务             /////
////////////////////////////////////////////////////////
template <class T>
class Thread
{
static void* thread_proc(void* user_data);
public:
	Thread(bool detachable=true, unsigned int stack_size=0, int id=0)
			:m_detachable(detachable)
			,m_stack_size(stack_size)
			,m_running(false)
			,m_id(id)
			,m_thread_id(0)
			,m_task_queue(true){}
	virtual ~Thread(){}
	bool set_detatchable(bool detachable){return m_running?false:(m_detachable=detachable,true);}
	bool set_stack_size(unsigned int stack_size){return m_running?false:(m_stack_size=stack_size,true);}
	bool set_id(unsigned int id){return m_running?false:(m_id=id,true);}
	unsigned int get_id(){return m_id;}
	void wait_terminate(){if(m_running && !m_detachable)pthread_join(m_thread_id, NULL);}
	//启动线程
	bool start();
	//添加任务
	bool add_task(T &task);
	//获取任务
	bool get_task(T &task){return m_task_queue.pop(task);}
	//获取待处理任务数
	unsigned int get_task_count(){return m_task_queue.count();}
private:
	bool m_detachable;
	unsigned int m_stack_size;
	bool m_running;
	unsigned int m_id;
	pthread_t m_thread_id;
	Queue<T> m_task_queue; //线程安全队列
protected:
	//线程实际运行的入口
	virtual void run_thread()=0;
	//发送添加任务事件
	virtual bool notify_add_task()=0;
	//响应添加任务事件
	virtual bool on_notify_add_task()=0;
};

template <class T>
void* Thread<T>::thread_proc(void* user_data)
{
	((Thread<T>*)user_data)->run_thread();
	((Thread<T>*)user_data)->m_running = false;
	return NULL;
}

template <class T>
bool Thread<T>::start()
{
	if(m_running)
		return true;
	m_running = true;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if(m_stack_size > 0)// set stack size
	{
		if(m_stack_size < MIN_STACK_SIZE)
		{
			SLOG_WARN("stack size of thread too small. set to min stack size: %d", MIN_STACK_SIZE);
			m_stack_size = MIN_STACK_SIZE;
		}
		pthread_attr_setstacksize(&attr, m_stack_size);
	}
	if(m_detachable)
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(pthread_create(&m_thread_id, &attr, thread_proc, (void*)this) != 0)
		m_running = false;
	pthread_attr_destroy(&attr);
	return m_running;
}

template <class T>
bool Thread<T>::add_task(T &task)
{
	SLOG_DEBUG("Thread[ID=%d, Addr=%x] add task", get_id(), this);
	if(m_task_queue.push(task) == false)
		return false;
	if(notify_add_task() == false)
		SLOG_WARN("notify add task failed.");
	return true;
}

#endif //_LIB_THREAD_H_

