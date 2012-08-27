#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "Socket.h"

#include "slog.h"

//ʹ��socket����������
void test_socket()
{
    SLOG_DEBUG("1. test socket.....");
    TransSocket client_socket("127.0.0.1", 3010, BLOCK);
    client_socket.connect_server();

	char buf[100];
    int len = 100;

    sprintf(buf, "hello, my socket.");
	len = strlen(buf)+1;

	//1.����
	SimpleCmd simple_cmd;
	simple_cmd.set_data(buf, len);
	DefaultProtocol protocol;
	protocol.attach_cmd(&simple_cmd);

	EncodeBuffer encode_buf;
	protocol.encode(&encode_buf);
    client_socket.send_data(encode_buf.get_buffer(), encode_buf.get_size());
	protocol.detach_cmd();
	
	//2. ����
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


//ʹ�ÿ������������
//Ӧ�ó�����
//��д�����Ա����recv_protocol,ʵ��ҵ����߼�
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

    //��д���ຯ��,ʵ��ҵ����߼�
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

	//Ӧ�ò���ʹ�õ�io����
	IODemuxer* get_io_demuxer()
	{
		static EpollDemuxer epoll_demuxer;
		return &epoll_demuxer;
	}

	//Ӧ�ò���ʹ�õ�Э����
	ProtocolFamily* get_protocol_family()
	{
		static DefaultProtocolFamily default_protocol_family;
		return &default_protocol_family;
	}
};

void test_socket_manager()
{
    SLOG_DEBUG("2. test framework......");

	AppFramework app_framework;  //�첽
    SocketHandle socket_handle = app_framework.create_active_trans_socket("127.0.0.1", 3010);  //������������
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
	
	test_socket(); //ʹ��socket��protocol����������
	test_socket_manager();  //ʹ�ÿ������������
	
	SLOG_UNINIT();
	return 0;
}



