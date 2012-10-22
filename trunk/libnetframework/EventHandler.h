#ifndef	 _LIB_EVENT_HANDLER_H_20120728_LIUYONGJIN
#define	_LIB_EVENT_HANDLER_H_20120728_LIUYONGJIN

#include "slog.h"

//事件类型
typedef unsigned short EVENT_TYPE;
const EVENT_TYPE EVENT_INVALID     = 0x0;
const EVENT_TYPE EVENT_READ        = 0x1; //可读事件
const EVENT_TYPE EVENT_WRITE       = 0x2; //可写事件
const EVENT_TYPE EVENT_PERSIST     = 0x10;//只有当和EVENT_READ组合或者是时钟事件时才有效

typedef enum
{
	HANDLE_OK,
	HANDLE_ERROR
}HANDLE_RESULT;

//事件回调类
class EventHandler
{
public:
	virtual ~EventHandler(){}
	virtual HANDLE_RESULT on_readable(int fd)
	{
		SLOG_TRACE("fd=%d reable able. do nothing.",fd);
		return HANDLE_OK;
	}
	virtual HANDLE_RESULT on_writeable(int fd)
	{
		SLOG_TRACE("fd=%d write able. do nothing.",fd);
		return HANDLE_OK;
	}
	virtual HANDLE_RESULT on_timeout(int fd)
	{
		SLOG_TRACE("fd=%d timeout. do nothing.",fd);
		return HANDLE_OK;
	}
	virtual HANDLE_RESULT on_error(int fd)
	{
		SLOG_TRACE("fd=%d error. do nothing.",fd);
		return HANDLE_OK;
	}
};

#endif //_LIB_EVENT_HANDLER_H_20120728_LIUYONGJIN

