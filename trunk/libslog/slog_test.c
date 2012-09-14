#include "slog.h"
#include <stdio.h>
#include <string.h>

int main()
{
	//SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 1);

	SLOG_INIT_WITH_CONFIG("./config/slog.config");
	SLOG_TRACE("....trace....");
	SLOG_DEBUG("....debug....");
	SLOG_WARN("....warn....");
	SLOG_ERROR("....error....");

	int i;
	char buf[5120];
	memset(buf, 'c', 5118);
	buf[5119]=0;
	while(1)
		SLOG_WARN("%s", buf);

	SLOG_ERROR("....error....");

	SLOG_UNINIT();
	return 0;
}
