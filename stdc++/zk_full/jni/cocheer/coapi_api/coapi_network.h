#ifndef __COAPI_NETWORK_LOGIC_H__
#define	__COAPI_NETWORK_LOGIC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    COAPI_AUDIOWAVE = 0, /* 声波配网模式 */
	COAPI_SMARTCONFIG,   /* smartconfig 配网模式 */
	COAPI_AP             /* AP配网模式 */
} network_mode_e;

typedef struct _network_info {
	char *ssid;
	char *passwd;
} network_info_t;

/* 此回调接口为把解析出来的wifi的ssid和password返回给用户层 */
typedef void (*coapi_network_cb)(network_info_t *nt_info);

/*
* 输入参数：
* data ：PCM 数据,参数要求：采样率44.1KHZ, 16bit,单声道
* size ：PCM 数据长度，1024的整数倍
* endflag ：结束标志符, 由使用者来决定是否结束配网,一般是两种条件结束:
*         1. 解码完成，ssid与passwd已经回调回去,使用者收到后结束
*         2. 使用认为配网超时了
*/
void coapi_network_data_process(char *data, int size, int endflag);

/**
 * @brief 初始化解析ssid&passwd信息模块
 * @network_cb 解析结果完成后回调函数
 * @mode 联网方式选择
 * @return: 0: succuss, -1: fail
 * 
 * @注：此接口在调用之前必须确保 coapi_logic_init(void)和coapi_comm_init(void)已经被完毕
 */
int coapi_network_init(coapi_network_cb network_cb, network_mode_e mode);

/**
 * @brief 反初始化解析ssid&passwd信息模块
 * @mode 联网方式选择
*  @return: 0: succuss, -1: fail
 */
int coapi_network_deinit(network_mode_e mode);

/*
* @brief 主要用于蓝牙联网、smartconfg、ap联网解析ssid&passwd
* @info：返回的网络信息
* @data：接收到的数据
* @size：数据长度
* @return: 0: succuss, -1: fail
* 注意事项: 调用此接口后info->ssid, info->passwd 的内存如果不是NLLL则需手动释放
*/
int coapi_network_info(network_info_t *info, unsigned char *data, int size);

#ifdef __cplusplus
} /* "C" */
#endif


#endif /*__COAPI_NETWORK_LOGIC_H__*/
