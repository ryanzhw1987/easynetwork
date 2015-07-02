## Queue ##

> Queue类是对进行队列封装的一个模板类。通过参数可以实现线程安全队列。

```
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
```