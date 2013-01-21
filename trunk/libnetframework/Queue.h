/*
 * Queue.h
 *
 *  Created on: 2012-9-10
 *      Author: LiuYongjin
 */

#ifndef _LIB_QUEUE_H_
#define _LIB_QUEUE_H_

#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "MemManager.h"

template <class T>
class DataNode
{
public:
	DataNode():m_next(NULL), m_pre(NULL){}
	DataNode *m_next;
	DataNode *m_pre;
	T m_value;
};

template <class T>
class Queue
{
public:
	Queue(bool thread_safe=false):m_thread_safe(thread_safe), m_count(0)
	{
		if(m_thread_safe)
			pthread_mutex_init(&m_mutex, NULL);
	}
	~Queue();

	bool is_thread_safe(){return m_thread_safe;}
	unsigned int count(){return m_count;}

	bool push(T& value);
	bool pop(T& value);

	//转换成另外一个队列.数据同时迁移到该新的队列.
	bool transform(Queue<T> *new_queue, bool thread_safe);
private:
	bool m_thread_safe;		//是否线程安全
	unsigned int m_count;	//元素个数
	DataNode<T> m_header;
	MemCache<DataNode<T> > m_data_node_memcache;

	pthread_mutex_t m_mutex;
	void lock()
	{
		if(m_thread_safe)
			pthread_mutex_lock(&m_mutex);
	}
	void unlock()
	{
		if(m_thread_safe)
			pthread_mutex_unlock(&m_mutex);
	}
};

template <class T>
Queue<T>::~Queue()
{
	if(m_thread_safe)
		pthread_mutex_destroy(&m_mutex);
	DataNode<T> *data_node = NULL;
	for(data_node=m_header.m_next; data_node!=NULL; data_node=m_header.m_next)
	{
		m_header.m_next = data_node->m_next;
		m_data_node_memcache.Free(data_node);
	}
	m_header.m_next = m_header.m_pre = NULL;
}

template <class T>
bool Queue<T>::push(T& value)
{
	lock();
	DataNode<T>* data_node = m_data_node_memcache.Alloc();
	if(data_node == NULL)
	{
		unlock();
		return false;
	}
	data_node->m_value = value;
	data_node->m_pre = m_header.m_pre;
	if(m_header.m_pre != NULL)
		m_header.m_pre->m_next = data_node;
	else
		m_header.m_next = data_node;
	m_header.m_pre = data_node;
	++m_count;
	unlock();
	return true;
}

template <class T>
bool Queue<T>::pop(T &value)
{
	lock();
	if(m_count <= 0)
	{
		unlock();
		return false;
	}
	assert(m_header.m_next != NULL);
	DataNode<T>* data_node = m_header.m_next;
	m_header.m_next = data_node->m_next;
	if(m_header.m_next == NULL)
		m_header.m_pre = NULL;
	--m_count;
	value = data_node->m_value;
	m_data_node_memcache.Free(data_node);
	unlock();
	return true;
}

template <class T>
bool Queue<T>::transform(Queue<T> *new_queue, bool thread_safe)
{
	lock();
	new_queue->m_count = m_count;
	new_queue->m_header = m_header;

	m_header.m_next = m_header.m_pre = NULL;
	m_count = 0;

	unlock();
	return true;
}

#endif //_LIB_QUEUE_H_



