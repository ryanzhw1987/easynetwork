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
    client_socket.open();

	char buf[100];
    int len = 100;

    sprintf(buf, "hello, my socket.");
	len = strlen(buf)+1;

	//1.发送
	SimpleCmd simple_cmd;
	simple_cmd.set_data(buf, len);
	DefaultProtocol protocol;
	protocol.attach_cmd(&simple_cmd);

	IOBuffer *send_buffer = client_socket.get_send_buffer();
	if(protocol.encode(send_buffer) == 0)
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
	//或者
	/*
	IOBuffer send_buffer;
	if(protocol.encode(&send_buffer) == 0)
    {
    	unsigned int size;
    	char *data_buffer = send_buffer.read_begin(&size);
    	client_socket.send_data(data_buffer, size)
		SLOG_DEBUG("send buffer data ok.");
	}
	*/

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
class ClientAppFramework: public SocketManager
{
public:
	ClientAppFramework(IODemuxer *io_demuxer, ProtocolFamily *protocol_family):SocketManager(io_demuxer, protocol_family){}
	int send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp)
	{
	    DefaultProtocolFamily *protocol_family = (DefaultProtocolFamily*)get_protocol_family();
		Protocol *protocol = protocol_family->create_protocol(cmd);
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
};

void test_socket_manager()
{
    SLOG_DEBUG("2. test framework......");
	EpollDemuxer io_demuxer;
	DefaultProtocolFamily protocol_family;
	ClientAppFramework cient_app_framework(&io_demuxer, &protocol_family);  //异步
    SocketHandle socket_handle = cient_app_framework.create_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
    if(socket_handle == SOCKET_INVALID)
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
        cient_app_framework.send_cmd(socket_handle, (Command*)simple_cmd, true);
    }
   	io_demuxer.run_loop();
}

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	
	test_socket(); //使用socket和protocol来发送数据
	test_socket_manager();  //使用框架来发送数据

	SLOG_UNINIT();
	return 0;
}



