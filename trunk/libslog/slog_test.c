#include "slog.h"
#include "stdio.h"
int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 1);

	SLOG_TRACE("....trace....");
	SLOG_DEBUG("....debug....");
    SLOG_WARN("....warn....");
	SLOG_ERROR("....error....");

    SLOG_UNINIT();
	return 0;
}
