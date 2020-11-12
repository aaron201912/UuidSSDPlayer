#ifndef _COAPI_DEVSTA_H_
#define _COAPI_DEVSTA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief 上报整形数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @val 要上报的数据点
 * @return 0: success, -1: fail
 */
int coapi_devsta_int(char *appkey, char *devid, int dpid,int qos,int val);

/*
 * @brief 上报字符型数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @str 要上报的数据点数据
 * @len 要上报的数据点数据长度
 * @return 0: success, -1: fail
 */
int coapi_devsta_string(char *appkey, char *devid, int dpid,int qos, const char *str, int len);

/*
 * @brief 上报布尔型数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @val 要上报的数据点
 * @return 0: success, -1: fail
 */
int coapi_devsta_bool(char *appkey, char *devid, int dpid,int qos, unsigned char val);

/*
 * @brief 上报枚举型数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @val 要上报的数据点
 * @return 0: success, -1: fail
 */
int coapi_devsta_enum(char *appkey, char *devid, int dpid,int qos, unsigned char val);

/*
 * @brief 上报浮点型数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @ft 要上报的数据点,32bit浮点型
 * @return 0: success, -1: fail
 */
int coapi_devsta_float(char *appkey, char *devid, int dpid,int qos, float ft);

/*
 * @brief 上报二进制数据点
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @bin 要上报的二进制数据
 * @len 要上报的二进制数据长度
 * @return 0: success, -1: fail
 */
int coapi_devsta_binary(char *appkey, char *devid, int dpid,int qos, unsigned char *bin, int len);

/*
 * @brief 上报故障信息数据
 * @appkey: 上报数据的设备appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 上报数据的设备唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 * @dpid 数据点ID,由云端创建数据点对应
 * @qos 0: 发送1次不保障达到率100%，没有返回结果; 1：有返回结果,没有返回接着发, 参考coapi_common.h @qos_type
 * @fault 要上报的故障信息
 * @len 要上报的故障信息长度
 * @return 0: success, -1: fail
 */
int coapi_devsta_fault(char *appkey, char *devid, int dpid,int qos, char *fault, int len);

/*
 * 数据点上报结果回调接口
 * @dpid:dpid: success, -x: 错误码
 * @data reserve
 **/
typedef void (*devsta_resp_cb)(int dpid,char *data);

/**
 * @brief 设备数据点上报初始化接口
 * @resp_cb 数据点上报结果回调接口
 * @return 0: success, -1: fail
 */
int coapi_devsta_init(devsta_resp_cb cb);
int coapi_devsta_deinit(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
