#ifndef __COAPI_DEVBIND_H_
#define __COAPI_DEVBIND_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 绑定结果返回
 * @bind_ok  绑定成功
 * @has_bind 已经绑定过了
 * @reg_fail 注册失败了无法绑定
 * @other_fail 其他原因无法绑定
 * @userinfo 返回用户信息
 * @qrcode 获取绑定二维码
 */
typedef enum {
    DEVBIND_STATE_BIND_OK = 0,
    DEVBIND_STATE_HAS_BIND,
    DEVBIND_STATE_REG_FAIL,
    DEVBIND_STATE_OTHER_FAIL,
    DEVBIND_STATE_USERINFO,
    DEVBIND_STATE_QRCODE,
} devbind_state_t;

/**
 * @brief 绑定相关功能
 * @userinfo 获取绑定后的用户信息
 * @qrcode 获取绑定二维码
 */
typedef enum {
    DEVBIND_FUNC_USERINFO = 1,
    DEVBIND_FUNC_QRCODE,
} devbind_func_t;

/**
 * @brief 绑定回调函数
 * @result 绑定结果返回，参考@devbind_state_t
 * @str 返回结果，当result ＝ｕserinfo时，　JSON字符串描述,如:
 *    {
 *      "id":"7362889281833338", //用户唯一ID (第一个绑定的用户/管理员)
 *      "email":"xxx@cocheer.net", //用户邮箱
 *      "country_code":"0086", //用户手机号国家代码，比如中国: 0086
 *      "phone":"13976671173", //用户手机号
 *      "name":"小明", //用户昵称
 *      "logo":"http:xxxx.png", //用户头像
 *      "url":"http:xxxx.mp3", //绑定结果提示语音
 *      "text":"绑定成功", //绑定结果提示文本
 *      "addr":"深圳", //设备所在城市
 *    }
 *
 *    当 result = qrcode时，JSON:
 *　  {
 *      "wc_qrcode_url":"http:xxx.jpg", //公众号绑定二维码
 *    }　
 */
typedef void (*devbind_cb)(int result,char *str);

/**
 * @brief 绑定函数：用于绑定用户和设备绑定
 *  必须在执行联网动作之后,coapi_register_start成功返回之后才能调用此函数
 *  流程: app/公众号联网 -> coapi_register_start注册　-> coapi_devbind_start绑定
 * @cb 绑定结果回调函数
 * @id 0:声波联网,其他联网需主动传入
 */
int coapi_devbind_start(devbind_cb cb, int id);

/**
 * @brief 设备绑定相关功能
 * @cb 结果回调函数
 * @func 参考＠devbind_func_t
 * @return 0: 成功; -1: 失败
 */
int coapi_devbind_func(devbind_cb cb, int func);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
