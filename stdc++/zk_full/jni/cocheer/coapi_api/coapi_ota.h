#ifndef _COAPI_OTA_H_
#define _COAPI_OTA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * OTA状态反馈给云端
 * 1 - 100: 设备升级中进度百分比
 * 1001: 设备升级成功
 * 1002: 设备升级失败
 * 1003: 设备没有升级
 */
enum ota_status {
    OTA_SUCCESS = 1001,
    OTA_FAIL = 1002,
    OTA_NONE = 1003,
};

/**
 * ota: 下发模式:
 * 1. 下发url
 * 2: 分片下发
 */
enum ota_mode {
    OTA_MODE_URL = 1,
    OTA_MODE_PKG = 2,
};

/**
 * ota: 请求动作:
 * 1: 检查是否有升级的版本;
 * 2: 自动升级;
 * 3: 同步升级状态;
 * 4: 手动升级;
 */
enum ota_req_action {
    OTA_ACTION_CHECK = 1,
    OTA_ACTION_REQ,
    OTA_ACTION_SYNC,
    OTA_ACTION_MANUAL_REQ,
};

/**
 * ota: 云端返回动作
 * 0: 没有可升级的版本
 * 1. 返回有升级版本的结果
 * 2: 下发升级包
 */
enum ota_rsp_action {
    OTA_ACTION_NO_NEW_VERSION = 0,
    OTA_ACTION_NEW_VERSION,
    OTA_ACTION_LOAD_PKG,
};

/**
 * @endflag 标记ota下发包是否是最后一帧数据
 * 0: 云端继续下发ota包
 * 1: 这是云端下发的最后一个ota包
 */
enum ota_pkg_flag {
    OTA_PKG_CONTINUE = 0,
    OTA_PKG_END,
};

/**
 * OTA相关的结构
 * 返回ota升级包数据
 * @action: 动作:
 *    1. 返回有升级版本的结果(云端返回);
 *    2: 没有可升级的版本(云端返回)
 *    3: 下发升级包(云端返回)
 * @mode:
 *    1: url模式
 *    2: 分片模式
 * @endflag: 分片下发模式下: 标记是否是最后一个包, 0:不是，1表示最后一个包
 * @offset: 分片下发模式下: 包的偏移量
 * @size: 包的大小，分片下发模式下: 本次下发数据包的大小
 * @data: 包的数据/url
 * @md5: 包的md5（升级包原始的md5值，由后台下发）
 * @version: 版本号
 * @calcmd5: 分片下发时，接收到的升级包计算出来的md5值
 * @detail: 升级版本描述信息 
 * @appkey: 要升级设备的appkey ,如果是本设备可以为NULL,一般用于网关子设备
 * @devid: 要升级设备的唯一ID,如果是本设备可以为NULL,一般用于网关子设备
 */
typedef struct {
    int action;
    int mode;
    int endflag;
    int offset;
    int size;
    char *data;
    char *version;
    char *md5;
    char *calcmd5;
    char *detail;
    char *appkey;
    char *devid;
} ota_info;

/*
 * OTA应答回调函数
 * @ota 返回的ota结果信息
 * @status 返回的状态 0: 无音频，1: 正常返回, 其他： 错误码
 **/
typedef void (*ota_resp_cb)(ota_info *ota, int status);

/**
 * @brief 初始化ota接口
 * @resp_cb 获取ota结果回调函数
 * @mode: 参考＠ota_mode
 * @return 0: success, -1: fail
 */
int coapi_ota_init(ota_resp_cb cb, int mode);
int coapi_ota_deinit(void);

/**
 * @brief 通知云端ota升级成功了
 * 同步给手机端实时显示升级状态
 * @return 0: success, -1: fail
 */
int coapi_ota_success(void);

/**
 * @brief 通知云端ota升级失败了
 * 同步给手机端实时显示升级状态
 * @return 0: success, -1: fail
 */
int coapi_ota_fail(void);

/**
 * @brief 通知云端当前没有进行ota升级
 * 同步给手机端实时显示升级状态
 * @return 0: success, -1: fail
 */
int coapi_ota_none(void);

/**
 * @brief 通知云端ota升级进度
 * 同步给手机端实时显示升级状态
 * @percent 升级进度百分比
 * @return 0: success, -1: fail
 */
int coapi_ota_process(int percent);

/**
 * @brief 主动检查是否有可升级的版本
 * 结果通过回调返回给设备
 * 需要先初始化 coapi_ota_init
 * @return 0: success, -1: fail
 */
int coapi_ota_check(void);

/**
 * @brief 执行自动升级动作
 * 设备端发起升级结果通过回调返回给设备
 * 需要先初始化 coapi_ota_init
 * @return 0: success, -1: fail
 */
int coapi_ota_start(void);

/**
 * @brief 执行手动升级动作
 * 由云端发起升级，如ota版本验证，用户通过手机端点击升级，
 * 结果通过回调返回给设备
 * 需要先初始化 coapi_ota_init
 * @return 0: success, -1: fail
 */
int coapi_ota_manual_start(void);

/**
 * @brief 获取升级模式 
 * 需要先初始化 coapi_ota_init
 * @return mode: 参考＠ota_mode, -1: fail
 */
int coapi_ota_mode(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
