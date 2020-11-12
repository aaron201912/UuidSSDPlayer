#ifndef __COAPI_DEV_H_
#define __COAPI_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "coapi_api/coapi_led.h"

/******************************************************************/
/********************** USED BY COAPI *****************************/
/****************** need to be compeleted *************************/

/**
 * @brief 获取电量
 * @return
 *      success: 电量值0——100
 *      fail: -1;
 */
int dev_get_battery(void);


/**
 * @brief 获取存储容量
 * @all  存储总容量
 * @free  剩余容量
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_get_storage(int *all, int *free);


/**
 * @brief 获取充电状态
 * @return 返回值
 *      1: 没有充电
 *      2: 充电中
 *      3: 充满了
 */
int dev_get_charge(void);

/**
 * @brief 获取ota的状态
 * @return 返回值
 *      0——100: 升级进度百分比
 *      1001: 升级成功
 *      1002: 升级失败
 *      1003: 没有升级
 */
int dev_ota_status(void);

/**
 * @brief 获取led的状态
 * @led 参考coapi_api/include/coapi_led.h led_info 定义
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_led_status(led_info *led);

/**
 * @brief 控制开关机（休眠/唤醒）
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_power_onoff(void);


/**
 * @brief 控制恢复出厂设置 
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_factory(void);

/**
 * @brief 获取童锁状态
 * @return
 *      0: 无此功能
 *      1: 童锁开启
 *      2: 童锁关闭
 */
int dev_childlock_status(void);

/**
 * @brief 控制童锁打开
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_childlock_open(void);

/**
 * @brief 控制童锁关闭
 * @return
 *      success: 0
 *      fail: -1;
 */
int dev_childlock_close(void);

int dev_mute(void);

int dev_unmute(void);

int dev_ota_start(void);

int dev_ota_cancle(void);

int dev_restart(void);

/******************************************************************/
/*************************** END **********************************/
/******************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COAPI_PLAYER_H_ */
