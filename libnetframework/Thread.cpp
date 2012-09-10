/*
 * Thread.cpp
 *
 *  Created on: 2012-9-9
 *      Author: LiuYongjin
 */
#include "Thread.h"
#include "slog.h"

#define MIN_STACK_SIZE 2*1024*1204
void* Thread::thread_proc(void* user_data)
{
	Thread* thread = (Thread*)user_data;
	thread->run();
	thread->m_running = false;
	return NULL;
}

bool Thread::start()
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




