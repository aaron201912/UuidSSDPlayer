#ifndef _COAPI_DEVCTRL_H_
#define _COAPI_DEVCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif


enum {
    DEVCTRL_OPT_AI = 0,
	DEVCTRL_OPT_CLIENT,
	DEVCTRL_OPT_TIMER,
};

/*
 * @brief 设备控制数据点下发结果回调接口
 * @appkey: 目标设备的appkey ,如果是本设备可以为NULL
 * @devid: 目标设备的设备唯一ID,如果是本设备可以为NULL
 * @dpid: 此数据点ID
 * @data: 下发的数据
 * @len: 下发的数据长度
 */
typedef void (*devctrl_dp_fn)(char *appkey, char *devid,int dpid, char *data, int len, int opt);

/*
 * 下发数据点结构体
 * @dpid: 数据点ID
 * @dptype: 数据点数据类型
 * @func函数指针，某个数据点的对应处理函数
 * @appkey: 子设备appkey（作为网关时使用）
 * @devid: 子设备唯一ID（作为网关时使用）
 */
typedef struct devctrl_dps {
    int dpid;
    int dptype;
    devctrl_dp_fn func;
    char *appkey;
    char *devid;
} devctrl_dps_t;

/*
 * @brief 将四个字节装换成整形
 */
int dp_bytes_to_int(const unsigned char bytes[4]);

/*
 * @brief 将1个字节装换成布尔
 */
int dp_bytes_to_bool(const unsigned char bytes[1]);

/*
 * @brief 将1个字节装换成枚举
 */
int dp_bytes_to_enum(const unsigned char bytes[1]);

/*
 * @brief 将四个字节装换成浮点性
 */
float dp_bytes_to_float(const unsigned char bytes[4]);

/*
 * @brief 处理封装的数据点,如：stream命令stream_t: data;
 *        timer命令timer_info:wcmd
 * @data: 数据
 * @len: 数据长度
 * @return 0: success, -1: fail
 */
int coapi_devctrl_data_handle(char *data, int len, int opt);

/**
 * @brief 设备控制数据点下发初始化接口
 * @handle 数据点下发回调注册
 * @return 0: success, -1: fail
 */
int coapi_devctrl_init(devctrl_dps_t *handle, int dpcnt);

/**
 * @brief 反初始化
 */
int coapi_devctrl_deinit(void);

/**
 * @brief 请求控制设备接口,无需应用调用
 * @return 0: success, -1: fail
 */
int coapi_devctrl_start(void);


/**
 * @brief 动态创建数据点,加入队列
 * @dpid 数据点ID
 * @dptype 数据点的数据类型
 * @appkey appkey
 * @devid 设备ID
 * @func 数据点下行回调函数
 *
 * @return 0: success, -1: fail
 */
devctrl_dps_t *coapi_devctrl_insert(int dpid, int dptype,char *appkey,char *devid, devctrl_dp_fn func);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
