#include "IODemuxerEpoll.h"
#include "EventHandler.h"

#include "slog.h"

class TimerHandler:public EventHandler
{
public:
	TimerHandler(IODemuxer *demuxer):m_demuxer(demuxer){;}
	HANDLE_RESULT on_timeout(int fd)
	{
		SLOG_DEBUG("timer timeout...");
		m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	
		return HANDLE_OK;
	}
private:
	IODemuxer *m_demuxer;
};


int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	EpollDemuxer io_server;
	
	//register timer
	TimerHandler timer(&io_server);	
	io_server.register_event(-1, EVENT_INVALID, 3000, &timer);	

	io_server.run_loop();

	SLOG_UNINIT();
}

