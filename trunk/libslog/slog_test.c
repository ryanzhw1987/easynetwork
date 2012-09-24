#include "slog.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
	//SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 1);

	SLOG_INIT("./config/slog.config");

	SLOG_TRACE("....trace....");
	SLOG_DEBUG("....debug....");
	SLOG_WARN("....warn....");
	SLOG_ERROR("....error....");

	int i,j;
	char buf[1000];
	memset(buf, 'c', 1000);
	buf[999]=0;

	i=0;
	for(i=0; i<40; ++i)
		for(j=0;j<10000; ++j)
			SLOG_WARN("|i=%d j=%d| %s", i,j,buf);

	SLOG_ERROR("....error....");

	SLOG_UNINIT();
	return 0;
}
