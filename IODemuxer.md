## !IODemuxer ##

IODemuxer接口类表示一个IO多路复用。其提供了注册，分配事件的接口方法。需要监听一个链接的读，写，错误等事件时，将该链接和对应的EventHandler事件处理类注册到IODemuxer中，由IODemuxer监听该链接，当该链接发送事件时，IODemuxer调用对应的接口方法实现事件的分派。

另外IODemuxer还实现了时钟超时检查功能。通过注册相应的时钟超时事件类，即可处理超时事件。

  * **IODemuxer方法**

  * 注册事件register\_event.事件发生后由EventHandler响应发生的事件
```
//fd: socket描述符. 当为小于0时表示注册的是时钟超时事件,此时timeout_ms必须大于0,并且无法注销;
//type: fd的读写事件.fd大于0时有效.
//timeout_ms: fd读写事件超时时间.或者时钟超时时间.单位毫秒.
//handler: 事件响应函数类.
//返回值:-1失败. 0成功;
virtual int register_event(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler)=0;
```

  * 事件类型
```
typedef unsigned short EVENT_TYPE;
const EVENT_TYPE EVENT_INVALID     = 0x0;
const EVENT_TYPE EVENT_READ        = 0x1; //可读事件
const EVENT_TYPE EVENT_WRITE       = 0x2; //可写事件
const EVENT_TYPE EVENT_PERSIST     = 0x10;//只有当和EVENT_READ组合或者是时钟事件时才有效
```

  * 事件响应类EventHandler
响应事件

  * 注销事件unregister\_event
```
//注销fd的事件. fd大于0时有效.0成功, -1失败
virtual int unregister_event(int fd)=0;
```

  * 分配事件
```
//循环等待/处理事件
virtual int run_loop()=0;
```

  * 退出
```
virtual void exit()=0;
```

  * IODemuxerEpoll
用epoll实现IODemuxer.