#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "TaskManager.h"
#include "SocketManager.h"
#include "IODemuxerEpoll.h"
#include "DownloadManager.h"

int main()
{
	SLOG_INIT_WITH_CONFIG("./config/slog.config");

	DownloadThreadPool download_pool(5);
	download_pool.start();

	TaskManager task_manager;
	task_manager.init_instance();
	task_manager.set_download_pool(&download_pool);
	task_manager.start();

	string file_name="test.flv";
	task_manager.add_task(file_name);

	EpollDemuxer main_io_demuxer;
	main_io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}



