#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "ListenHandler.h"
#include "NetInterface.h"

#include "slog.h"

class TimerHandler:public EventHandler
{
public:
	TimerHandler(IODemuxer *demuxer):m_demuxer(demuxer){;}
	HANDLE_RESULT on_timeout(int fd)
	{
		SLOG_DEBUG("timer timeout...");
		m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	
		return HANDLE_OK;
	}
private:
	IODemuxer *m_demuxer;
};

//应用程序框架
//重写父类成员函数recv_protocol,实现业务层逻辑
class AppFramework: public NetInterface
{
public:

	//重写父类函数,实现业务层逻辑
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
	{
		switch(((DefaultProtocol*)protocol)->get_type())
		{
		case PROTOCOL_STRING:
			{
				StringProtocol* string_protocol = (StringProtocol*)protocol;
				string data = string_protocol->get_string();
				SLOG_DEBUG("receive string protocol from fd=%d. receive data:[%s], length=%d", socket_handle, data.c_str(), data.length());

				StringProtocol* resp_protocol = (StringProtocol*)((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
				string temp = "server receive data:";
				temp += data;
				resp_protocol->set_string(temp);
				send_protocol(socket_handle, resp_protocol);
			}
			break;
		default:
			SLOG_WARN("reveive undefine protocol. ignore it.");
			break;
		}

		return 0;
	}

	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
	{
		SLOG_ERROR("app send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
		get_protocol_family()->destroy_protocol(protocol);
		return 0;
	}

	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
	{
		SLOG_DEBUG("app send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
		get_protocol_family()->destroy_protocol(protocol);
		return 0;
	}

	int on_socket_handle_error(SocketHandle socket_handle)
	{
		SLOG_DEBUG("app socket handle error. fd=%d", socket_handle);
		return 0;
	}

	int on_socket_handle_timeout(SocketHandle socket_handle)
	{
		SLOG_DEBUG("app socket handle error. fd=%d", socket_handle);
		return 0;
	}

	//////////////////由应用层重写 创建IODemuxer//////////////////
	IODemuxer* create_io_demuxer()
	{
		return new EpollDemuxer;
	}
	//////////////////由应用层重写 销毁IODemuxer//////////////////
	void delete_io_demuxer(IODemuxer* io_demuxer)
	{
		delete io_demuxer;
	}
	//////////////////由应用层重写 创建SocketManager//////////////
	SocketManager* create_socket_manager()
	{
		return new SocketManager();
	}
	//////////////////由应用层重写 销毁SocketManager//////////////
	void delete_socket_manager(SocketManager* socket_manager)
	{
		delete socket_manager;
	}
	//////////////////由应用层重写 创建具体的协议族//////////////
	ProtocolFamily* create_protocol_family()
	{
		return new DefaultProtocolFamily;
	}
	//////////////////由应用层重写 销毁协议族////////////////////
	void delete_protocol_family(ProtocolFamily* protocol_family)
	{
		delete protocol_family;
	}
};

int main()
{
	SLOG_INIT(NULL);

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	AppFramework app_server;

	//listen event
	ListenHandler listen_handler(&app_server);
	app_server.get_io_demuxer()->register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//timer event
	TimerHandler timer(app_server.get_io_demuxer());
	app_server.get_io_demuxer()->register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server
	app_server.get_io_demuxer()->run_loop();

	SLOG_UNINIT();
	return 0;
}



