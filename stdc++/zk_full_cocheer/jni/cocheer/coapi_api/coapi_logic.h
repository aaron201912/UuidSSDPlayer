#ifndef __COAPI_LOGIC__
#define __COAPI_LOGIC__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @server_host coapi连接服务器类型
 * @svr: 正式服务器,用于生产
 * @gray: 灰度服务器,用于产品开发期间调试测试,不能用于生产!!
 * @dev: 开发服务器,勿用
 */
enum server_host {
    COAPI_SERVER_SVR = 0,
    COAPI_SERVER_GRAY,
    COAPI_SERVER_DEV,
};

/* 此回调接口为把网络状态返回给用户层,需用户自己实现其内容
 * 参数 status 的值表示如下:
 * 0: 未连接状态
 * 1: 连接中
 * 2: 已连接状态
 * 3: 关闭
 * 4: 网络差，信号弱
 * 5: 重新连接
 * 6: 未知状态
 */
typedef void (*conn_state_cb)(unsigned char status);

/*
 * @brief 获取网络状态，由开发者实现函数体逻辑
 * 0: 网络断开
 * 1: 网络连接
 * 注意事项: 函数体的运行逻辑尽量简单，不能有耗时的操作，运行耗时<5毫秒
 */
typedef int (*wifi_state_cb)(void);

/**
 * @brief 初始化连接模块
 * @conn: coapi与云端连接状态返回的回调
 * @wifi: 通过此回调把网络状态告诉coapi
 * @server_sel 选择要连接的服务器，参考＠server_host
 * @注：此接口建议在设备主程序初始化时调用(必须确保在进行声波配网之前调用)
 */
void coapi_logic_init(conn_state_cb conn, wifi_state_cb wifi, unsigned char server_sel);
void coapi_logic_deinit(void);

/**
 * @brief 判断连接模块是否连接
 * @return 0: 未连接；> 0 连接成功
 */
int coapi_connect_status(void);

/**
 * @brief coapi在使用期间更换服务器
 * @server_sel 选择要连接的服务器，参考＠server_host
 */
void coapi_relink_server(unsigned char server_sel);

#ifdef __cplusplus
} /* "C" */
#endif

#endif /* __COAPI_LOGIC__ */
