## EventHandler ##
EventHandler是事件响应接口。主要用来监听链接的读，写，错误，超时等事件。将一个链接和其对应的具体的事件相应类注册到IODemuxer后，在该链接上发生事件时会调用该链接对应的事件处理方法。IODemuxer和各种各样的EventHandler组成了EasyNetwork框架的事件驱动模型。EventHandler的接口方法如下：

  * 响应socket可读
```
virtual HANDLE_RESULT on_readable(int fd);
```

  * 响应socket可写
```
virtual HANDLE_RESULT on_writeabble(int fd);
```

  * 响应超时
```
virtual HANDLE_RESULT on_timeout(int fd);
```

  * 响应错误
```
virtual HANDLE_RESULT on_error(int fd);
```