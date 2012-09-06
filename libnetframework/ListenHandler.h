/*
 * SocketHandler.h
 *
 *  Created on: 2012-9-5
 *      Author: LiuYongjin
 */

#ifndef _LIB_SOCKET_HANDLER_H_
#define _LIB_SOCKET_HANDLER_H_

#include "SocketType.h"
#include "EventHandler.h"

class ConnectAccepter
{
public:
	virtual bool accept(SocketHandle trans_fd) = 0;
};

class ListenHandler:public EventHandler
{
public:
	ListenHandler(ConnectAccepter *connect_accepter):m_connect_accepter(connect_accepter){}
public:  //重写EventHander的虚函数
	HANDLE_RESULT on_readable(int fd);
	//HANDLE_RESULT on_writeabble(int fd){return HANDLE_OK;}	//to do deal with timeout
	//HANDLE_RESULT on_timeout(int fd){return HANDLE_OK;} 		//to do deal with timeout
	//HANDLE_RESULT on_error(int fd){return HANDLE_OK;} 		//to do deal with error

private:
	ConnectAccepter *m_connect_accepter;
};

#endif //_LIB_SOCKET_HANDLER_H_
