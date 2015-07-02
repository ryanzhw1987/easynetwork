## Thread ##

> Thread类是对线程的封装。add\_task方法用于向线程添加一个任务，同时该方法会调用notify\_add\_task接口方法通知线程有新的任务添加到队列中。线程需要实现notify\_add\_task和on\_notify\_add\_task接口方法来实现通知和响应新任务的添加。

```
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
	bool set_thread_id(unsigned int id){return m_running?false:(m_id=id,true);}
	unsigned int get_thread_id(){return m_id;}
	void wait_terminate(){if(m_running && !m_detachable)pthread_join(m_thread_id, NULL);}
	//启动线程
	bool start();
	//添加任务
	bool add_task(T &task);
	//获取任务
	bool get_task(T &task){return m_task_queue.pop(task);}
	//获取待处理任务数
	unsigned int get_task_count(){return m_task_queue.count();}

	//线程是否已经启动
	bool is_thread_ready(){return m_running;}
	//设置线程已经准备好
	void set_thread_ready(){m_running = true;}

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
```