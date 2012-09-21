#include "slog.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

#include "IOBuffer.h"

//应用层每次写入log最大长度
#define BUFSIZE 1024

static pthread_mutex_t g_slog_mutex;
static int g_mutex_is_inited = 0;
#define SLOG_LOCK()	pthread_mutex_lock(&g_slog_mutex)
#define SLOG_UNLOCK()	pthread_mutex_unlock(&g_slog_mutex)
static IOBuffer *g_io_buffer = NULL;

//slog.conf样例
/*
 * ### log级别
 * slog_level=DEBUG
 * ### log文件名
 * slog_log_name=../log/server.log
 * ### log 文件最大大小(单位M)
 * slog_log_maxsize=10M
 * ### log文件最多个数
 * slog_log_maxcount=10
*/
typedef struct _slog_setting_
{
	LOG_LEVEL slog_level;	//log的级别
	char slog_name[128];	//输出文件名
	FILE* slog_file;		//当前打开到log文件
	int max_size;			//每个log数据文件到最大大小(单位M)
	int max_count;			//log文件到个数
	int max_cur_index;		//当前最大文件索引号
}SlogSetting;
static SlogSetting g_slog_setting={SLOG_LEVEL_DEBUG, "", NULL, 10, 10, 0};

//打印时间
static void print_time(char *buf)
{
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(buf,"%d-%02d-%02d-%02d:%02d:%02d",t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

static void move_log_file()
{
	if(g_slog_setting.slog_name[0] == '\0' || g_slog_setting.slog_file==stdout)
		return ;

	long int size = ftell(g_slog_setting.slog_file);
	if(size/(1024*1024) < g_slog_setting.max_size)
		return ;

	fclose(g_slog_setting.slog_file);

	int i=g_slog_setting.max_cur_index;
	char old_log_name[128],new_log_name[128];
	while(i > 0)
	{
		snprintf(old_log_name, 128, "%s.%d", g_slog_setting.slog_name, i);
		snprintf(new_log_name, 128, "%s.%d", g_slog_setting.slog_name, i+1);
		if(i == g_slog_setting.max_count)
			remove(old_log_name);
		else
			rename(old_log_name, new_log_name);
		--i;
	}

	snprintf(old_log_name, 128, "%s", g_slog_setting.slog_name);
	snprintf(new_log_name, 128, "%s.%d", g_slog_setting.slog_name, i+1);
	rename(old_log_name, new_log_name);

	if(g_slog_setting.max_cur_index < g_slog_setting.max_count)
		++g_slog_setting.max_cur_index;

	g_slog_setting.slog_file = fopen(old_log_name, "w");
}

//格式化输入内容到buf
static void format_buffer(char *buf, int buf_size, const char *log_level, const char* fmt, va_list* args)
{
	int len;
	if(buf==NULL || buf_size<100)  //打印时间格式需要预留空间
		return ;

	print_time(buf);
	if(log_level != NULL)
		sprintf(buf+strlen(buf),"[%s]", log_level);
	len = strlen(buf);
	vsnprintf(buf+len, buf_size-len, fmt, *args);
}

//输出buf的内容(以'\0'结束)
static void outpub_buffer(const char *buf)
{
	//将buf内容写入到IOBuffer

	return;
}


void SLOG_INIT(LOG_LEVEL log_level, const char* log_file, int append)
{
    if(log_level<SLOG_LEVEL_TRACE || log_level>SLOG_LEVEL_NOLOG)
		log_level = SLOG_LEVEL_DEBUG;

    g_slog_setting.slog_level = log_level;

    g_slog_setting.slog_file = stdout;
    if(log_file!=NULL && strlen(log_file)>0)
    {
        FILE* temp;
        if(append != 0)
            temp = fopen(log_file, "a");
        else
            temp = fopen(log_file, "w");
        if(temp != NULL)
        {
        	strcpy(g_slog_setting.slog_name, log_file);
        	g_slog_setting.slog_file = temp;
        }
    }

    char buf[1024];
    print_time(buf);
    sprintf(buf+strlen(buf), "[SLOG INIT] successful.level:%d, output file:%s, append mode:%d"
    		, g_slog_setting.slog_level
    		, log_file==NULL?"NULL(use screen)":log_file, append);
    fprintf(g_slog_setting.slog_file, "%s\n", buf);

    if(g_mutex_is_inited == 0)
    {
    	g_mutex_is_inited = 1;
    	pthread_mutex_init(&g_slog_mutex, NULL);
    }
}

void SLOG_INIT_WITH_CONFIG(const char* config_file)
{
	if(g_mutex_is_inited == 0)
	{
		g_mutex_is_inited = 1;
		pthread_mutex_init(&g_slog_mutex, NULL);
	}

	SLOG_LOCK();

	char buf[1024];
	FILE* file = NULL;
	if(config_file == NULL || strlen(config_file)==0)
		printf("no config file is set.\n");
	else if((file = fopen(config_file, "r"))==NULL)
		printf("can't open config file:%s\n", config_file);
	else
	{

		while(fgets(buf,1024,file) != NULL)
		{
			int len = strlen(buf);
			int i=0;
			while(i<len && buf[i]==' ')
				++i;
			if(buf[i]=='\n' || buf[i]=='#' || buf[i]=='/')	//属于空行或者注释
				continue;
			char *key = strtok(&buf[i], "=");
			if(key == NULL)
				continue;
			char *value = strtok(NULL, "=");
			if(value == NULL)
				continue;

			if(strcmp(key, "slog_level")==0)  //slog_level
			{
				char str[30];
				sscanf(value, "%s", str);
				if(strcmp(str, "TRACE")==0)
					g_slog_setting.slog_level = SLOG_LEVEL_TRACE;
				else if(strcmp(str, "DEBUG")==0)
					g_slog_setting.slog_level = SLOG_LEVEL_DEBUG;
				else if(strcmp(str, "INFO")==0)
					g_slog_setting.slog_level = SLOG_LEVEL_INFO;
				else if(strcmp(str, "WARN")==0)
					g_slog_setting.slog_level = SLOG_LEVEL_WARN;
				else if(strcmp(str, "ERROR")==0)
					g_slog_setting.slog_level = SLOG_LEVEL_ERROR;
			}
			else if(strcmp(key, "slog_log_name")==0)
			{
				char log_filename[128];
				sscanf(value, "%s", log_filename);
				if(g_slog_setting.slog_name[0] == '\0')
				{
					g_slog_setting.slog_file = fopen(log_filename, "w");
					if(g_slog_setting.slog_file != NULL)
						strcpy(g_slog_setting.slog_name, log_filename);
					else
					{
						g_slog_setting.slog_file = stdout;
						printf("can't open log file:%s and use screen\n", log_filename);
					}
				}
			}
			else if(strcmp(key, "slog_log_maxsize")==0)
			{
				int max_size;
				sscanf(value, "%d", &max_size);
				g_slog_setting.max_size = max_size;
			}
			else if(strcmp(key, "slog_log_maxcount")==0)
			{
				int max_count;
				sscanf(value, "%d", &max_count);
				g_slog_setting.max_count = max_count;
			}
		}

	}

	print_time(buf);
	sprintf(buf+strlen(buf),"[init slog whit config file]successful.log_level=%d, max_log_size=%d(M), max_log_count=%d."
			, g_slog_setting.slog_level
			, g_slog_setting.max_size
			, g_slog_setting.max_count);

	fprintf(g_slog_setting.slog_file, "%s\n", buf);

	SLOG_UNLOCK();
}

void SLOG_UNINIT()
{
    char buf[1024];
    print_time(buf);
    sprintf(buf+strlen(buf), "[SLOG UNINIT] successful.");
    fprintf(g_slog_setting.slog_file, "%s\n", buf);

    if(g_slog_setting.slog_file!=NULL && g_slog_setting.slog_file!=stdout)
        fclose(g_slog_setting.slog_file);

    pthread_mutex_destroy(&g_slog_mutex);
}


void slog_trace(const char* fmt, ...)
{
    if(SLOG_LEVEL_TRACE >= g_slog_setting.slog_level)
    {
    	char buf[BUFSIZE];
    	va_list args;

    	//格式化
		va_start(args, fmt);
		format_buffer(buf, BUFSIZE, "TRACE", fmt, &args);
		va_end(args);
		//输出
		output_buffer(buf);
    }
}


void slog_debug(const char* fmt, ...)
{
    if(SLOG_LEVEL_DEBUG >= g_slog_setting.slog_level)
    {
    	char buf[BUFSIZE];
		va_list args;

		//格式化
		va_start(args, fmt);
		format_buffer(buf, BUFSIZE, "DEBUG", fmt, &args);
		va_end(args);
		//输出
		output_buffer(buf);
    }
}

void slog_info(const char* fmt,  ...)
{
    if(SLOG_LEVEL_INFO >= g_slog_setting.slog_level)
    {
    	char buf[BUFSIZE];
		va_list args;

		//格式化
		va_start(args, fmt);
		format_buffer(buf, BUFSIZE, "INFO", fmt, &args);
		va_end(args);
		//输出
		output_buffer(buf);
    }
}
void slog_warn(const char* fmt,  ...)
{
    if(SLOG_LEVEL_WARN >= g_slog_setting.slog_level)
    {
    	char buf[BUFSIZE];
		va_list args;

		//格式化
		va_start(args, fmt);
		format_buffer(buf, BUFSIZE, "WARN", fmt, &args);
		va_end(args);
		//输出
		output_buffer(buf);
    }
}

void slog_error(const char* fmt, ...)
{
    if(SLOG_LEVEL_ERROR >= g_slog_setting.slog_level)
    {
    	char buf[BUFSIZE];
		va_list args;

		//格式化
		va_start(args, fmt);
		format_buffer(buf, BUFSIZE, "ERROR", fmt, &args);
		va_end(args);
		//输出
		output_buffer(buf);
    }
}

