/*
 * STAppTemplate.cpp
 *
 *  Created on: #CreateDate#
 *      Author: #Author#
 */

#include "STAppTemplate.h"
#include "slog.h"


bool STAppTemplate::start_server()
{
	////Init NetInterface
	init_net_interface();

	////Add your codes here
	///////////////////////

	return true;
}

ProtocolFamily* STAppTemplate::create_protocol_family()
{
	//Create your protocol family here
	//return new 'YourProtocolFamily';
}

void STAppTemplate::delete_protocol_family(ProtocolFamily* protocol_family)
{
	//Delete your protocol family that created by function 'create_protocol_family'
	delete protocol_family;
}

bool STAppTemplate::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	//Add your code to handle the protocol
	//////////////////////////////////////

	return true;
}

bool STAppTemplate::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("Send protocol[detail=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool STAppTemplate::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("Send protocol[detail=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool STAppTemplate::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("Handle socket error. fd=%d", socket_handle);
	//Add your code to handle the socket error
	//////////////////////////////////////////

	return true;
}

bool STAppTemplate::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("Handle socket timeout. fd=%d", socket_handle);
	//Add your code to handle the socket timeout
	////////////////////////////////////////////

	return true;
}

bool STAppTemplate::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("Handle new socket. fd=%d", socket_handle);
	//Add your code to handle new socket
	////////////////////////////////////

	return true;
}

