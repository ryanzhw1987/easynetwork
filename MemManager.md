## MemManager ##

MemManager模块实现内存的管理和优化.

为何需要内存管理?在高并发,高吞吐量的服务器中, 每秒可能有上万个连接请求,请求完后断开连接,或者不断发送协议请求.在连接创建、断开，协议的创建、销毁过程中，极容易产生内存碎片，影响服务器的性能,所以需要内存管理来解决这个问题.

MemManager提供一个MemCache的模版.当框架需要一个对象的时候,从MemCache中分配,当该对象需要释放时,由MemCache收回并管理这些自由的内存空间, 当下次分配时,从这些空闲的空间中获取,从而减少对系统调用alloc(new)和free(delete)的请求次数,避免产生内存碎片.

# Details #
  * **Memcache模板**
```
class MemCache
{
public:
	MemCache():m_free_list(NULL){}
	~MemCache();
	T* Alloc();
	bool Free(T *&element);
private:
	void* m_free_list;
};
```

  * **分配protocol例子**
    * 定义StringProtocl的memcache:
```
MemCache<StringProtocol> m_string_protocol_memcache;
```
    * 生成StringProtocol
```
Protocol* protocol = (Protocol*)m_string_protocol_memcache.Alloc();
```
    * 释放StringProtocol
```
StringProtocol* string_protocol = (StringProtocol*)protocol;
m_string_protocol_memcache.Free(string_protocol);
```

  * **分配TransSocket**
    * 定义TransSocket的Memcache
```
MemCache<TransSocket> m_trans_socket_memcache;
```
    * 生成TransSocket
```
Socket* socket = (Socket*)m_trans_socket_memcache.Alloc();
```
    * 释放TransSocket
```
TransSocket* trans_socket = (TransSocket*)socket;
m_trans_socket_memcache.Free(trans_socket);
```


这些虽然都是由框架来完成的,但是应用程序中同样可以使用Memcache.