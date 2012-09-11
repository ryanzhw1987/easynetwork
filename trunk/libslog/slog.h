#ifndef _SAMPLE_LOG_H_20120612_LIUYONGJIN_
#define _SAMPLE_LOG_H_20120612_LIUYONGJIN_


typedef enum __log_level
{
    SLOG_LEVEL_TRACE = 0,
	SLOG_LEVEL_DEBUG,
	SLOG_LEVEL_INFO,
	SLOG_LEVEL_WARN,
	SLOG_LEVEL_ERROR,
    SLOG_LEVEL_NOLOG
}LOG_LEVEL;

#ifdef __cplusplus
extern "C"{
#endif

void SLOG_INIT(LOG_LEVEL log_level, const char* log_file, int append);
void SLOG_UNINIT();

#ifdef __SLOG_FULL__
#define PRE_STR "[%s:%s(%d)] ... "
#define VAL_STR ,__FILE__,__FUNCTION__,__LINE__
#else
#define PRE_STR 
#define VAL_STR 
#endif

#define SLOG_TRACE(format, ...) slog_trace(PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_DEBUG(format, ...) slog_debug(PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_INFO(format, ...) slog_info(PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_WARN(format, ...) slog_warn(PRE_STR format VAL_STR ,##__VA_ARGS__)
#define SLOG_ERROR(format, ...) slog_error(PRE_STR format VAL_STR ,##__VA_ARGS__)

void slog_trace(const char *fmt, ...);
void slog_debug(const char *fmt, ...);
void slog_info(const char *fmt, ...);
void slog_warn(const char *fmt, ...);
void slog_error(const char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif
