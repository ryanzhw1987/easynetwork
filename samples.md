**一、前言**
> 本节分别介绍利用EasyNetwork搭建单线程和多线程服务器框架的例子。我们的目标是在客户端和服务端传送简单的字符串协议。在搭建服务器框架前，我们需要先创建自己的协议和协议类。这一般也是开发者需要提前准备的。现在开始我们的1分钟搭建服务器框架之旅，Let‘s go...

**二、协议族StringProtocolFamily**
  1. SpringProtocolFamily从DefaultProtocolFamily继承，用于创建和销毁StringProtocol协议：
```

class StringProtocolFamily: public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
 };

inline
Protocol* StringProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
	Protocol *protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_STRING:
		protocol  =  new StringProtocol;
		break;
	}
	
	return protocol;
}
 
inline
void StringProtocolFamily::destroy_protocol(Protocol *protocol)
{
	delete protocol;
}
```
> 这两个接口允许开发者自己控制协议的创建和销毁方式，比如开发者可以提供自己的内存管理模块，并从内存管理模块中分配和销毁自己的协议对象。[多任务多线程下载](Download.md)展示了利用内存管理模块分配协议对象的例子。

<br><br>
2. StringProtocol协议<br>
<blockquote>StringProtocol从Protocol继承，需要实现encode_body和decode_body等接口方法：<br>
<pre><code>class StringProtocol:public Protocol<br>
{<br>
public: <br>
	//协议的描述信息 <br>
	const char* details(){return "raw_string_protocol";}<br>
	//编码协议体数据到io_buffer,成功返回true,失败返回false. 	<br>
	bool encode_body(ByteBuffer *byte_buffer); 	<br>
	//解码协议体数据io_buffer.成功返回true,失败返回false. 	<br>
	bool decode_body(const char *buf, int size);<br>
};<br>
<br>
/////////////////////////////////////////////////////////////////////<br>
//编码整数<br>
#define ENCODE_INT(i) do{ \<br>
if(!byte_buffer-&gt;append((const char*)&amp;i, sizeof(i))) \<br>
	return false; \<br>
}while(0)<br>
//解码整数<br>
#define DECODE_INT(i) do{ \<br>
if(size &lt; sizeof(i)) return false; \<br>
	i = *(int*)buf; buf+=sizeof(i); size-=sizeof(i); \<br>
}while(0)<br>
 <br>
//编码字符串<br>
#define ENCODE_STRING(str) do{\<br>
len = str.size(); \<br>
if(!byte_buffer-&gt;append((const char*)&amp;len, sizeof(len))) \<br>
	return false; \<br>
if(len &gt; 0 &amp;&amp; !byte_buffer-&gt;append(str.c_str())) \<br>
	return false; \<br>
 }while(0)<br>
//解码字符串<br>
#define DECODE_STRING(str) do{\<br>
DECODE_INT(len); \<br>
if(len&lt;0 || size&lt;len) return false; \<br>
if(len &gt; 0) \<br>
	str.assign(buf, len); buf+=len; size-=len; \<br>
}while(0)<br>
 <br>
//编码协议体数据到io_buffer,成功返回true,失败返回false.<br>
inline<br>
bool StringProtocol::encode_body(ByteBuffer *byte_buffer)<br>
{<br>
	int len = 0;<br>
	////m_str<br>
	ENCODE_STRING(m_str);<br>
<br>
	return true;<br>
}<br>
//解码协议体数据io_buffer.成功返回true,失败返回false.<br>
inline<br>
bool StringProtocol::decode_body(const char *buf, int size)<br>
{<br>
	int len = 0;<br>
	////m_str<br>
	DECODE_STRING(m_str);<br>
<br>
	return true;<br>
}<br>
</code></pre></blockquote>


<br><br>
<b>三、单线程开发范式</b>
<blockquote>单线程开发范式指服务器只有一个线程，在这个线程里面监听服务端口、接收链接请求、处理客户端发送的请求等。该例子见框架源码中的AppFramework_Sample。</blockquote>

