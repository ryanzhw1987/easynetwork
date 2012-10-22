#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "IODemuxerEpoll.h"
#include "TaskManager.h"
#include "DownloadManager.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	DownloadThreadPool download_pool(5);
	download_pool.start();

	TaskManager task_manager;
	task_manager.start_instance();
	task_manager.set_download_pool(&download_pool);
	task_manager.start();

	string file_name="test.flv";
	task_manager.add_task(file_name);

	EpollDemuxer main_io_demuxer;
	main_io_demuxer.run_loop();

	task_manager.stop_instance();
	SLOG_UNINIT();
	return 0;
}



