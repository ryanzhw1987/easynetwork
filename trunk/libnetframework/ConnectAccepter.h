/*
 * ConnectAccepter.h
 *
 *  Created on: 2012-9-6
 *      Author: LiuYongjin
 *
 */

#ifndef _LIB_CONNECT_ACCEPTER_H_
#define _LIB_CONNECT_ACCEPTER_H_

#include "SocketType.h"

//链接请求接收器
class ConnectAccepter
{
public:
	virtual ~ConnectAccepter(){}
	virtual bool accept(SocketHandle trans_fd) = 0;
};

#endif //_LIB_CONNECT_ACCEPTER_H_

