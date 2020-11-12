#ifndef __COAPI_LOGGER_H_
#define __COAPI_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

/* 定义日志等级 */
typedef enum {
	ELEVEL_DEBUG = 0,
	ELEVEL_INFO,
	ELEVEL_ERROR,
} TlogLevel;

/*
 * 根据场景选择合适的打印函数输出
 **/
#define xtrace(...)				__xlogger_printf(ELEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define xdebug(...)				__xlogger_printf(ELEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define xinfo(...)				__xlogger_printf(ELEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define xerror(...)				__xlogger_printf(ELEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define xassert(e)	{ if(e); else {__xlogger_printf(ELEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, "assert false, %s", #e); assert(e); } }
void __xlogger_printf(int level, const char* filename, const char* function, unsigned int line, const char* format, ...);

#define str_not_null(str) ((str != NULL) && (strlen((const char *)str) > 0))

/*
 * @brief 输出调试信息
 * 在coapi_logic_init之后调用生效
 */
void coapi_logger_debug(void);


#ifdef __cplusplus
} /* "C" */
#endif

#endif /* LOGGER_H_ */
