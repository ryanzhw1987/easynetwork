#include "IODemuxerEpoll.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "slog.h"

static 	unsigned long long get_current_time_ms()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec*1000 + now.tv_usec/1000;
}

/////////////// EpollDemuxer implementation //////////////
class EventInfo
{
public:
	EventInfo(int fd=0, EVENT_TYPE type=EVENT_INVALID, int timeout_ms=0, EventHandler *handler=NULL)
	{
		set(fd, type, timeout_ms, handler);
	}

	void set(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler)
	{
		m_fd = fd;
		m_timeout_ms = timeout_ms;
		m_type = type;
		m_handler = handler;
		m_occur_type = EVENT_INVALID;
		m_occur_timestamp = get_current_time_ms();
	}

public:
	int m_fd;
	EVENT_TYPE m_type;
	int m_timeout_ms;
	EventHandler *m_handler;

	EVENT_TYPE m_occur_type; //发生的事件
	unsigned long long m_occur_timestamp;  //添加/更新的时间.毫秒
};

//epoll io server.
EpollDemuxer::EpollDemuxer(unsigned int max_events/*=4096*/, unsigned int et_mode/*=1*/)
{
	m_exit = false;

	m_epoll_fd = -1;
	if(max_events <= 0)
		return ;

	m_epoll_fd = epoll_create(max_events+1);
	if (m_epoll_fd == -1) 
	{
		SLOG_ERROR("epoll create error. errno=%d", errno);
		return;
	}
	//close on exec
	int flags;
	if ((flags = fcntl(m_epoll_fd, F_GETFD, NULL)) < 0)
	{
		SLOG_ERROR("epoll fcntl get error. errno=%d", errno);
		close(m_epoll_fd);
		m_epoll_fd = -1;
		return;
	}
	if (fcntl(m_epoll_fd, F_SETFD, flags|FD_CLOEXEC) == -1)
	{
		SLOG_ERROR("epoll fcntl set error. errno=%d", errno);
		close(m_epoll_fd);
		m_epoll_fd = -1;
		return;
	}

	m_et_mode = et_mode;
	m_max_events = max_events;
	m_events = new struct epoll_event[max_events];
	m_event_info = new EventInfo[max_events];

	int i=0;
	while(i<max_events)
	{
		m_free_event_info.push_back(m_event_info+i);
		++i;
	}
}

EpollDemuxer::~EpollDemuxer()
{
	if(m_epoll_fd > 0)
	{
		close(m_epoll_fd);
		m_epoll_fd = -1;
	}	
	if(m_events != NULL)
	{
		delete[] m_events;
		m_events = NULL;
	}
	if(m_event_info != NULL)
	{
		delete[] m_event_info;
		m_event_info = NULL;
	}
}

int EpollDemuxer::register_event(int fd, EVENT_TYPE type, int timeout_ms, EventHandler *handler)
{
	SLOG_TRACE("register_event. fd=%d, type=%d, timeout_ms=%d.", fd, type, timeout_ms);
	
	if(m_epoll_fd == -1)
	{
		return -1;
	}

	if(handler == NULL)
	{
		return -1;
	}

	if(fd <= 0) //时钟超时事件
	{
		if(timeout_ms <= 0) //timeout_ms必须大于0
			return -1;
			
		//时钟事件		
		EventInfo *event_info = NULL;
		if(m_free_timer.empty())
		{
			event_info = new EventInfo(fd, type, timeout_ms, handler);
		}
		else
		{
			event_info = m_free_timer.back();
			event_info->set(fd, type, timeout_ms, handler);
			m_free_timer.pop_back();
		}
		m_timer_list.push_back(event_info);
	}
	else
	{
		if(m_free_event_info.empty()) //没有空闲
		{
			return -1;
		}

		if((type&(EVENT_READ|EVENT_WRITE)) == 0) //非时钟事件,却没有注册读写事件
		{
			return -1;
		}
		
		int epoll_op = EPOLL_CTL_ADD;
		int flags = 0;
		EventInfo *event_info = NULL;
		EVENT_MAP::iterator it = m_using_event_info.find(fd);
		if(it == m_using_event_info.end()) //没有注册过
		{
			event_info = m_free_event_info.back();
			event_info->set(fd, type, timeout_ms, handler);
			pair<EVENT_MAP::iterator, bool> ret_pair = m_using_event_info.insert(make_pair(fd, event_info));
			if(ret_pair.second == false)
				return -1;
			m_free_event_info.pop_back();

			it = ret_pair.first;
			epoll_op = EPOLL_CTL_ADD;	//添加
		}
		else
		{
			event_info = it->second;
			int old_type = event_info->m_type;
			if((type&EVENT_READ) == 0) //没有读事件,则先过滤掉EVENT_PERSIST标记
				type &= ~EVENT_PERSIST;
			event_info->m_type |= type; //有可能只是读事件增加了EVENT_PERSIST,所以重新设置一次.

			if((old_type&type) != 0) //已经注册过
				return 0;
			epoll_op = EPOLL_CTL_MOD;	//修改
		}
	
		if(event_info->m_type&EVENT_READ)  //添加读
			flags |= EPOLLIN;
		if(event_info->m_type&EVENT_WRITE) //添加写
			flags |= EPOLLOUT;
		if(m_et_mode != 0)
			flags |= EPOLLET;

		struct epoll_event temp;
		temp.data.ptr = (void*)event_info;
		temp.events = flags;
		if(epoll_ctl(m_epoll_fd, epoll_op, fd, &temp) == -1)
		{
			SLOG_ERROR("register event failed. errno=%d", errno);
			m_free_event_info.push_back(it->second);			
			m_using_event_info.erase(it);
			return -1;
		}

		SLOG_TRACE("register_event succ. fd=%d, type=%d, timeout_ms=%d. epoll_op=%s", fd, type, timeout_ms,epoll_op==EPOLL_CTL_ADD?"EPOLL_CTL_ADD":"EPOLL_CTL_MOD");
	}

	return 0;
}

