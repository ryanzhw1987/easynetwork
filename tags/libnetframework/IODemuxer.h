#ifndef _LIB_IO_DEMUXER_H_20120613_LIUYONGJIN
#define	_LIB_IO_DEMUXER_H_20120613_LIUYONGJIN

#include <stdio.h>
#include "EventHandler.h"

//网络io多路复用虚基类
class IODemuxer
{
public:
	virtual ~IODemuxer(){}
	//注册事件.
	//fd: socket描述符. 当为小于0时表示注册的是时钟超时事件,此时timeout_ms必须大于0,并且无法注销;
	//type: fd的读写事件.fd大于0时有效.
	//timeout_ms: fd读写事件超时时间.或者时钟超时时间.单位毫秒.
	//handler: 事件响应函数类.
	//返回值:-1失败. 0成功;
	virtual int register_event(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler)=0;

	//注销fd的事件. fd大于0时有效.0成功, -1失败
	virtual int unregister_event(int fd)=0;

	//循环等待/处理事件
	virtual int run_loop()=0;
	virtual void exit()=0;
};

#endif //_LIB_IO_DEMUXER_H_20120613_LIUYONGJIN

