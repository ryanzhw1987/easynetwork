# 前言 #
> 本节介绍一个使用EasyNetwork框架搭建的多任务/多线程下载服务器。该Donwload server/client比较简单,没有做过多的优化和检查工作来保证服务的稳定性。因为我们的focus是“怎么使用框架搭建Download server"，而不是“Download server"。当然，现在我们只要把精力放在应用层的实现逻辑就可以了。

> 整个服务器/客户端的工作流程大概如下：客户端向服务端发送请求文件大小的协议，收到服务端回复之后根据文件大小进行分片；客户端向服务端请求每个分片的数据；服务端收到请求分片的任务后，按照分片在文件中的起始位置和大小读取数据并发送给客户端。

> 另外，在协议族中创建和销毁协议对象时我们使用来MemCache内存管理模块来控制内存的分配和释放，从而避免产生内存碎片。具体见下面的协议族定义。

> 好吧，进入主题，本次新闻联播的主要内容有：
    * [MThreadDownload#1.\_Download\_ProtocolFamily](MThreadDownload#1._Download_ProtocolFamily.md)
    * [MThreadDownload#2.\_Download\_Server](MThreadDownload#2._Download_Server.md)
    * [MThreadDownload#3.\_Download\_Client](MThreadDownload#3._Download_Client.md)
    * [MThreadDownload#4.\_Ending](MThreadDownload#4._Ending.md)

# 0. 前期准备 #
> 如果要运行例子，需要在/data/目录下存放一个test.flv文件(几十M的文件即可)，例子中下载的就是该文件。

# 1. Download ProtocolFamily #
> 搭建框架之前我们先定义好协议和协议族。我们利用框架的DefaultProtocol来定义我们下载服务器需要用到的4个协议:RequestSize, RespondSize, RequestData, RespondData.(具体见源码)

> a) RequestSize请求文件大小
```
 //DownloadProtocolFamily.h
 //请求文件大小
 class RequestSize: public Protocol
 {
 public://实现protocol的接口
 	//协议的描述信息
 	const char* details(){return "RequestSize";}
 	//编码协议体数据到byte_buffer,成功返回true,失败返回false.
 	bool encode_body(ByteBuffer *byte_buffer);
 	//解码大小为size的协议体数据buf.成功返回true,失败返回false.
 	bool decode_body(const char *buf, int size);
 public:
 	RequestSize(){}
 	RequestSize(string &filename)
 	{
 		m_file_name = filename;
 	}
 	void assign(const string &file_name){m_file_name = file_name;}
 	const string& get_file_name(){return m_file_name;}
 private:
 	string m_file_name;
 };
 
 //DownloadProtocolFamily.cpp
 //编码协议体数据到byte_buffer,成功返回true,失败返回false.
 bool RequestSize::encode_body(ByteBuffer *byte_buffer)
 {
 	int len = 0;
 	//file name
 	ENCODE_STRING(m_file_name);
 
 	return true;
 }
 
 //解码大小为size的协议体数据buf.成功返回true,失败返回false.
 bool RequestSize::decode_body(const char* buf, int size)
 {
 	int len = 0;
 	//file name
 	DECODE_STRING(m_file_name);
 
 	return true;
 }
```
> 在编解码中我们使用来ENCODE\_STRING和DECODE\_STRING两个宏定义以便快速的编解码相关字段。这些宏定义在库Protocol.h。

> b) RespondSize 回复文件大小（略）

> c) RequestData 请求文件数据（略）

> d) RespondData 回复文件数据（略）