int EpollDemuxer::unregister_event(int fd)
{
	if(m_epoll_fd == -1)
		return -1;

	EVENT_MAP::iterator it = m_using_event_info.find(fd);
	if(it != m_using_event_info.end())
	{
		if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		{
			SLOG_ERROR("delete event from epoll failed. fd:%d, errno:%d", fd, errno);
			return -1;
		}

		m_using_event_info.erase(it);
		m_free_event_info.push_back(it->second);
	}
	return 0;
}

const int WAIT_TIME_MS = 10; //10ms
const EVENT_TYPE EVENT_ERROR   = 0x100;  //发送错误(内部使用)
const EVENT_TYPE EVENT_TIMEOUT = 0x1000; //超时
typedef vector<EventInfo*> OccurEventList;

int EpollDemuxer::run_loop()
{
	int i;
	unsigned long long start_time_ms = 0;
	unsigned long long last_check_time_ms = 0;
	OccurEventList occur_events;

	while(!m_exit)
	{
		int count = epoll_wait(m_epoll_fd, m_events, m_max_events, WAIT_TIME_MS);
		if (count == -1)
		{
			if (errno == EINTR)
				continue;

			SLOG_ERROR("epoll_wait error. errno=%d.", errno);
			break;
		}

		occur_events.clear();
		start_time_ms = get_current_time_ms();
		
		//1. 检查发生的读写事件
		for (i=0; i<count; i++)
		{
			int is_error = 0;
			EventInfo *event_info = (EventInfo *)m_events[i].data.ptr;
			event_info->m_occur_type = EVENT_INVALID;

			if(m_events[i].events & (EPOLLHUP|EPOLLERR))
			{
				event_info->m_occur_type = EVENT_ERROR;
				is_error = 1;
			}
			if(!is_error && (m_events[i].events&EPOLLIN))
			{
				event_info->m_occur_type |= EVENT_READ;
				if((event_info->m_type&EVENT_PERSIST) == 0)
					event_info->m_type &= ~EVENT_READ;
			}
			if(!is_error && (m_events[i].events&EPOLLOUT))
			{
				event_info->m_occur_type |= EVENT_WRITE;
				event_info->m_type &= ~EVENT_WRITE;
			}

			if(event_info->m_occur_type == EVENT_INVALID)
			{
				SLOG_WARN("Unknow event(%d) happend on fd=%d", m_events[i].events, event_info->m_fd);
				continue;
			}

			//错误或者发生所有注册的事件.删除注册的事件
			if((event_info->m_occur_type&EVENT_ERROR) || (event_info->m_type&(EVENT_WRITE|EVENT_READ))==EVENT_INVALID)
			{
				SLOG_TRACE("remove fd=%d from epoll because of error or unpersist.", event_info->m_fd);
				epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event_info->m_fd, NULL);
				m_using_event_info.erase(event_info->m_fd);				
				m_free_event_info.push_back(event_info);
			}

			event_info->m_occur_timestamp = start_time_ms;
			occur_events.push_back(event_info);
		}

		//检查超时
		if(start_time_ms-last_check_time_ms > 300)
		{
			//2. 检查socket读写超时
			EventInfo *event_info;
			EVENT_MAP::iterator it;
			for(it=m_using_event_info.begin(); it!=m_using_event_info.end(); )
			{
				event_info = (EventInfo *)it->second;
				if(event_info->m_timeout_ms>0 && (start_time_ms-event_info->m_occur_timestamp)>event_info->m_timeout_ms)
				{
					event_info->m_occur_type = EVENT_TIMEOUT;
					occur_events.push_back(event_info);					
					//event_info->m_time_ms = start_time_ms;

					//读写超时. 删除注册的事件
					SLOG_TRACE("remove fd=%d from epoll because of timeout.", event_info->m_fd);
					epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event_info->m_fd, NULL);
					m_using_event_info.erase(it++);
					m_free_event_info.push_back(event_info);
				}
				else
					++it;
			}

			//3. 检查时钟是否超时
			list<EventInfo*>::iterator timer_it;
			for(timer_it=m_timer_list.begin(); timer_it!=m_timer_list.end(); )
			{
				event_info = (EventInfo *)*timer_it;
				if(start_time_ms-event_info->m_occur_timestamp > event_info->m_timeout_ms)
				{
					event_info->m_occur_type = EVENT_TIMEOUT;
					occur_events.push_back(event_info);

					//移除时钟
					if((event_info->m_type&EVENT_PERSIST) == 0)
					{
						m_free_timer.push_back(event_info);
						timer_it = m_timer_list.erase(timer_it);
						continue;
					}
					event_info->m_occur_timestamp = start_time_ms;
				}
				
				++timer_it;
			}

			last_check_time_ms = start_time_ms;
		}

		//处理发生的事件
		if(!occur_events.empty())
		{
			EventInfo *event_info;
			for(i=0; i<occur_events.size(); ++i)
			{
				HANDLE_RESULT handle_result = HANDLE_OK;
				event_info = occur_events[i];
				if(event_info->m_occur_type&EVENT_ERROR)
				{
					event_info->m_handler->on_error(event_info->m_fd);
					continue;
				}
				if(event_info->m_occur_type&EVENT_TIMEOUT)
				{
					event_info->m_handler->on_timeout(event_info->m_fd);
					continue;
				}
				if(event_info->m_occur_type&EVENT_READ)
				{
					handle_result = event_info->m_handler->on_readable(event_info->m_fd);
					if(handle_result == HANDLE_ERROR)
					{
						event_info->m_handler->on_error(event_info->m_fd);
						EVENT_MAP::iterator it = m_using_event_info.find(event_info->m_fd);
						if(it != m_using_event_info.end())
						{
							SLOG_TRACE("remove fd=%d from epoll because read error.", event_info->m_fd);
							epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event_info->m_fd, NULL);
							m_using_event_info.erase(it);
							m_free_event_info.push_back(event_info);							
						}
					}
				}
				if(handle_result==HANDLE_OK && event_info->m_occur_type&EVENT_WRITE)
				{
					handle_result = event_info->m_handler->on_writeabble(event_info->m_fd);
					if(handle_result == HANDLE_ERROR)
					{
						event_info->m_handler->on_error(event_info->m_fd);
						EVENT_MAP::iterator it = m_using_event_info.find(event_info->m_fd);
						if(it != m_using_event_info.end())
						{
							SLOG_TRACE("remove fd=%d from epoll because write error.", event_info->m_fd);
							epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event_info->m_fd, NULL);
							m_using_event_info.erase(it);
							m_free_event_info.push_back(event_info);
						}
					}
				}
			}
		}

		//处理过程所花费在时间
		int elapsed = get_current_time_ms() - start_time_ms;
		if(elapsed > 100)
		{
			SLOG_WARN("epoll check events use :%d ms", elapsed);
		}
		else
		{
			SLOG_TRACE("epoll check events use :%d ms", elapsed);
		}
	}//while

	SLOG_INFO("epoll demuxer exit.");
}

void EpollDemuxer::exit()
{
	m_exit = true;
}
