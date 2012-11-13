/*
 * ThreadPool.h
 *
 *  Created on: 2012-9-9
 *      Author: LiuYongjin
 */
#ifndef _LIB_THREAD_POOL_H_
#define _LIB_THREAD_POOL_H_
#include <stdio.h>
#include <string.h>
#include "Thread.h"
#include "slog.h"

template <class T>
class ThreadPool
{
public:
	ThreadPool(unsigned int thread_num);
	virtual ~ThreadPool();

	bool start();
	bool add_task(T &task);
protected:
	//创建一个线程
	virtual Thread<T>* create_thread()=0;
	unsigned int get_thread_num(){return m_thread_num;}
	Thread<T>* get_thread(unsigned int thread_index){return thread_index<m_thread_num?m_thread_array[thread_index]:NULL;}

private:
	bool m_inited;
	Thread<T> **m_thread_array;
	unsigned int m_thread_num;
	unsigned int m_last_index;  //添加任务到其指定的线程
};

template <class T>
ThreadPool<T>::ThreadPool(unsigned int thread_num)
{
	m_thread_num = thread_num;
	m_thread_array = NULL;
	m_inited = false;
	m_last_index = 0;

	if(m_thread_num > 0)
		m_thread_array = new Thread<T>*[m_thread_num];
	memset(m_thread_array, 0, m_thread_num*sizeof(Thread<T>*));
}

template <class T>
ThreadPool<T>::~ThreadPool()
{
	if(m_thread_array != NULL)
		delete[] m_thread_array;
	m_thread_num = 0;
	m_thread_array = NULL;
}

template <class T>
bool ThreadPool<T>::start()
{
	if(m_inited)
		return true;

	int i;
	for(i=0; i<m_thread_num; ++i)
	{
		m_thread_array[i] = create_thread();
		if(m_thread_array[i] == NULL)
			SLOG_ERROR("create thread %d failed", i);
		m_thread_array[i]->set_thread_id(i);
	}
	for(i=0; i<m_thread_num; ++i)
	{
		if(m_thread_array[i] != NULL)
			m_thread_array[i]->start();
	}

	return true;
}

template <class T>
bool ThreadPool<T>::add_task(T &task)
{
	unsigned int thread_num = get_thread_num();
	if(thread_num <= 0)
		return false;
	Thread<T>* thread = get_thread(m_last_index);
	if(thread == NULL)
		return false;
	SLOG_DEBUG("ThreadPool dispatch new task to Thread[ID=%d,Addr=%x]", m_last_index, thread);
	m_last_index = (++m_last_index)%thread_num;
	return thread->add_task(task);
}

#endif //_LIB_THREAD_POOL_H_



