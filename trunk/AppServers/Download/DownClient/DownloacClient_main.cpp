#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "TaskManager.h"
#include "SocketManager.h"
#include "IODemuxerEpoll.h"
#include "DownloadProtocol.h"
#include "DownloadManager.h"

int main()
{
	SLOG_INIT_WITH_CONFIG("./config/slog.config");

	DownloadThreadPool download_pool(5);
	download_pool.start();

	EpollDemuxer io_demuxer;
	DownloadProtocolFamily protocol_family;
	SocketManager socket_manager;
	TaskManager task_manager(&io_demuxer, &protocol_family, &socket_manager);
	task_manager.set_download_pool(&download_pool);
	task_manager.start();

	string file_name="test.flv";
	task_manager.add_task(file_name);

	EpollDemuxer main_io_demuxer;
	main_io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}



