#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "Socket.h"

#include "slog.h"

//使用socket来发送数据
void test_socket()
{
    SLOG_DEBUG("1. test socket.....");
    TransSocket client_socket("127.0.0.1", 3010, BLOCK);
    client_socket.connect_server();

	char buf[100];
    int len = 100;

    sprintf(buf, "hello, my socket.");
	len = strlen(buf)+1;

	//1.发送
	SimpleCmd simple_cmd;
	simple_cmd.set_data(buf, len);
	DefaultProtocol protocol;
	protocol.attach_cmd(&simple_cmd);

	EncodeBuffer encode_buf;
	protocol.encode(&encode_buf);
    client_socket.send_data(encode_buf.get_buffer(), encode_buf.get_size());
	protocol.detach_cmd();
	
	//2. 接收
	int header_size = protocol.get_header_size();
	int ret = client_socket.recv_data(buf, header_size);
	if(ret == header_size)
	{
		ret = protocol.decode_header(buf, header_size);
		if(ret == 0)
		{
			int body_size = protocol.get_body_size();
			ret= client_socket.recv_data(buf, body_size);
 			if(ret == body_size &&	protocol.decode_body(buf, body_size) ==0)
 			{
 				ProtocolType type = protocol.get_type();
				switch(type)
				{
				case PROTOCOL_SIMPLE:
					{
						SimpleCmd* cmd = (SimpleCmd*)protocol.get_cmd();
						SLOG_DEBUG("receive data:%s.", cmd->get_data());
					}
					break;
				}
			}
		}
	}
}


//使用框架来发送数据
//应用程序框架
//重写父类成员函数recv_protocol,实现业务层逻辑
class AppFramework: public SocketManager
{
public:
	int send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp)
	{
	    DefaultProtocolFamily *protocol_family = (DefaultProtocolFamily*)get_protocol_family();
		DefaultProtocol *protocol = (DefaultProtocol*)protocol_family->create_protocol(cmd);
		send_protocol(socket_handle, protocol, has_resp);
		return 0;
	}

    //重写父类函数,实现业务层逻辑
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)
	{
		DefaultProtocol* default_protocol = (DefaultProtocol*)protocol;
		ProtocolType type = default_protocol->get_type();
		switch(type)
		{
		case PROTOCOL_SIMPLE:
			{
				SimpleCmd* simple_cmd = (SimpleCmd*)default_protocol->get_cmd();
				SLOG_DEBUG("receive server resp. simple cmd. recevie data:%s.", simple_cmd->get_data());
			}
			break;
		default:
			SLOG_DEBUG("reveive undefine cmd.");
			break;
		}

		//get_io_demuxer()->exit();
		return 0;
	}
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
    {
    	SLOG_ERROR("app send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
    	delete protocol;
    	return 0;
    }

    int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
    {
    	SLOG_DEBUG("app send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
    	delete protocol;
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

	//应用层所使用的io复用
	IODemuxer* get_io_demuxer()
	{
		static EpollDemuxer epoll_demuxer;
		return &epoll_demuxer;
	}

	//应用层所使用的协议族
	ProtocolFamily* get_protocol_family()
	{
		static DefaultProtocolFamily default_protocol_family;
		return &default_protocol_family;
	}
};

void test_socket_manager()
{
    SLOG_DEBUG("2. test framework......");

	AppFramework app_framework;  //异步
    SocketHandle socket_handle = app_framework.create_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
    if(socket_handle == HANDLE_INVALID)
        return;

    char buf[100];
    int len = 100;

    int i=0;
    while(i++<5)
    {
        sprintf(buf, "hello, my socket.command index=%d", i);
	    len = strlen(buf)+1;
        SimpleCmd *simple_cmd = new SimpleCmd;
        simple_cmd->set_data(buf, len);
        app_framework.send_cmd(socket_handle, (Command*)simple_cmd, true);
    }
	app_framework.get_io_demuxer()->run_loop();
}

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	
	test_socket(); //使用socket和protocol来发送数据
	test_socket_manager();  //使用框架来发送数据
	
	SLOG_UNINIT();
	return 0;
}



