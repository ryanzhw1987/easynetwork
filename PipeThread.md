## PipeThread ##

PipeThread管道线程类用来创建ConnectThread。其用管道实现了通知添加线程任务事件：<br>1. 注册管道的读事件;<br>2. 当往线程添加任务后，notify_add_task往管道写入数据触发事件；<br>3.注册的PipeHandler的on_readable方法则调用on_notify_add_task方法通知线程处理队列的任务；<br>
<br>
<pre><code>template &lt;class T&gt;<br>
class PipeThread: public Thread&lt;T&gt;<br>
{<br>
public:<br>
	PipeThread(bool detachable=true, unsigned int stack_size=0, int id=0);<br>
	virtual ~PipeThread();<br>
private:<br>
	int m_pipe[2];<br>
	void close_pipe();<br>
	bool m_register_handler;	//是否需要注册管道事件<br>
<br>
	/////////////////////////////////////////////////////////////////<br>
	/////////////////////////////////////////////////////////////////<br>
	/////////                  PipeHandler                  /////////<br>
	/////////  响应PipeThread的管道读事件，调用PipeThread   /////////<br>
	////////   的on_notify_add_task接口通知线程处理任务     /////////<br>
	/////////////////////////////////////////////////////////////////<br>
	/////////////////////////////////////////////////////////////////<br>
	friend class PipeHandler;<br>
	class PipeHandler:public EventHandler<br>
	{<br>
	public:<br>
		PipeHandler(PipeThread&lt;T&gt; *thread):m_thread(thread){}<br>
		HANDLE_RESULT on_readable(int fd) //实现EventHandler的接口<br>
		{<br>
			SLOG_DEBUG("Thread[ID=%d, Addr=%x] pipe fd=%d readable", m_thread-&gt;get_thread_id(), m_thread, fd);<br>
			//接收消息,把管道到数据全部读取出来,很快,一般只循环一次;<br>
			//如果链接发得太快,导致很多消息堵塞呢?...<br>
			char buf[100];<br>
			while(read(fd, buf, 100) &gt; 0)<br>
				;<br>
			m_thread-&gt;on_notify_add_task();<br>
			return HANDLE_OK;<br>
		}<br>
	private:<br>
		PipeThread&lt;T&gt; *m_thread;<br>
	};<br>
	PipeHandler m_pipe_handler;<br>
<br>
////////////////// Thread线程接口 ////////////////<br>
protected:<br>
	//实现Thread接口:发送添加任务事件<br>
	bool notify_add_task();<br>
<br>
	////线程实际运行的入口<br>
	//virtual void run_thread()=0;<br>
	////响应添加任务事件<br>
	//virtual bool on_notify_add_task()=0;<br>
	//新增加的接口:注册管道事件,当写入管道时,通知线程调用on_notify_add_task来响应事件<br>
	virtual bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler)=0;<br>
};<br>
<br>
template &lt;class T&gt;<br>
PipeThread&lt;T&gt;::PipeThread(bool detachable, unsigned int stack_size, int id)<br>
								:Thread&lt;T&gt;(detachable, stack_size, id)<br>
								 ,m_pipe_handler(this)<br>
								 ,m_register_handler(true)<br>
{<br>
	int flags;<br>
	if (pipe(m_pipe))<br>
	{<br>
		SLOG_ERROR("create pipe errer when creating connect thread");<br>
		return ;<br>
	}<br>
	if((flags=fcntl(m_pipe[0], F_GETFL, 0)) == -1)<br>
	{<br>
		close_pipe();<br>
		SLOG_ERROR("fcntl pipe errer when register_notify_handler");<br>
		return ;<br>
	}<br>
	if(fcntl(m_pipe[0], F_SETFL, flags|O_NONBLOCK) == -1)<br>
	{<br>
		close_pipe();<br>
		SLOG_ERROR("fcntl pipe no block failed. errno=%d", errno);<br>
		return ;<br>
	}<br>
}<br>
<br>
template &lt;class T&gt;<br>
PipeThread&lt;T&gt;::~PipeThread()<br>
{<br>
	close_pipe();<br>
}<br>
<br>
template &lt;class T&gt;<br>
bool PipeThread&lt;T&gt;::notify_add_task()<br>
{<br>
	if(m_register_handler)	//先注册管道可读事件<br>
	{<br>
		if(register_notify_handler(m_pipe[0], EVENT_READ|EVENT_PERSIST, &amp;m_pipe_handler))<br>
			m_register_handler = false;<br>
		else<br>
			return false;<br>
	}<br>
<br>
	//往管道写数据,通知connect thread<br>
	if(write(m_pipe[1], "", 1) != 1)<br>
		SLOG_WARN("notify connect thread to accept a new connect failed.");<br>
	return true;<br>
}<br>
<br>
template &lt;class T&gt;<br>
void PipeThread&lt;T&gt;::close_pipe()<br>
{<br>
	close(m_pipe[0]);<br>
	close(m_pipe[1]);<br>
	m_pipe[0] = m_pipe[1] = -1;<br>
}<br>
</code></pre>