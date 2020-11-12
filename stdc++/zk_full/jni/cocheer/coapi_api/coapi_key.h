#ifndef _COAPI_KEY_H_
#define _COAPI_KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @key_id 按键ID
 * @short_press 短按功能
 * @long_press 长按功能
 * @double_click 双击功能
 */
typedef struct {
    int key_id;
    int short_press;
    int long_press;
    int double_click;
} key_info;

/*
 * @key_id 按键ID(由多个单个按键的按键id或操作之后获得)
 * @func_id 功能ID
 */
typedef struct {
    int key_id;
    int func_id;
} comk_info;

/*
 * 按键定义信息返回
 * @info 返回的按键定义结果信息
 * @comk 返回的按键定义结果信息
 * @nkey 返回的状态 0: 无，n: (正整数: 单按键的个数)正常返回, 其他： 错误码
 * @ncomk 返回的状态 0: 无，n: (正整数: 组合按键的个数)正常返回, 其他： 错误码
 * @tick 时间戳, 用于判断本地key与云端key的定义哪个是最新的
 **/
typedef void (*key_resp_cb)(key_info **info, int nkey, comk_info **comk,int ncomk,int tick);

/**
 * @brief 获取按键定义接口
 * @resp_cb 获取按键定义果回调函数
 * @return 0: success, -1: fail
 */
int coapi_key_start(key_resp_cb cb);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
