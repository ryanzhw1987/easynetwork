#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "Socket.h"
#include "NetInterface.h"
#include "slog.h"

//使用socket来发送数据
void test_socket()
{
    SLOG_DEBUG("1. test socket.....");
    TransSocket client_socket("127.0.0.1", 3010, BLOCK);
    client_socket.open();

	char buf[100];
    int len = 100;

    sprintf(buf, "hello, my socket.");
	len = strlen(buf)+1;

	//1.发送
	DefaultHeader default_header(PROTOCOL_STRING, VERSION, MAGIC_NUM);
	StringProtocol string_protocol;
	string_protocol.set_header(&default_header);
	string_protocol.set_string("hello, my lib.");

	IOBuffer *send_buffer = client_socket.get_send_buffer();
	if(string_protocol.encode(send_buffer) == 0)
    {
    	TransStatus trans_status;
		while((trans_status=client_socket.send_buffer()) == TRANS_PENDING)
			;
		if(trans_status == TRANS_ERROR)
		{
			SLOG_ERROR("send buffer data error.");
			return ;
		}
		SLOG_DEBUG("send buffer data ok.");
	}
	else
		return ;

	//2. 接收
	int header_size = default_header.get_header_size();
	int ret = client_socket.recv_data(buf, header_size);
	if(ret == header_size)
	{
		ret = default_header.decode(buf, header_size);
		if(ret == 0)
		{
			int body_size = default_header.get_body_size();
			ret= client_socket.recv_data(buf, body_size);
 			if(ret == body_size)
 			{
 				switch(default_header.get_type())
 				{
 				case PROTOCOL_STRING:
 					{
 						string_protocol.decode_body(buf, body_size);
 						string resp = string_protocol.get_string();
 						SLOG_DEBUG("receive from server:\"%s\"", resp.c_str());
 					}
 					break;
 				default:
 					SLOG_DEBUG("receive undefine protocol. ignore it.");
 					break;
 				}
 			}
		}
	}
}


//使用框架来发送数据
//应用程序框架
//重写父类成员函数recv_protocol,实现业务层逻辑
class ClientAppFramework: public NetInterface
{
public:

    //重写父类函数,实现业务层逻辑
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
	{
		switch(((DefaultProtocol*)protocol)->get_type())
		{
		case PROTOCOL_STRING:
			{
				string resp = ((StringProtocol*)protocol)->get_string();
				SLOG_DEBUG("receive from server:[%s]", resp.c_str());
			}
			break;
		default:
			SLOG_DEBUG("receive undefine protocol. ignore it.");
			break;
		}

		//get_io_demuxer()->exit();
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
		SLOG_DEBUG("app socket handle timeout. fd=%d", socket_handle);
		get_io_demuxer()->exit();
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

void test_socket_manager()
{
	SLOG_DEBUG("2. test framework......");

	ClientAppFramework cient_app_framework;  //异步
	SocketHandle socket_handle = cient_app_framework.get_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
	if(socket_handle == SOCKET_INVALID)
	return;

	char buf[100];
	int len = 100;

	int i=0;
	while(i++<5)
	{
		sprintf(buf, "hello, my socket.command index=%d", i);
		len = strlen(buf)+1;
		StringProtocol *string_protocol = (StringProtocol *)cient_app_framework.get_protocol_family()->create_protocol(PROTOCOL_STRING);
		string_protocol->set_string(buf);
		cient_app_framework.send_protocol(socket_handle, string_protocol, true);
	}
	cient_app_framework.get_io_demuxer()->run_loop();
}

int main()
{
	SLOG_INIT(NULL);
	
	test_socket(); //使用socket和protocol来发送数据
	test_socket_manager();  //使用框架来发送数据

	SLOG_UNINIT();
	return 0;
}



