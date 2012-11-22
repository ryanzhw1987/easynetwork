/*
 * slog.h
 *
 *  Created on: 2012-6-12
 *      Author: LiuYongjin
 * Description: 多线程安全日志库
 */

#ifndef _LIB_SAMPLE_LOG_H_
#define _LIB_SAMPLE_LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

int SLOG_INIT(const char* config);
void SLOG_UNINIT();

#ifdef __SLOG_FULL__
#define PRE_STR "[%s:%s(%d)] ... "
#define VAL_STR ,__FILE__,__FUNCTION__,__LINE__
#else
#define PRE_STR " ... "
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
