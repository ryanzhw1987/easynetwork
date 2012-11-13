/*
 * MTServerAppFramework.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "MTAppTemplate.h"
#include "slog.h"

/////////////////////////////////////// MTAppTemplate ///////////////////////////////////////
bool MTAppTemplate::start_server()
{
	//Init NetInterface
	init_net_interface();

	////Add your codes here
	///////////////////////

	return true;
}

ProtocolFamily* MTAppTemplate::create_protocol_family()
{
	//Create your protocol family here
	//return new 'YourProtocolFamily';
}

void MTAppTemplate::delete_protocol_family(ProtocolFamily* protocol_family)
{
	//Delete your protocol family that created by function 'create_protocol_family'
	delete protocol_family;
}

bool MTAppTemplate::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	//Add your code to handle the protocol
	//////////////////////////////////////

	return false;
}

bool MTAppTemplate::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("Thread[ID=%d] send protocol[details=%s] error. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MTAppTemplate::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("Thread[ID=%d] send protocol[details=%s] succ. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MTAppTemplate::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket error. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket error
	//////////////////////////////////////////

	return true;
}

bool MTAppTemplate::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket timeout. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket timeout
	////////////////////////////////////////////

	return true;
}

bool MTAppTemplate::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("Thread[ID=%d] handle new socket. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle new socket
	////////////////////////////////////

	return true;
}


/////////////////////////////////////// MTAppTemplateThreadPool ///////////////////////////////////////
Thread<SocketHandle>* MTAppTemplateThreadPool::create_thread()
{
	MTAppTemplate *app = new MTAppTemplate;
	return app;
}


