#ifndef _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_
#define _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

#include "Socket.h"
#include "Protocol.h"
#include "IODemuxer.h"
#include "EventHandler.h"

#include <map>
#include <queue>
using std::queue;
using std::map;
typedef map<SocketHandle, Socket*> SocketMap;
typedef map<SocketHandle, queue<Protocol*> > SendTaskMap;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////                                            ////////
////////               SocketMnager��               ////////
////////                                            ////////
////////          1.������������                    ////////
////////          2.�����������˿�                  //////// 
////////          3.�ṩӦ�ò�Э�����ݷ��ͽ���      ////////
////////          4.֪ͨӦ�ò����ӳ�ʱ/����״̬     //////// 
////////                                            ////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
class SocketManager
{
public:
    SocketManager(BlockMode block_mode=NOBLOCK);
    virtual ~SocketManager();
private:
    BlockMode m_block_mode;             //���ӵ�����ģʽ

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////       ���Ӵ�������         //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
    //����ĳ���˿�, �ɹ�����0, ʧ�ܷ���-1;
    int listen(int port);
    //����������������
    virtual SocketHandle create_active_trans_socket(const char *ip, int port);

    //��ӱ�����������(��Ҫ�ɿ�ܵ���)
    virtual int add_passive_trans_socket(const char *peer_ip, SocketHandle socket_handle);
    //ɾ��(��/����)��������,ȡ����������������ϵ�Э��,ͬʱ֪ͨӦ�ò�(��Ҫ�ɿ�ܵ���)
    virtual int delete_trans_socket(SocketHandle socket_handle);
    //���Ҽ�������(��Ҫ�ɿ�ܵ���)
    virtual Socket* find_listen_socket(SocketHandle socket_handle);
    //����(��/����)��������.(��Ҫ�ɿ�ܵ���)
    virtual Socket* find_trans_socket(SocketHandle socket_handle);
protected:
    virtual Socket* new_listen_socket();         //���������listen socket
    virtual EventHandler* new_listen_handler();  //���������listen handler
    virtual Socket* new_trans_socket();          //���������trans socket
    virtual EventHandler* new_trans_handler();   //���������trans handler
	virtual int init_passive_trans_socket(SocketHandle socket_handle, BlockMode block_mode); //����ӵ�trans socket���б�Ҫ�ĳ�ʼ��
private:
    ListenSocket *m_listen_socket;   //����socket
	EventHandler *m_listen_handler;  //����handler
	SocketMap m_trans_sockets_map;   //����socket
	EventHandler *m_trans_handler;   //����handler


////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////       Э�鷢�ʹ���         //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
	//���Э�鵽���Ͷ���.�ɹ�����0.ʧ�ܷ���-1,��Ҫ���д���protocol.
    virtual int send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp);
	//��ȡ�ȴ������д����͵�Э��
    virtual Protocol* get_wait_to_send_protocol(SocketHandle socket_handle);
    //��ȡ�ȴ������д����͵�Э�����
    virtual int get_wait_to_send_protocol_number(SocketHandle socket_handle);
    //ȡ�����д�����Э��,ͬʱ����on_protocol_send_error֪ͨӦ�ò�
    virtual int cancal_wait_to_send_protocol(SocketHandle socket_handle);
private:
    //�����͵�Э�����
    SendTaskMap m_send_tasks_map;


////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   Ӧ�ò���д�¼���Ӧ����   //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
    //////////////////��Ӧ�ò���д ����Э�麯��//////////////////
    virtual int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)=0;
    //////////////////��Ӧ�ò���д Э�鷢�ʹ�������//////////
	virtual int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////��Ӧ�ò���д Э�鷢�ͳɹ�������//////////
	virtual int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;
    //////////////////��Ӧ�ò���д ���Ӵ�������//////////////
    virtual int on_socket_handle_error(SocketHandle socket_handle)=0;
    //////////////////��Ӧ�ò���д ���ӳ�ʱ������//////////////
    virtual int on_socket_handle_timeout(SocketHandle socket_handle)=0;
	//////////////////��Ӧ�ò���д �յ�һ���µ���������////////
	virtual int on_socket_handler_accpet(SocketHandle socket_handle){return 0;}

	//Ӧ�ò���ʹ�õ�io����
	virtual IODemuxer* get_io_demuxer()=0;
	//Ӧ�ò���ʹ�õ�Э����
	virtual ProtocolFamily* get_protocol_family()=0;
};


#endif // _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

