#include "ProtocolDefault.h"

#include <stdio.h>
#include <string.h>

int main()
{
	//1. 编码
	char str[] = "test simple cmd";
	unsigned int size = sizeof(str);
	
	SimpleCmd *simple_cmd = new SimpleCmd;
	simple_cmd->set_data(str, size);

	DefaultProtocol default_protocol;
	default_protocol.attach_cmd(simple_cmd);

	IOBuffer io_buffer;
	default_protocol.encode(&io_buffer);
	io_buffer.read_begin(&size);
	printf("encode buf: data size=%d\n", size);
	
	//2. 解码
	char *buf = io_buffer.read_begin(&size);
	DefaultProtocolFamily default_family;
	DefaultProtocol* protocol = (DefaultProtocol*)default_family.create_protocol();
	
	int header_size = protocol->get_header_size();
	if(protocol->decode_header(buf, header_size) == 0)
	{
		int body_size = protocol->get_body_size();
		if(protocol->decode_body(buf+header_size, body_size) == 0)
		{
			ProtocolType type = protocol->get_type();
			if(type == PROTOCOL_SIMPLE)
			{
				SimpleCmd *simple_cmd = (SimpleCmd *)protocol->get_cmd();
				printf("decode simple cmd. data:%s, size=%d\n", simple_cmd->get_data(), simple_cmd->get_size());
			}
		}
	}

	delete protocol;

	return 0;
}
