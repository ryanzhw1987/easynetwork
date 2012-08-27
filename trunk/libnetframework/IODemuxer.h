#ifndef _LIB_IO_DEMUXER_H_20120613_LIUYONGJIN
#define	_LIB_IO_DEMUXER_H_20120613_LIUYONGJIN

#include <stdio.h>
#include "EventHandler.h"

//����io��·���������
class IODemuxer
{
public:
    virtual ~IODemuxer(){}
    //ע���¼�.
    //fd: socket������. ��ΪС��0ʱ��ʾע�����ʱ�ӳ�ʱ�¼�,��ʱtimeout_ms�������0,�����޷�ע��;
    //type: fd�Ķ�д�¼�.fd����0ʱ��Ч.
    //timeout_ms: fd��д�¼���ʱʱ��.����ʱ�ӳ�ʱʱ��.��λ����.
    //handler: �¼���Ӧ������.
    //����ֵ:-1ʧ��. 0�ɹ�;
	virtual int register_event(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler)=0;

    //ע��fd���¼�. fd����0ʱ��Ч.0�ɹ�, -1ʧ��
	virtual int unregister_event(int fd)=0;

	//ѭ���ȴ�/�����¼�
	virtual int run_loop()=0;
	virtual void exit()=0;
};

#endif //_LIB_IO_DEMUXER_H_20120613_LIUYONGJIN