> e) DownloadProtocolFamily下载协议族
> 最后需要定义一个协议族类用来创建和销毁协议：
```
 //DownloadProtocolFamily.h
 class DownloadProtocolFamily:public DefaultProtocolFamily
 {
 public:
 	Protocol* create_protocol_by_header(ProtocolHeader *header);
 	void destroy_protocol(Protocol *protocol);
 private:
 	//memory cache:
 	MemCache<RequestSize> m_request_size_memcache;
 	MemCache<RespondSize> m_respond_size_memcache;
 	MemCache<RequestData> m_request_data_memcache;
 	MemCache<RespondData> m_respond_data_memcache;
 };
 
 //DownloadProtocolFamily.cpp
 /////////////////////////////////////////////////////////////////////////////////
 Protocol* DownloadProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
 {
 	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
 	Protocol *protocol = NULL;
 	switch(protocol_type)
 	{
 	case PROTOCOL_REQUEST_SIZE:
 		protocol = (Protocol*)m_request_size_memcache.Alloc();
 		SLOG_DEBUG("create RequestSize[%x] from m_request_size_memcache", protocol);
 		break;
 	case PROTOCOL_RESPOND_SIZE:
 		protocol = (Protocol*)m_respond_size_memcache.Alloc();
 		SLOG_DEBUG("create RespondSize[%x] from m_respond_size_memcache", protocol);
 		break;
 	case PROTOCOL_REQUEST_DATA:
 		protocol = (Protocol*)m_request_data_memcache.Alloc();
 		SLOG_DEBUG("create RequestData[%x] from m_request_data_memcache", protocol);
 		break;
 	case PROTOCOL_RESPOND_DATA:
 		protocol = (Protocol*)m_respond_data_memcache.Alloc();
 		SLOG_DEBUG("create RespondData[%x] from m_respond_data_memcache", protocol);
 		break;
 	default:
 		break;
 	}
 
 	return protocol;
 }
 
 void DownloadProtocolFamily::destroy_protocol(Protocol* protocol)
 {
 	if(protocol == NULL)
 		return;
 	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
 	switch(header->get_protocol_type())
 	{
 	case PROTOCOL_REQUEST_SIZE:
 	{
 		SLOG_DEBUG("free DownloadRequest[%x] to m_request_size_memcache", protocol);
 		RequestSize *temp_protocol = (RequestSize*)protocol;
 		m_request_size_memcache.Free(temp_protocol);
 		break;
 	}
 	case PROTOCOL_RESPOND_SIZE:
 	{
 		SLOG_DEBUG("free RespondSize[%x] to m_respond_size_memcache", protocol);
 		RespondSize *temp_protocol = (RespondSize*)protocol;
 		m_respond_size_memcache.Free(temp_protocol);
 		break;
 	}
 	case PROTOCOL_REQUEST_DATA:
 	{
 		SLOG_DEBUG("free RequestData[%x] to m_request_data_memcache", protocol);
 		RequestData *temp_protocol = (RequestData*)protocol;
 		m_request_data_memcache.Free(temp_protocol);
 		break;
 	}
 	case PROTOCOL_RESPOND_DATA:
 	{
 		SLOG_DEBUG("free RespondData[%x] to m_respond_data_memcache", protocol);
 		RespondData *temp_protocol = (RespondData*)protocol;
 		m_respond_data_memcache.Free(temp_protocol);
 		break;
 	}
 	default:
 		break;
 	}
 }
```
> 协议族需要实现create\_protocol\_by\_header和destroy\_protocol两个接口方法，用来创建/销毁具体的协议。

> 另外代码中使用了MemCache模块来控制内存空间到分配。协议的创建和销毁在实际项目中是非常频繁的操作，通过使用内存管理模块可以避免产生内存碎片而导致程序效率降低的问题。

# 2. Download Server #
> 这一节开始我们介绍多任务/多线程下载服务。服务器的框架大体如下：
    * 服务器收到一个新链接时，将该链接分配到某个线程，由该线程处理在该链接上的请求。
    * 某个线程收到RequestSize协议时，返回指定的file\_name文件的大小给客户端(回复RespondSize协议)。
    * 某个线程收到RequestData协议请求时，按协议指定的数据偏移位置start\_pos和分片大小将分片数据返回给客户端(回复RespondData协议)。

> (1) 利用enetlib工具生成框架代码类
```
enetlib -m DownloadServer -main
```
> 生成了3个文件：
```
DownloadServer.cpp 
DownloadServer.h
DownloadServerMain.cpp 
```
> ok, 我们的下载服务器完成了, 收工....开玩笑的, 还需要添加业务逻辑层代码呢:

> (2) DownloadServer 线程

> DownloadServer从ConnectThread继承，实际是一个NetInterface的实例，因此需要实现NetInterface的接口方法。
```
////创建/消毁协议族
ProtocolFamily* DownloadServer::create_protocol_family()
{ 
	return new DownloadProtocolFamily;
}
void DownloadServer::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
} 
```

