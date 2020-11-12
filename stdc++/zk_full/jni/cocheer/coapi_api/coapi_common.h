#ifndef __COAPI_COMMON_H_
#define __COAPI_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

enum qos_type {
    QOS_ZERO = 0, /* 只发送一次，不保证100%达到率, 适用于不那么重要的数据,尤其连续高频的上报数据 */
    QOS_ONE, /* 每一个发送都会有云端反馈，没有反馈会接着发,适用于重要不能丢失的数据 */
};

enum dp_type {
   DP_TYPE_INT = 1, /* 整形 */
   DP_TYPE_BOOL,    /* 布尔 */
   DP_TYPE_ENUM,    /* 枚举 */
   DP_TYPE_STRING,  /* 字符 */
   DP_TYPE_FLOAT,   /* 浮点 */
   DP_TYPE_FAULT,   /* 错误 */
   DP_TYPE_BIN,     /* 二进制 */
};

/**
 * @brief 设备基础信息
 * @devid: 设备唯一id
 * @devtype: 设备类型, 0: 故事机/机器人,1:智能灯
 * @devops: android/freertos/mbed/ucos/linux
 * @devcpu: cpu型号
 * @devver: 产品的固件版本, 版本格式：01.00, 主要用于ota升级,高于此版本才会升级
 * @devname: 产品名称
 * @vendor: 厂商名称
 */
typedef struct {
    char devtype[16];
    char ver[8];
    char cpu[16];
    char ops[16];
    char devid[32];
    char name[32];
    char vendor[32];
} dev_comm;

/*
 * coapi_dev_init()接口可以在任何时候调用
 * 但必须在coapi_register_start被调用之前被调用
 */
int coapi_dev_init(dev_comm *dev);

/*
 * coapi_dev_deinit()
 */
int coapi_dev_deinit(void);

/* @brief 合成生成绑定二维码所需的数据，使用此数据生成的二维码
 * 		  可以使用公众号扫描，实现公众号与设备进行绑定。
 *
 * @ appkey 输入参数，产品appkey
 * @ devid  输入参数， 设备的唯一ID号
 * @ qrcode_data 输出参数,合成数据,此数据用于生成绑定二维码, 大小建议不小于128 byte。
 * @ qrdatalen  输出参数，合成数据的长度，等同于strlen(qrcode_data) 的值。
 *
 * @ 返回值：0 表示成功，其他表示失败
 */
int coapi_generate_qrcodedata(char *appkey, char *devid, char *qrcode_data, int *qrdatalen);

/* @brief 解析WiFi二维码数据. 用于设备扫描公众号二维码配网时，所获得到二维码内容的解析，
 * 		  从二维码内容中解析出WiFi的ssid和password；
 *
 * @ qrcodedata 输入参数，二维码内容数据
 * @ outssid  输出参数， WiFi的ssid，建议分配空间为 128 byte
 * @ outpasswd 输出参数,WiFi的password, 建议分配空间为 128 byte。
 *
 * @ 返回值：0 表示成功，其他表示失败
 *
 * 注意：使用二维码配网成功之后，设备绑定接口的第二个参数设为0，如：coapi_devbind_start(dev_bind_cb,0);
 */
int coapi_wifi_qrcodedata_analysis(char *qrcodedata, char *outssid, char *outpasswd);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