<ol><li>利用enetlib工具生产框架代码<br>
<pre><code>enetlib -s ServerAppFramework -main<br>
enetlib -s ClientAppFramework -main<br>
</code></pre>
</li></ol><blockquote>将在当前目录中生成如下6个文件，分别时客户端和服务端的代码框架：<br>
<pre><code>ClientAppFramework.cpp<br>
ClientAppFramework.h<br>
ClientAppFrameworkMain.cpp<br>
<br>
ServerAppFramework.cpp<br>
ServerAppFramework.h<br>
ServerAppFrameworkMain.cpp<br>
</code></pre></blockquote>

2. 添加服务端框架代码<br>
<pre><code>//ServerAppFramework.cpp<br>
//实现NetInterface的接口<br>
bool ServerAppFramework::start_server()<br>
{<br>
	////Init NetInterface<br>
	init_net_interface();<br>
<br>
	////Add Your Codes From Here<br>
	ListenSocket linsten_socket(3010);<br>
	if(!linsten_socket.open())<br>
	{<br>
		SLOG_ERROR("listen on port:3010 error.");<br>
		return -1;<br>
	}<br>
<br>
	//listen event<br>
	ListenHandler listen_handler(this);<br>
	get_io_demuxer()-&gt;register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &amp;listen_handler);<br>
	//timer event<br>
	TimerHandler timer(get_io_demuxer());<br>
	get_io_demuxer()-&gt;register_event(-1, EVENT_INVALID, 3000, &amp;timer);<br>
<br>
	//run server forever<br>
	get_io_demuxer()-&gt;run_loop();<br>
<br>
	return true;<br>
}<br>
//创建销毁协议族<br>
ProtocolFamily* ServerAppFramework::create_protocol_family()<br>
{<br>
	return new StringProtocolFamily;<br>
}<br>
void ServerAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)<br>
{<br>
	delete protocol_family;<br>
}<br>
<br>
//接收客户端请求<br>
bool ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &amp;detach_protocol)<br>
{<br>
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol-&gt;get_protocol_header();<br>
	DefaultProtocolFamily* protocol_family = (DefaultProtocolFamily *)get_protocol_family();<br>
	switch(header-&gt;get_protocol_type())<br>
	{<br>
	case PROTOCOL_STRING:<br>
		{<br>
			StringProtocol* string_protocol = (StringProtocol*)protocol;<br>
			SLOG_INFO("receive string protocol from fd=%d. receive data:[%s].", socket_handle, string_protocol-&gt;get_string().c_str());<br>
<br>
			StringProtocol *resp_protocol = (StringProtocol *)protocol_family-&gt;create_protocol(PROTOCOL_STRING);<br>
			string str = "server respond";<br>
			resp_protocol-&gt;set_string(str);<br>
			//发送协议<br>
			if(!send_protocol(socket_handle, resp_protocol))<br>
			{<br>
				SLOG_ERROR("send protocol failed.");<br>
				protocol_family-&gt;destroy_protocol(resp_protocol);<br>
			}<br>
		}<br>
		break;<br>
	default:<br>
		SLOG_ERROR("receive undefine protocol. ignore it.");<br>
		return false;<br>
	}<br>
<br>
	return true;<br>
}<br>
</code></pre>
<blockquote>这里只简单介绍几个主要的接口方法。其他详细内容见源码。</blockquote>

<br><br>
3. 添加客户端框架代码<br>
<pre><code>//ClientAppFramework.cpp<br>
//实现NetInterface的接口<br>
bool ClientAppFramework::start_server()<br>
{<br>
	////Init NetInterface<br>
	init_net_interface();<br>
<br>
	////Add Your Codes From Here<br>
	SocketHandle socket_handle = get_active_trans_socket("127.0.0.1", 3010);  //创建主动连接<br>
	if(socket_handle == SOCKET_INVALID)<br>
		return false;<br>
<br>
	PingHandler ping_handler(this, socket_handle);<br>
	ping_handler.register_handler();<br>
<br>
	get_io_demuxer()-&gt;run_loop();<br>
<br>
	return true;<br>
}<br>
//创建销毁协议族<br>
ProtocolFamily* ClientAppFramework::create_protocol_family()<br>
{<br>
	return new StringProtocolFamily;<br>
}<br>
void ClientAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)<br>
{<br>
	delete protocol_family;<br>
}<br>
<br>
//接收服务端响应<br>
bool ClientAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &amp;detach_protocol)<br>
{<br>
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol-&gt;get_protocol_header();<br>
	int type = header-&gt;get_protocol_type();<br>
	switch(type)<br>
	{<br>
	case PROTOCOL_STRING:<br>
		{<br>
			StringProtocol *str_protocol = (StringProtocol *)protocol;<br>
			SLOG_INFO("client recv resp:[%s].", str_protocol-&gt;get_string().c_str());<br>
		}<br>
		break;<br>
	default:<br>
		SLOG_ERROR("receive undefine protocol. ignore it.");<br>
		return false;<br>
	}<br>
<br>
	//get_io_demuxer()-&gt;exit();<br>
	return true;<br>
}<br>
</code></pre>

