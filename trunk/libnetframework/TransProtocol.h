/*
 * TransProtocol.h
 *
 *  Created on: 2012-11-11
 *      Author: LiuYongJin
 */

//今天是光棍节*_*
#ifndef _LIB_TRANS_PROTOCOL_H_20121111
#define _LIB_TRANS_PROTOCOL_H_20121111

#include "Socket.h"
#include "ProtocolFamily.h"

class TransProtocol
{
public:
	static bool send_protocol(TransSocket *trans_socket, Protocol *protocol);
	static bool recv_protocol(TransSocket *trans_socket, Protocol *protocol);
};

#endif //_LIB_TRANS_PROTOCOL_H_20121111