> 接下来当然是处理请求协议,来看看我们是如何做到的:当然是实现on\_recv\_protocol接口了.
```
bool DownloadServer::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DownloadProtocolFamily* protocol_family = (DownloadProtocolFamily*)get_protocol_family();
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_REQUEST_SIZE:
	{
		RequestSize *temp_protocol = (RequestSize*)protocol;
		const string file_name = temp_protocol->get_file_name();
		SLOG_INFO("receive <RequestSize:file=%s>", file_name.c_str());

		string path="/data/";
		path += file_name;

		//get file size
		unsigned long long file_size=0;
		FILE *fp = fopen(path.c_str(), "r");
		if(fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			file_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fclose(fp);
		}

		RespondSize* resp_protocol = (RespondSize*)protocol_family->create_protocol(PROTOCOL_RESPOND_SIZE);
		if(resp_protocol)
		{
			resp_protocol->assign(file_name, file_size);
			send_protocol(socket_handle, resp_protocol);
		}
		else
			SLOG_ERROR("create RespondSize protocol failed.");

		break;
	}
	case PROTOCOL_REQUEST_DATA:
	{
		RequestData *temp_protocol = (RequestData*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long start_pos = temp_protocol->get_start_pos();
		unsigned int size = temp_protocol->get_size();
		SLOG_INFO("receive <RequestData: file=%s, start_pos=%ld, size=%d>", file_name.c_str(), start_pos, size);

		string path="/data/";
		path += file_name;
		FILE *fp = fopen(path.c_str(), "r");
		if(fp != NULL)
		{
			RespondData* resp_protocol = (RespondData*)protocol_family->create_protocol(PROTOCOL_RESPOND_DATA);
			resp_protocol->assign(file_name, start_pos, size);

			DefaultProtocolHeader *header = (DefaultProtocolHeader *)resp_protocol->get_protocol_header();
			int header_length = header->get_header_length();
			ByteBuffer *byte_buffer = new ByteBuffer;
			//1. 预留协议头空间
			byte_buffer->get_append_buffer(header_length);
			byte_buffer->set_append_size(header_length);
			//2. 编码协议体数据
			resp_protocol->encode_body(byte_buffer);
			//3. 添加数据
			char *data_buffer = byte_buffer->get_append_buffer(size);
			fseek(fp, start_pos, SEEK_SET);
			fread(data_buffer, 1, size, fp);
			fclose(fp);
			byte_buffer->set_append_size(size);
			//4. 编码协议头
			int body_length = byte_buffer->size()-header_length;
			char *header_buffer = byte_buffer->get_data(0, header_length);
			header->encode(header_buffer, body_length);
			//5. 发送协议
			resp_protocol->attach_raw_data(byte_buffer);
			send_protocol(socket_handle, resp_protocol);
		}
		else
		{
			SLOG_ERROR("can't open file=%s", file_name.c_str());
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		return false;
	}
 
	return true;
}
```
> 嗯，到现在我们的下载服务器完成了。收工，洗洗睡了... 哦，忘记还有客户端的代码，顺便提供一下，嘿嘿...


# 3. Download Client #
> 实际上客户端的东西跟下载服务器没有太大的关系了，但是你总得对服务进行测试吧，人不能太懒了。更重要的是你不能说你利用框架搭了一个下载服务器，就说明它是OK的吧。（嗯，好，我们大家一起来验证一下...代码如下...我发现,客户端反而复杂了一点..）

> 客户端所做的事情如下：
    * TaskManager模块接收一个任务，向服务端请求该任务的文件大小；
    * 收到服务端的回复后，TaskManager模块将文件划分成若干数据块，将每个分片分配到下载线程DownloadThread模块进行下载；
    * 下载完所有分片后合并成一个完整的文件（请再次原谅我没有做这一步）；

> (1) 利用enetlib工具生成框架代码类
```
enetlib -s TaskManager
enetlib -s DownloadManager
```

> (2) TaskManager任务管理器

> 任务管理器其实是一个线程，用来接收任务，向服务器请求文件大小，然后将文件划分成分片任务提交给下载线程池，由下载线程池完成分片的下载任务。任务管理器从Netnterface派生，实现协议的发送和接收。

> 我们需要稍微修改一下TaskManager，使其从PipeThread派生并实现Thread相应的接口方法：
```
class TaskManager: public NetInterface, public PipeThread<string>
{
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现接口:注册管道事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);
...
...
};
```
> 其他见源码。

