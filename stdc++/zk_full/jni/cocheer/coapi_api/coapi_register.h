#ifndef __COAPI_REGISTER_
#define __COAPI_REGISTER_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 注册返回结果定义
 *     0: 注册成功
 *     1: 已经注册
 *     2: 注册失败
 *     3: 无效APPkey
 *     4: 授权数超出
 *     5: 未充值过期了
 */
typedef enum {
    AUTH_STATE_AUTH_OK = 0,
    AUTH_STATE_HAS_AUTH,
    AUTH_STATE_AUTH_FAIL,
    AUTH_STATE_INVALID_PARA,
    AUTH_STATE_AUTH_OUT,
    AUTH_STATE_AUTH_OVERDUE
} auth_state_t;

/**
 * @brief 注册事件选项
 *     0: 主设备注册
 *     1: 子设备上线
 *     2: 子设备下线
 */
typedef enum {
    REG_OPT_MAINDEV_REG = 0,
    REG_OPT_SUBDEV_ONLINE,
    REG_OPT_SUBDEV_OFFLINE,
} register_opt;

/**
 * @brief 注册回调函数
 * @result 绑定结果返回，参考@auth_state_t
 */
typedef void (*register_cb)(unsigned char result, char *devid, char *uuid, char *wxid);

/**
 * @brief 如果在联网之前调用，需要联网完成才能完成注册回调结果回来)
 * coapi_register_start()接口可以在任何地方调用，但是在调用之前必须确保
 * coapi_logic_init(void)、coapi_comm_init(void)、coapi_dev_init(dev_comm *dev)已经初始化完毕
 *
 * @cb 注册结果回调函数
 * @appkey 必须 产品key
 * @secretkey  必须 产品key
 * @ssid 非必须，WIFI 名称，用于手机端显示连接的事哪个WIFI网络
 * @uuid 非必须，作为devid的备份id,与devid关联，相互绑定d
 * @nodevid 必须，0： 设备端注册时有devid，无需云端分配id; 1: 设备端没有唯一ID,云端分配ID
 */
void coapi_register_start(register_cb cb,char *appkey, char *secretkey, char *ssid, char *uuid, int nodevid);


/**
 * @brief 子设备上线(此接口是异步的，如果在联网之前调用，需要联网完成才能完成注册回调结果回来)
 * coapi_subdev_online()接口可以在任何地方调用，但是在调用之前必须确保
 * coapi_logic_init(void)、coapi_comm_init(void)、coapi_dev_init(dev_comm *dev)已经初始化完毕
 *
 * @cb 结果回调函数
 * @appkey 必须 子设备产品key
 * @secretkey  必须 子设备产品secretkey
 * @subid: 子设备的唯一ID
 * @version: 子设备的版本号
 */
void coapi_subdev_online(register_cb cb,char *appkey, char *secretkey, char *subid, char *version);

/**
 * @brief 子设备下线(此接口是异步的，如果在联网之前调用，需要联网完成才能完成注册回调结果回来)
 * coapi_subdev_online()接口可以在任何地方调用，但是在调用之前必须确保
 * coapi_logic_init(void)、coapi_comm_init(void)、coapi_dev_init(dev_comm *dev)已经初始化完毕
 *
 * @cb 结果回调函数
 * @appkey 必须 子设备产品key
 * @secretkey  必须 子设备产品secretkey
 * @subid: 子设备的唯一ID
 */
void coapi_subdev_offline(register_cb cb,char *appkey, char *secretkey, char *subid);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
