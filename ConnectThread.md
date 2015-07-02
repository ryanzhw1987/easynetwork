## ConnectThread ##
ConnectThread类继承NetInterface类和PipeThread类，主要实现类PipeThread的接口方法：
```
class ConnectThread:public NetInterface, public PipeThread<SocketHandle>
{
public:
	ConnectThread(bool detachable=true, unsigned int stack_size=0, int id=0):PipeThread<SocketHandle>(detachable, stack_size, id){}
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现PipeThread接口:注册通知事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);
```

<br>

<b>1. register_notify_handler方法</b>

该方法用来向IODemuxer注册管道事件；<br>
<pre><code>bool ConnectThread::register_notify_handler(int read_pipe, EVENT_TYPE event_type, EventHandler* event_handler)<br>
{<br>
	IODemuxer* io_demuxer = get_io_demuxer();<br>
	return io_demuxer-&gt;register_event(read_pipe,event_type,-1,event_handler)==0?true:false;<br>
}<br>
</code></pre>

<b>2. on_notify_add_task方法</b>

该方法响应添加一个链接任务：<br>
<pre><code>bool ConnectThread::on_notify_add_task()<br>
{<br>
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task", get_thread_id(), this);<br>
	SocketHandle trans_fd;<br>
	while(get_task(trans_fd))<br>
	{<br>
		SLOG_DEBUG("thread accept trans fd=%d", trans_fd);<br>
		if(NetInterface::accept(trans_fd) == false)<br>
			SLOG_ERROR("connect thread accept fd=%d failed", trans_fd);<br>
	}<br>
	return true;<br>
}<br>
</code></pre>
<blockquote>该方法通过调用NetInterface的accept将接收到的新链接注册到IODemuxer中进行监听。之后所有该链接的事件都由ConnectThread来处理。</blockquote>

<br>
<b>3. run_thread方法</b>
该方法真正执行线程的运行，我们记得NetInterface使用前需要显示调用start_server来完成初始化工作，因此在run_thread中完成这件事情，而线程要做的具体事情由start_server的实现来完成：<br>
<pre><code>void ConnectThread::run_thread()<br>
{<br>
	SLOG_INFO("ConnectThread[ID=%d] is running...", get_thread_id());<br>
	//Start App Server(NetInterface)<br>
	start_server();<br>
<br>
	SLOG_INFO("ConnectThread end...");<br>
}<br>
</code></pre>