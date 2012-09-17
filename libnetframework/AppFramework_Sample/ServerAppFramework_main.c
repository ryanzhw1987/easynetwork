#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "ServerAppFramework.h"

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	ServerAppFramework app_framework;
	if(app_framework.listen(3010) == -1)
	{
	    return -1;
	}

	IODemuxer *io_demuxer = app_framework.get_io_demuxer();
	TimerHandler timer(io_demuxer);	
	io_demuxer->register_event(-1, EVENT_INVALID, 3000, &timer);	

	io_demuxer->run_loop();

	SLOG_UNINIT();
	return 0;
}



