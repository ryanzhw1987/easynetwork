/*
 * SocketType.h
 *
 *  Created on: 2012-9-5
 *      Author: LiuYongjin
 */
#ifndef _LIB_SOCKET_TYPE_H_
#define _LIB_SOCKET_TYPE_H_

typedef int SocketHandle;
#define SOCKET_INVALID -1

typedef enum
{
	BLOCK,
	NOBLOCK
}BlockMode;

#endif //_LIB_SOCKET_TYPE_H_
