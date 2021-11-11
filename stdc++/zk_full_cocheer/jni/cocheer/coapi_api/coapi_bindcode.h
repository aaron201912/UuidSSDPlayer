#ifndef __COAPI_BINDCODE_H_
#define __COAPI_BINDCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取绑定码回调函数
 * @status 绑定结果返回 0: success; -1: fail
 * @url 绑定码播报连接
 * @code 绑定码
 */
typedef void (*bindcode_cb)(int status, char *url, char *code);

/**
 * @brief 绑定函数：用于获取绑定码, 用于airkiss联网之后，通过播报绑定码进行绑定
 *  必须在执行联网动作之后,coapi_register_start成功返回之后才能调用此函数
 *  流程: 联网(airkiss) -> coapi_register_start注册　-> coapi_bindcode_start绑定
 * @cb 绑定码结果回调函数
 */
void coapi_bindcode_start(bindcode_cb cb);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
