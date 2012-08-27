#include "slog.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static int g_cur_level = 4;
static FILE* g_outfile = NULL;

void print_time(char *buf)
{
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    sprintf(buf,"%d-%02d-%02d-%02d:%02d:%02d ",t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec); 
}


void SLOG_INIT(LOG_LEVEL log_level, const char* log_file, int append)
{
    if(log_level<SLOG_LEVEL_TRACE || log_level>SLOG_LEVEL_NOLOG)
		log_level = SLOG_LEVEL_DEBUG;

    g_cur_level = log_level;

    g_outfile = stdout;
    if(log_file!=NULL && strlen(log_file)>0)
    {
        
        FILE* temp;
        if(append!=0)
            temp = fopen(log_file, "a");
	else
            temp = fopen(log_file, "w");

        if(temp != NULL)
	    g_outfile = temp;
    }
    
    char buf[1024];
    print_time(buf);
    sprintf(buf+strlen(buf), "[SLOG INIT] successful.level:%d, output file:%s, append mode:%d", g_cur_level, log_file==NULL?"NULL(use screen)":log_file, append);
    fprintf(g_outfile, "%s\n", buf);
}

void SLOG_UNINIT()
{
    char buf[1024];
    print_time(buf);
    sprintf(buf+strlen(buf), "[SLOG UNINIT] successful.");
    fprintf(g_outfile, "%s\n", buf);

    if(g_outfile!=NULL && g_outfile!=stdout)
        fclose(g_outfile);
}


void slog_trace(const char* fmt, ...)
{
    if(SLOG_LEVEL_TRACE >= g_cur_level)
    {
    	char buf[1024];
    	int len = 1024;
        int tlen;

    	va_list args;
        va_start(args, fmt);

        print_time(buf);
        sprintf(buf+strlen(buf),"[TRACE] ");
        tlen = strlen(buf);    
        vsnprintf(buf+tlen, len-tlen, fmt, args);
        if(g_outfile==NULL)
            fprintf(stdout, "%s\n",buf); 
        else
            fprintf(g_outfile, "%s\n", buf);
        va_end(args);
    }
}


void slog_debug(const char* fmt, ...)
{
    if(SLOG_LEVEL_DEBUG >= g_cur_level)
    {
	    char buf[1024];
	    int len = 1024;
        int tlen;

	    va_list args;
        va_start(args, fmt);

        print_time(buf);
        sprintf(buf+strlen(buf),"[DEBUG] ");
        tlen = strlen(buf);    
        vsnprintf(buf+tlen, len-tlen, fmt, args);
        if(g_outfile==NULL)
            fprintf(stdout, "%s\n",buf); 
        else
            fprintf(g_outfile, "%s\n", buf);
        va_end(args);
    }
}

void slog_info(const char* fmt,  ...)
{
    if(SLOG_LEVEL_INFO >= g_cur_level)
    {
        char buf[1024];
        int len = 1024;
	    int tlen;

        va_list args;
        va_start(args, fmt);

        print_time(buf);
        sprintf(buf+strlen(buf),"[INFO] ");
        tlen = strlen(buf);
        vsnprintf(buf+tlen, len-tlen, fmt, args);
        if(g_outfile==NULL)
            fprintf(stdout, "%s\n",buf); 
        else
            fprintf(g_outfile, "%s\n", buf);
        va_end(args);
    }
}
void slog_warn(const char* fmt,  ...)
{
    if(SLOG_LEVEL_WARN >= g_cur_level)
    {
        char buf[1024];
        int len = 1024;
        int tlen;

        va_list args;
        va_start(args, fmt);

        print_time(buf);
        sprintf(buf+strlen(buf),"[WARN] ");
        tlen = strlen(buf);
        vsnprintf(buf+tlen, len-tlen, fmt, args);
        if(g_outfile==NULL)
            fprintf(stdout, "%s\n",buf); 
        else
            fprintf(g_outfile, "%s\n", buf);

        va_end(args);
    }
}

void slog_error(const char* fmt, ...)
{
    if(SLOG_LEVEL_ERROR >= g_cur_level)
    {
        char buf[1024];
        int len = 1024;
        int tlen;

        va_list args;
        va_start(args, fmt);
       
        print_time(buf);
        sprintf(buf+strlen(buf),"[ERROR] ");
        tlen = strlen(buf);
        vsnprintf(buf+tlen, len-tlen, fmt, args);
        if(g_outfile==NULL)
            fprintf(stdout, "%s\n",buf); 
        else
            fprintf(g_outfile, "%s\n", buf);

        va_end(args);
    }
}

