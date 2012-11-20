/*
 * TemplateProtocolFamily.cpp
 *
 *  Created on: #CreateDate#
 *      Author: #Author#
 */

#include "TemplateProtocolFamily.h"

//////////////////////////////  ProtooclFamily  //////////////////////////////
Protocol* TemplateProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	Protocol *protocol = NULL;
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();

	switch(protocol_type)
	{
#if TEST
	case PROTOCOL_TEST:
		protocol = new TemplateProtocol;
		break;
#endif

	////Add Your Code Frome Here
	////////////////////////////

	}

	return protocol;
}

void TemplateProtocolFamily::destroy_protocol(Protocol *protocol)
{
	////Add Your Code To Destroy Protocol From Here
	////////////////////////////////////////////////
	delete protocol;
}

//////////////////////////////  Protocol  //////////////////////////////
#if TEST
////编码协议体数据到io_buffer,成功返回true,失败返回false.
bool TemplateProtocol::encode_body(ByteBuffer *byte_buffer)
{
	////m_value
	ENCODE_INT(m_value);
	////m_str
	ENCODE_STRING(m_str);

	return true;
}

////解码协议体数据io_buffer.成功返回true,失败返回false.
bool TemplateProtocol::decode_body(const char *buf, int size)
{
	////m_value
	DECODE_INT(m_value);
	////m_str
	DECODE_STRING(m_str);

	return true;
}
#endif

////Add Your Protocol From Here
///////////////////////////////


