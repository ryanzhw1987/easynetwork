/*
 * ThreadPool.cpp
 *
 *  Created on: 2012-9-9
 *      Author: tim
 */
#include "ThreadPool.h"
#include "slog.h"

ThreadPool::ThreadPool(unsigned int thread_num)
{
	m_thread_num = thread_num;
	m_thread_array = NULL;
	m_inited = false;

	if(m_thread_num > 0)
		m_thread_array = new Thread*[m_thread_num];
	memset(m_thread_array, 0, m_thread_num*sizeof(Thread*));
}

ThreadPool::~ThreadPool()
{
	if(m_thread_array != NULL)
		delete[] m_thread_array;
	m_thread_num = 0;
	m_thread_array = NULL;
}

bool ThreadPool::start()
{
	if(m_inited)
		return true;

	int i;
	for(i=0; i<m_thread_num; ++i)
	{
		m_thread_array[i] = create_thread();
		if(m_thread_array[i] == NULL)
			SLOG_ERROR("create thread %d failed", i);
	}
	for(i=0; i<m_thread_num; ++i)
	{
		if(m_thread_array[i] != NULL)
			m_thread_array[i]->start();
	}

	return true;
}




