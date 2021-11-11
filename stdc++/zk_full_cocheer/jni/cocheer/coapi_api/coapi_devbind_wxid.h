#ifndef __COAPI_DEVBIND_WXID_H_
#define __COAPI_DEVBIND_WXID_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 绑定结果返回
 * @bind_ok  绑定成功
 * @has_bind 已经绑定过了
 * @dev_not_exist 设备不存在
 * @wxid_null 微信授予id为空
 * @other_faile 其他 服务器错误
 */
typedef enum {
    DEVBIND_WXID_STATE_BIND_OK = 0,
    DEVBIND_WXID_STATE_HAS_BIND,
    DEVBIND_WXID_STATE_DEV_NOT_EXIST,
    DEVBIND_WXID_STATE_WXID_NULL,
    DEVBIND_WXID_STATE_OTHER_FAILED,
} devbind_wxid_state_t;

/**
 * @brief 绑定回调函数
 * @result 绑定结果返回，参考@devbind_wxid_state_t 
 *
 */
typedef void (*devbind_wxid_cb)(int result);

/**
 * @brief 绑定函数：用于绑定用户和设备绑定
 *  必须在执行联网动作之后,coapi_register_start成功返回之后才能调用此函数
 *  流程: app/公众号联网 -> coapi_register_start注册　-> coapi_devbind_wxid_start
 * @cb 绑定结果回调函数
 * @appkey 客户申请的appkey
 * @wxid 客户通过coapi_get_wxid请求获取的微信id
 */
int coapi_wxid_devbind_start(devbind_wxid_cb cb, const char *appkey, const char *wxid);

/**
 * @brief 获取微信iD的接口
 *  微信AIRKISS配网后自动绑定,需要先获取到关联的微信ID,并在保存在FLASH中,
 *  以后只需要从FLASH中获取,不需要再从后台获取.
 *  
 * @wx_origin_id 公众号id 格式如下gh_xxxxxx
 * @devid 设备的id
 * @wxid 长度大于32字节的数组指针,用于接收返回的微信id.
 * @retval 0 成功,微信id位于param wxid
 * @retval 0 失败
 */
int coapi_get_wxid(char *wx_origin_id, char *devid, char *wxid);

/**
  * airkiss设备绑定流程如下:
  *       公众号airkiss联网
  *              |
  *       网络连接成功后 coapi_register_start 注册
  *              |
  *       注册成功，后判断flash中是否存在wxid, 如不存在通过coapi_get_wxid获取,并保存到flash中(注意：必须保存，否则会造成严重后果). 
  *              |
  *       coapi_devbind_wxid_start建立关联
  *				 |
  *       通过airkiss发现设备协议发送广播包类似{"deviceInfo":{"deviceType":"gh_5e2706398fd0","deviceId":"gh_5e2706398fd0_de14c4d851c9ac23"}}, 
  *		  此处的deviceId为wxid; 广播端口：12476， 如果有airkiss_lan_pack（）接口，就可以调这个接口打包数据
  *              |
  *       公众号收到设备广播信息自动完成设备绑定
  */

#ifdef __cplusplus
} /* "C" */
#endif

#endif
