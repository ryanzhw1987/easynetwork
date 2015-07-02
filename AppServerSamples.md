# Introduction #

介绍如何在最短时间内用框架搭建自己的服务器(单线程和多线程).


# Details #
  * **单线程框架**
  * 继承NetInterface,实现接口函数
```
class ServerAppFramework: public NetInterface
{
public:
	ServerAppFramework(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager* socket_manager)
			:NetInterface(io_demuxer, protocol_family, socket_manager)
	{}

	//////////////////由应用层重写 接收协议函数//////////////////
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	int on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	int on_socket_handle_timeout(SocketHandle socket_handle);
};


int ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			string data = string_protocol->get_string();
			SLOG_DEBUG("receive string protocol from fd=%d. receive data:[%s], length=%d", socket_handle, data.c_str(), data.length());

			Protocol* resp_protocol = ((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
			string temp = "server receive data:";
			temp += data;
			((StringProtocol*)resp_protocol)->set_string(temp);
			send_protocol(socket_handle, resp_protocol);
		}
		break;
	default:
		SLOG_DEBUG("receive undefine protocol. ignore it.");
		break;
	}
	
	return 0;
}

int ServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int ServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int ServerAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int ServerAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}
```

  * 单线程框架main函数
```
///////////// main 函数 ////////////
int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	EpollDemuxer io_demuxer;
	DefaultProtocolFamily protocol_family;
	SocketManager socket_manager;
	ServerAppFramework app_server(&io_demuxer, &protocol_family, &socket_manager);

	//listen event
	ListenHandler listen_handler(&app_server);
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);
	//timer event
	TimerHandler timer(&io_demuxer);
	io_demuxer.register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}
```

  * **多线程框架**
  * 继承ConnectThread
```
class MTServerAppFramework:public ConnectThread
{
public:
	MTServerAppFramework(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager* socket_manager)
			:ConnectThread(io_demuxer, protocol_family, socket_manager)
	{}

	//////////////////由应用层重写 接收协议函数//////////////////
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	int on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	int on_socket_handle_timeout(SocketHandle socket_handle);

protected://实现Thread到纯虚函数
	void run();
};


//////////////////由应用层重写 接收协议函数//////////////////
void MTServerAppFramework::run()
{
	SLOG_INFO("MTServerAppFramework[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("MTServerAppFramework end...");
}

int MTServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			string data = string_protocol->get_string();
			SLOG_INFO("thread[ID=%d] receive string protocol from fd=%d. receive data:[%s], length=%d", get_id(), socket_handle, data.c_str(), data.length());

			Protocol* resp_protocol = ((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
			string temp = "server receive data:";
			temp += data;
			((StringProtocol*)resp_protocol)->set_string(temp);
			send_protocol(socket_handle, resp_protocol);
		}
		break;
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return 0;
}

int MTServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int MTServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int MTServerAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int MTServerAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}

```

  * 继承ConnectThreadPool实现create\_thread
```
class MTServerAppFrameworkPool:public ConnectThreadPool
{
public:
	MTServerAppFrameworkPool(unsigned int thread_num):ConnectThreadPool(thread_num){}
protected:
	//实现创建一个线程
	Thread* create_thread();
};

/////////////////  thread pool  /////////////////////
Thread* MTServerAppFrameworkPool::create_thread()
{
	EpollDemuxer *io_demuxer = new EpollDemuxer;
	DefaultProtocolFamily *protocol_family = new DefaultProtocolFamily;
	SocketManager *socket_manager = new SocketManager;
	return new MTServerAppFramework(io_demuxer, protocol_family, socket_manager);
}

```

  * 多线程框架main函数
```
int main()
{
	//SLOG_INIT(SLOG_LEVEL_INFO, NULL, 0);
	SLOG_INIT_WITH_CONFIG("./config/slog.config");

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}



	MTServerAppFrameworkPool server_pool(3);  //3个线程
	server_pool.start();

	//listen event
	ListenHandler listen_handler(&server_pool);
	EpollDemuxer io_demuxer;
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//timer event
	TimerHandler timer(&io_demuxer);
	io_demuxer.register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}

```int main()
{
	//SLOG_INIT(SLOG_LEVEL_INFO, NULL, 0);
	SLOG_INIT_WITH_CONFIG("./config/slog.config");

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}



	MTServerAppFrameworkPool server_pool(3);  //3个线程
	server_pool.start();

	//listen event
	ListenHandler listen_handler(&server_pool);
	EpollDemuxer io_demuxer;
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//timer event
	TimerHandler timer(&io_demuxer);
	io_demuxer.register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}

}}}```