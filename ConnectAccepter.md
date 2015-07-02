## ConnectAccepter ##

ConnectAccepter是链接接收器纯虚类，用来控制如何接收一个新的链接请求（SocketHanle）。只有一个纯虚方法，实现不同的accept方法可控制对链接的不同分配策略：
```
//链接请求接收器
class ConnectAccepter
{
public:
	virtual ~ConnectAccepter(){}
	virtual bool accept(SocketHandle trans_fd) = 0;
};
```

ConnectAccepter的具体子类通过实现accept方法完成新链接的处理，比如NetInterface类实现accept为接收一个新链接，并把该链接注册到IODemuxer中进行监听。而ConnectThreadPool则实现为将该新链接添加到ConnectThread线程队列，由线程处理该链接（线程接收到该链接时添加到IODexmer中进行监听）。