<br><br>
4. 编译运行<br>
<pre><code>make<br>
./test_server<br>
./test_client<br>
</code></pre>


<br><br>
<b>四、多线程开发范式</b>
<blockquote>多线程开发范式指服务器使用一个主线程监听服务端口，当接收到链接请求时，把该新链接按某种策略分配到其他子线程，由该子线程负责对新链接的监听、处理客户端发送的请求等。当新链接分配到某个子线程后，该链接和主线程不再有任何关系。<br>
这里我们只生成服务端的代码，客户端代码使用上面的例子。</blockquote>

<ol><li>利用enetlib工具生成框架代码<br>
<pre><code>enetlib -m  MTServerAppFramework -main<br>
</code></pre>
</li></ol><blockquote>生成如下3个服务器框架代码文件：<br>
<pre><code> MTServerAppFramework.cpp<br>
 MTServerAppFramework.h<br>
 MTServerAppFrameworkMain.cpp<br>
</code></pre>
其中MTServerAppFramework类从ConnectThread类继承。ConnectThread实现了接收新的链接请求、监听链接事件等接口方法。具体实现请看<a href='MainFramework.md'>框架的详细介绍</a>。</blockquote>

2. 添加框架代码<br>
<pre><code>//MTServerAppFramework.cpp<br>
//实现NetInterface的接<br>
bool MTServerAppFramework::start_server()<br>
{<br>
	//初始化NetInterface<br>
	init_net_interface();<br>
<br>
	//// Add Your Codes From Here<br>
	SLOG_INFO("Start server.");<br>
	get_io_demuxer()-&gt;run_loop();<br>
	return true;<br>
}<br>
//创建销毁协议族<br>
ProtocolFamily* MTServerAppFramework::create_protocol_family()<br>
{<br>
	return new StringProtocolFamily;<br>
}<br>
void MTServerAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)<br>
{<br>
	delete protocol_family;<br>
}<br>
//接收客户端请求<br>
bool MTServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &amp;detach_protocol)<br>
{<br>
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol-&gt;get_protocol_header();<br>
	switch(header-&gt;get_protocol_type())<br>
	{<br>
	case PROTOCOL_STRING:<br>
		{<br>
			StringProtocol* string_protocol = (StringProtocol*)protocol;<br>
			string &amp;recv_string = string_protocol-&gt;get_string();<br>
			SLOG_INFO("thread[ID=%d] receive string protocol from fd=%d. receive data:[%s], length=%d", get_thread_id(), socket_handle, recv_string.c_str(), recv_string.size());<br>
<br>
			StringProtocol* resp_protocol = (StringProtocol*)((DefaultProtocolFamily*)get_protocol_family())-&gt;create_protocol(PROTOCOL_STRING);<br>
			string send_string = "server receive data:";<br>
			send_string += recv_string;<br>
			((StringProtocol*)resp_protocol)-&gt;set_string(send_string);<br>
			send_protocol(socket_handle, resp_protocol);<br>
		}<br>
		break;<br>
	default:<br>
		SLOG_WARN("receive undefine protocol. ignore it.");<br>
		return false;<br>
	}<br>
<br>
	return false;<br>
}<br>
</code></pre>

3. 编译运行<br>
<pre><code>make<br>
./test_server_mt<br>
</code></pre>

<br><br>