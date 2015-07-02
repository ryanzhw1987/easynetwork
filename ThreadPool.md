## ThreadPool ##

> 线程池的封装。当调用add\_task方法向线程池添加任务时，线程池按轮询的策略选择一个线程，并将该任务添加到线程中。线程在添加一个新任务时会收到通知，以便从任务队列中取出任务进行处理。
```
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
```