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

class ThreadPool
{
public:
	ThreadPool(unsigned int thread_num);
	virtual ~ThreadPool();

	bool start();
protected:
	//创建一个线程
	virtual Thread* create_thread()=0;
	unsigned int get_thread_num(){return m_thread_num;}
	Thread* get_thread(unsigned int thread_index){return thread_index<m_thread_num?m_thread_array[thread_index]:NULL;}
private:
	bool m_inited;
	Thread **m_thread_array;
	unsigned int m_thread_num;

};

#endif //_LIB_THREAD_POOL_H_



