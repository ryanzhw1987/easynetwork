/*
 * Thread.h
 *
 *  Created on: 2012-9-9
 *      Author: LiuYongjin
 */

#ifndef _LIB_THREAD_H_
#define _LIB_THREAD_H_

#include <pthread.h>

class Thread
{
public:
	Thread(bool detachable=true, unsigned int stack_size=0, int id=0)
		:m_detachable(detachable)
		,m_stack_size(stack_size)
		,m_running(false)
		,m_thread_id(0)
		,m_my_id(id){}

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

	bool start();
	void wait_terminate()
	{
		if(!m_detachable)
			pthread_join(m_thread_id, NULL);
	}

	int get_id(){return m_my_id;}
	void set_id(int id){m_my_id=id;}
private:
	bool m_running;
	bool m_detachable;
	unsigned int m_stack_size;
	pthread_t m_thread_id;
	int m_my_id;
	static void* thread_proc(void* user_data);
protected:
	virtual void run()=0;
};

#endif //_LIB_THREAD_H_



