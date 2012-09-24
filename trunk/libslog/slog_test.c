#include "slog.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

void test1()
{
	SLOG_TRACE("....trace....");
	SLOG_DEBUG("....debug....");
	SLOG_WARN("....warn....");
	SLOG_ERROR("....error....");
}

void test2()
{
	char buf[100];
	memset(buf, 'c', 100);
	buf[99]=0;

	int i;
	for(i=0; i<100; ++i)
	{
		sleep(1);
		SLOG_WARN("i=%d %s", i,buf);
	}
}

void test3()
{
	char buf[100];
	memset(buf, 'c', 100);
	buf[99]=0;

	int i,j;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	for(i=0; i<100; ++i)
	{
		for(j=0;j<10000; ++j)
			SLOG_WARN("%s", buf);
	}
	gettimeofday(&end, NULL);
	printf("time:%d",  end.tv_sec-start.tv_sec);
}


int main()
{
	//SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 0);
	//SLOG_INIT(SLOG_LEVEL_DEBUG, "a.txt", 1);

	SLOG_INIT("./config/slog.config");

	//test1();
	//test2();
	test3();

	SLOG_UNINIT();
	return 0;
}
