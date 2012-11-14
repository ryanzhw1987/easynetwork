/*
 * WorkThread.h
 *
 *  Created on: 2012-11-14
 *      Author: LiuYongJin
 */

#ifndef _LIB_WORK_THREAD_H_20121114
#define _LIB_WORK_THREAD_H_20121114

#include "Thread.h"
#include "slog.h"

#include <pthread.h>

/////////////////////////////////////////////////////
////                  WorkThread                 ////
////     工作者线程不断从队列里面获取并处        ////
////     理任务,当任务队列为空时阻塞直到又       ////
////     有任务添加到队列                        ////
/////////////////////////////////////////////////////
template <class T>
class WorkThread: public Thread<T>
{
protected:
	////实现Thread接口:发送添加任务事件
	bool notify_add_task();
	////实现Thread接口:响应添加任务事件
	bool on_notify_add_task();
	////实现Thread接口:线程实际运行的入口
	void run_thread();
protected:
	////线程处理任务接口
	void handle_task(T &task)=0;
public:
	WorkThread();
	~WorkThread();
private:
	pthread_cond_t m_cond;
	pthread_mutex_t m_cond_mutex;
};

////////////////////////////////////////////////
template <class T>
WorkThread<T>::WorkThread()
{
	pthread_cond_init(&m_cond, NULL);
	pthread_mutex_init(&m_cond_mutex, NULL);
}

template <class T>
WorkThread<T>::~WorkThread()
{
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_cond_mutex);
}

template <class T>
bool WorkThread<T>::notify_add_task()
{
	pthread_mutex_lock(&m_cond_mutex);
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_cond_mutex);
	return true;
}

template <class T>
bool WorkThread<T>::on_notify_add_task()
{
	//等待任务队列非空
	pthread_mutex_lock(&m_cond_mutex);
	pthread_cond_wait(&m_cond, &m_cond_mutex);
	pthread_mutex_unlock(&m_cond_mutex);
	return true;
}

template <class T>
void WorkThread<T>::run_thread()
{
	while(true)
	{
		on_notify_add_task();
		T task;
		while(get_task(task))
			handle_task(task);
	}
}

#endif //_LIB_WORK_THREAD_H_20121114


