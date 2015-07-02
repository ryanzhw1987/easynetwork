## ListenHandler ##

> ListenHandler类用来响应服务端监听端口的读、写、超时、错误等事件。当监听端口可读时，IODemuxer会调用ListenHandler类的on\_readable方法，该方法accept一个新的链接，然后通过ConnectAccepter类来分派该新链接。通过设置不同的ConnectAccepter类可以实现单线程、多线程服务器框架。
```
//ListenHandler.h
class ListenHandler:public EventHandler
{
public:
	ListenHandler(ConnectAccepter *connect_accepter):m_connect_accepter(connect_accepter){}
	virtual ~ListenHandler(){}

public:  //重写EventHander的虚函数
	virtual HANDLE_RESULT on_readable(int fd);
	//virtual HANDLE_RESULT on_writeabble(int fd){return HANDLE_OK;}	//to do deal with timeout
	//virtual HANDLE_RESULT on_timeout(int fd){return HANDLE_OK;} 		//to do deal with timeout
	//virtual HANDLE_RESULT on_error(int fd){return HANDLE_OK;} 		//to do deal with error

protected:
	//返回值:从listen_fd接收到的新连接
	virtual int receive_connect(int listen_fd);
private:
	ConnectAccepter *m_connect_accepter;
};

//ListenHandler.cpp
HANDLE_RESULT ListenHandler::on_readable(int fd)
{
	SLOG_DEBUG("on readable");
	int new_fd = receive_connect(fd);
	if(new_fd == -1)
		return HANDLE_OK;

	assert(m_connect_accepter != NULL);
	if(m_connect_accepter->accept(new_fd) == false)
	{
		SLOG_ERROR("connect_accepter accepts new connection failed. close connection. fd=%d", new_fd);
		close(new_fd);
	}

	return HANDLE_OK;
}

int ListenHandler::receive_connect(int listen_fd)
{
	int fd = accept(listen_fd, NULL, 0);
	if(fd == -1)
	{
		if(errno==EAGAIN || errno==EINPROGRESS || errno==EINTR)  //被中断
			SLOG_WARN("accept client socket interrupted. errno=%d", errno);
		else
			SLOG_ERROR("accept client socket error. errno=%d", errno);
	}
	return fd;
}
```