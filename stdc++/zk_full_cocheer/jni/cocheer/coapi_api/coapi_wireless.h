#ifndef _COAPI_WIRELESS_H_
#define _COAPI_WIRELESS_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @used: 无线指令的个数
 * @data: 无线指令数据json
 */
typedef struct {
    int used;
    char *data;
} wireless_info;

/*
 * 语音上传应答回调函数
 * @wireless 返回的无线指令结果信息
 * @status 返回的状态 0: 无，1: 正常返回, 其他： 错误码
 **/
typedef void (*wireless_resp_cb)(wireless_info *info, int status);

/**
 * @brief 初始化无线指令获取接口
 * @resp_cb 获取无线指令结果回调函数
 * @return 0: success, -1: fail
 */
int coapi_wireless_init(wireless_resp_cb cb);

/**
 * @brief 无线指令获取接口
 * @used 发送无线指令的个数
 * @data 无线指令数据(json)
 * @return 0: success, -1: fail
 **/
int coapi_wireless_start(int used, char *data);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
