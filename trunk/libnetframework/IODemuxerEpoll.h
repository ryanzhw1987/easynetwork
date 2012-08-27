#ifndef _LIB_IO_DEMUXER_EPOLL_H_20120613_LIUYONGJIN
#define	_LIB_IO_DEMUXER_EPOLL_H_20120613_LIUYONGJIN
#include "IODemuxer.h"

#include <sys/epoll.h>

#include <map>
#include <vector>
#include <list>
using std::map;
using std::vector;
using std::list;

using std::pair;
using std::make_pair;

//epoll io��·����
class EventInfo;
typedef map<int, EventInfo*> EVENT_MAP;

class EpollDemuxer:public IODemuxer
{
public: //��д���ി�麯��
    int register_event(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler);
	int unregister_event(int fd);
	int run_loop();
    void exit();
public:

    //max_events: ���������¼���
    //et_mode:�Ƿ�ʹ��ETģʽ. 1ʹ��ETģʽ. 0ʹ��LTģʽ;
	EpollDemuxer(unsigned int max_events=4096, unsigned int et_mode=1);
	~EpollDemuxer();
private:
	int m_epoll_fd;
	int m_et_mode;
	int m_max_events;
	struct epoll_event *m_events;
	EventInfo *m_event_info;
	vector<EventInfo *> m_free_event_info;
	EVENT_MAP m_using_event_info;

	list<EventInfo *> m_timer_list;//ʱ���¼�
	vector<EventInfo *> m_free_timer;

	bool m_exit;
};


#endif //_LIB_IO_DEMUXER_EPOLL_H_20120613_LIUYONGJIN