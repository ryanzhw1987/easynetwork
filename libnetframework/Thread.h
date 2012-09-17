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

template <class T>
class Thread
{
static void* thread_proc(void* user_data);
public:
	Thread(bool detachable=true, unsigned int stack_size=0, int id=0)
		:m_detachable(detachable)
		,m_stack_size(stack_size)
		,m_running(false)
		,m_thread_id(0)
		,m_my_id(id)
		,m_task_queue(true){}
	virtual ~Thread(){}

	bool set_detatchable(bool detachable)
	{
		if(m_running)
			return false;
		m_detachable = detachable;
		return true;
	}
	bool set_stack_size(unsigned int stack_size)
	{
		if(m_running)
			return false;
		m_stack_size = stack_size;
		return true;
	}
	void wait_terminate()
	{
		if(!m_detachable)
			pthread_join(m_thread_id, NULL);
	}

	int get_id(){return m_my_id;}
	void set_id(int id){m_my_id=id;}

	bool start();
	bool add_task(T &task);		//添加任务
	virtual bool do_task()=0; 	//处理任务
private:
	bool m_running;
	bool m_detachable;
	unsigned int m_stack_size;
	int m_my_id;
	pthread_t m_thread_id;
	Queue<T> m_task_queue;
protected:
	Queue<T>* get_task_queue(){return &m_task_queue;}
	virtual bool notify_add_task()=0;
	virtual void run()=0;
};

#define MIN_STACK_SIZE 2*1024*1204
template <class T>
void* Thread<T>::thread_proc(void* user_data)
{
	Thread<T>* thread = (Thread<T>*)user_data;
	thread->run();
	thread->m_running = false;
	return NULL;
}

template <class T>
bool Thread<T>::start()
{
	if(m_running)
		return true;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// set stack size
	if(m_stack_size > 0)
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
		return false;
	pthread_attr_destroy(&attr);

	m_running = true;
	return true;
}

template <class T>
bool Thread<T>::add_task(T &task)
{
	SLOG_DEBUG("Thread[ID=%d, Addr=%x] add task", get_id(), this);
	if(m_task_queue.push(task) == false)
		return false;
	if(notify_add_task() == false)
		SLOG_WARN("notify recv new task failed.");
	return true;
}

#endif //_LIB_THREAD_H_