> (3) DownloadManager下载管理器

> 下载管理线程要做的事情主要有：
    * 接收TaskManager发送过来的下载任务；
    * 从队列里面获取一个任务进行下载；
    * 下载完后跳到step 2. （即每个线程一次只下载一个分片任务）

> 同TaskManager一样, 稍微修改一下使其从PipeThread派生并实现Thread相应的接口方法:
```
class DownloadThread:public NetInterface, public PipeThread<DownloadTask* >
{
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现接口:注册管道事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);
...
...
};
```

> (a) 接收任务

> 下载线程收到任务时，会收到on\_notify\_add\_task的通知（怎么收呢？呃，框架帮你做了），在这里面我们发送一个下载任务，即发送一个RequestData请求：
```
bool DownloadThread::on_notify_add_task()
{
	SLOG_INFO("Thread[ID=%d,Addr=%x] on_notify_add_task", get_thread_id(), this);
	return send_download_task();
}

bool DownloadThread::send_download_task(SocketHandle socket_handle)
{
	if(m_is_downloading)  //一次只执行一个下载任务
		return false;

	DownloadTask* download_task = NULL;
	if(get_task(download_task) && send_download_task(socket_handle, download_task))
		m_is_downloading = true;
	return true;
}
```

> (b) 接收数据

> 同样，还是在on\_recv\_protocol里面处理啦，都不用说的啦... 在接收完数据后，会调用send\_download\_task来发送下一个分片任务的请求：
```
bool DownloadThread::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_RESPOND_DATA:
	{
		RespondData* temp_protocol = (RespondData*)protocol;
		const string file_name = temp_protocol->get_file_name();
		unsigned long long start_pos = temp_protocol->get_start_pos();
		unsigned int size = temp_protocol->get_size();
		const char *data = temp_protocol->get_data();

		ostringstream temp;
		temp<<file_name<<"_"<<start_pos;
		DownloadMap::iterator it = m_downloading_task.find(temp.str());
		if(it == m_downloading_task.end())
		{
			SLOG_WARN("receive RespondData[file=%s, start=%ld], but can't not find task", file_name.c_str(), start_pos);
		}
		else
		{
			DownloadTask* task = it->second;
			SLOG_INFO("receive RespondData[ID=%d, fd=%d, file=%s, index=%d, start_pos=%ld, size=%d]", get_thread_id(), socket_handle, file_name.c_str(), task->task_index, task->start_pos, task->size);
			if(task->fp == NULL)
			{
				char buf[128];
				sprintf(buf, "./download_data/%s.%d", task->file_name.c_str(), task->task_index);
				task->fp = fopen(buf, "wb");
			}

			fwrite(data, 1, size, task->fp);
			task->down_size += size;
			if(task->down_size == task->size)
			{
				SLOG_INFO("finish download[ID=%d, fd=%d, file=%s, index=%d]", get_thread_id(), socket_handle, file_name.c_str(), task->task_index);
				fclose(task->fp);
				delete task;
				m_downloading_task.erase(it);
				m_is_downloading = false;
				send_download_task(socket_handle);
			}
		}
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return true;
}
```

> 到此我们的下载客户端已经完成了，具体例子详见源码。

# 4. Ending #
> 本章快速地介绍了用EasyNetwork框架快速地搭建一个下载服务器和下载客户端...感觉还不错吧。贴几行log展示一下下载的过程吧，各个分片由不同的线程下载：
```
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=4, fd=29, file=test.flv, index=4]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=4, fd=29, file=test.flv, index=9]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=0, fd=25, file=test.flv, index=0]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=3, fd=28, file=test.flv, index=3]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=1, fd=26, file=test.flv, index=1]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=2, fd=27, file=test.flv, index=2]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=4, fd=29, file=test.flv, index=14]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=0, fd=25, file=test.flv, index=5]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=0, fd=25, file=test.flv, index=10]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=3, fd=28, file=test.flv, index=8]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=4, fd=29, file=test.flv, index=19]
2012-09-18-14:13:00 INFO [DownClient/DownloadManager.cpp:on_recv_protocol(68)] ... finish download[ID=1, fd=26, file=test.flv, index=6]
```