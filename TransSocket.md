## TransSocket ##

TransSocket类实现socket的链接,数据的传送.
  1. 包含一个receive [IOBuffer](IOBuffer.md)用于接收数据.
  1. 包含一个send [IOBuffer](IOBuffer.md)用于发送数据.

# Details #

  * **实现[Socket](Socket.md)的open方法**
```
virtual bool open(int timeout_ms=2000);
```
连接服务器, 链接超过timeout\_ms,连接失败.

  * **接收数据recv\_data**
```
//尝试接收指定长度的数据.
//返回值:
//大于0:成功返回读取的字节数(可能是部分数据).
//TRANS_CLOSE: 连接正常关闭
//TRANS_NODATA: 没有数据
//TRANS_ERROR: 失败
virtual int recv_data(char *buffer, int len);
```

  * **发送数据send\_data**
```
//发送指定长度的数据(全部发送)
//返回值:
//大于0: 发送的字节数
//TRANS_ERROR: 失败
virtual int send_data(char *buffer, int len);
```

  * **获取输入缓冲区**
```
IOBuffer* get_recv_buffer(){return &m_recv_buffer;}
```

  * **获取输出缓冲区**
```
IOBuffer* get_send_buffer(){return &m_send_buffer;}
```

  * **接收数据到io buffer**
```
//接收所有数据到输入缓冲区.!!!***仅用于非阻塞模式***!!!
//返回值:
//TRANS_OK:成功
//TRANS_NOMEM: 没有内存
//TRANS_ERROR: 错误
//TRANS_CLOSE: 对端关闭链接
//TRANS_BLOCK: 当前是阻塞模式
TransStatus recv_buffer();
```
  * **发送io buffer中的数据**
```
//尝试发送输出缓冲区中的所有数据,直到发送完成或者发送不出去.
//返回值:
//TRANS_OK:成功
//TRANS_PENDING: 只发送部分数据,缓冲区还有数据待发送
//TRANS_ERROR: 错误
TransStatus send_buffer();
```