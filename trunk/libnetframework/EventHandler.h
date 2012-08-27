#ifndef	 _LIB_EVENT_HANDLER_H_20120728_LIUYONGJIN
#define	_LIB_EVENT_HANDLER_H_20120728_LIUYONGJIN

#include "slog.h"

//�¼�����
typedef unsigned short EVENT_TYPE;
const EVENT_TYPE EVENT_INVALID 		= 0x0;
const EVENT_TYPE EVENT_READ			= 0x1; //�ɶ��¼�
const EVENT_TYPE EVENT_WRITE		= 0x2; //��д�¼�
const EVENT_TYPE EVENT_PERSIST	    = 0x10;//ֻ�е���EVENT_READ��ϻ�����ʱ���¼�ʱ����Ч

typedef enum
{
    HANDLE_OK,
    HANDLE_ERROR
}HANDLE_RESULT;

//�¼��ص���
class EventHandler
{
public:
	virtual HANDLE_RESULT on_readable(int fd)
	{
		SLOG_TRACE("fd=%d reable able. do nothing.",fd);
		return HANDLE_OK;
	}
	virtual HANDLE_RESULT on_writeabble(int fd)
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

