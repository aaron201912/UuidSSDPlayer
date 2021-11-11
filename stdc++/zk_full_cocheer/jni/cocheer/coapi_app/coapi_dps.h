#ifndef COAPI_DPS_H_
#define COAPI_DPS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 上传数据的类型
 *
 */
typedef enum {
    DEV_TYPE_INT      = 1, /*!< 整形 */
    DEV_TYPE_BOOL     = 2, /*!< 布尔 */
    DEV_TYPE_ENUM     = 3, /*!< 枚举 */
    DEV_TYPE_STRING   = 4, /*!< 字符 */
    DEV_TYPE_FLOAT    = 5, /*!< 浮点 */
    DEV_TYPE_FAULT    = 6, /*!< 错误型 */
    DEV_TYPE_BIN      = 7, /*!< 二进制型 */
} co_dev_type_t;

/**
 * @brief 功能点唯一ID
 * 通用101xxxxxxxx/客户产品102xxxxxxxx
 * 客户可以通过应用管理平台添加自己的设备能力
 * 上报：可查询
 * 下发：可控制
 */
typedef enum {
    DEV_FUNC_ID_VOLUME      , /*!< 音量 	101000001-101000007 */
    DEV_FUNC_ID_CHARGE      , /*!< 充电 	101000008 			*/
    DEV_FUNC_ID_BATTERY     , /*!< 电量 	101000009 			*/
    DEV_FUNC_ID_OTA         , /*!< OTA  	101000010-101000014 */
    DEV_FUNC_ID_STORE       , /*!< 存储 	101000015-101000017 */
    DEV_FUNC_ID_CHILDLOCK   , /*!< 童锁 	101000018 			*/
    DEV_FUNC_ID_ONOFF       , /*!< 开关机 	101000019-101000021 */
} co_dev_func_id_t;

/**
 * @brief 音量控制数据点
 */
typedef enum {
    DEV_VOLUME_UP = 101000001,  /*!< 音量加 可控制 */
    DEV_VOLUME_DOWN,            /*!< 音量减 可控制 */
    DEV_VOLUME_VAL,             /*!< 音量值 可查询&可控制 */
    DEV_VOLUME_MUTE,            /*!< 静音   可查询&可控制 */
    DEV_VOLUME_MAX,             /*!< 最大音量 可查询&可控制 */
    DEV_VOLUME_MIN,             /*!< 最小音量 可查询&可控制 */
    DEV_VOLUME_UNMUTE,          /*!< 取消静音 可控制 */
} dev_volume_ctrl_t;

/**
 * @brief 充电控制数据点
 */
typedef enum {
    DEV_CHARGE_CHECK = 101000008,          /*!< 充电状态查询 可查询 */
} dev_charge_ctrl_t;

/**
 * @brief 充电控制数据点
 */
typedef enum {
    DEV_BATTERY_CHECK = 101000009,          /*!< 充电状态查询 可查询 */
} dev_battery_ctrl_t;


/**
 * @brief ota控制数据点
 */
typedef enum {
    DEV_OTA_START    = 101000010,   /*!< 开始升级 可控制 */
    DEV_OTA_CANCEL   ,              /*!< 取消升级 可控制 */
    DEV_OTA_FAIL     ,              /*!< 升级失败 可查询 */
    DEV_OTA_SUCCESS  ,              /*!< 升级成功 可查询 */
    DEV_OTA_PROGRESS ,              /*!< 升级进度 可查询 */
} dev_ota_ctrl_t;

/**
 * @brief 存储控制数据点
 */
typedef enum {
    DEV_STORE_QUERY_TATOL   = 101000015,    /*!< 查询总容量   可查询 */
    DEV_STORE_QUERY_USED    ,               /*!< 查询使用容量 可查询 */
    DEV_STORE_QUERY_FREE    ,               /*!< 查询剩余容量 可查询 */
} dev_store_ctrl_t;

/**
 * @brief 童锁控制数据点
 */
typedef enum {
    DEV_CHILDLOCK   = 101000018,          /*!< 可查询&可控制 */
} dev_childlock_ctrl_t;

/**
 * @brief 童锁控制数据点
 */
typedef enum {
    DEV_ON          = 101000019,    /*!< 开机 可控制 */
    DEV_OFF         ,               /*!< 关机 可控制 */
    DEV_REBOOT      ,               /*!< 重启 可控制 */
} dev_onoff_ctrl_t;



/**
 * @brief 取暖桌控制数据点
 */
typedef enum {
    WARMER_ON          = 202000001,    /*!< 开打开取暖 */
    WARMER_OFF         ,               /*!< 关闭取暖 */
    WARMER_VALUE      ,               /*!< 取暖档位 */
} warmer_ctrl_t;

/**
 * @brief 切纸机数据点
 */
typedef enum {
    DEV_SET_DIATANCE         = 202000020,    /*!< 设置距离 */
} dev_paper_ctrl_t;

/*
 * @brief Init coapi dev data point callback
 *
 * @return int
 * @retval 0  success
 * @retval -1 failed
 *
 */
int coapi_dev_ctrl_init(void);
int coapi_dev_ctrl_deinit(void);

#ifdef __cplusplus
}
#endif

#endif //end of file COAPI_DEV_CTRL_H_
