#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "IODemuxerEpoll.h"
#include "TaskManager.h"
#include "DownloadManager.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	DownloadWorkerPool download_pool(5);
	download_pool.start();

	TaskManager task_manager;
	task_manager.set_download_pool(&download_pool);
	task_manager.start();

	while(!task_manager.is_thread_ready())
		;

	string file_name="test.flv";
	task_manager.add_task(file_name);

	EpollDemuxer main_io_demuxer;
	main_io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}



