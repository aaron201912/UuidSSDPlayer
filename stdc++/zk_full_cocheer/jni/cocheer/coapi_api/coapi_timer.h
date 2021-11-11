#ifndef _COAPI_TIMER_H_
#define _COAPI_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 每次从云端同步多少个闹钟到本地,默认3个 */
#define TIMER_LOCAL_CNT 3

/* 东八区 */
#define TIMER_DEFAULT_ZONE 8

 /**
  * @idle  无闹钟响铃
  * @ring  闹钟触发响铃
  * @stop  闹钟响铃结束
  * @sleep 闹钟触发贪睡
  * @stopprank 关闭地域模式提醒
  * @clear 清除本设备的所有闹钟
  * @del 删除某个闹钟
  * @close 关闭某个闹钟
  */
typedef enum {
    TIMER_IDLE = 0,
    TIMER_RING,
    TIMER_STOP,
    TIMER_SLEEP,
    TIMER_STOPPRANK,
    TIMER_CLEAR,
    TIMER_DEL,
    TIMER_CLOSE,
} timer_status;

/**
 * mode A: 模式A: 只管响应触发闹钟(播放铃声), 无需处理闹钟逻辑,每次只返回到点触发的闹钟
 * mode B: 应用端需要根据回调的闹钟结果处理闹钟逻辑,每次返回的闹钟个数根据TIMER_LOCAL_CNT而定
 */
typedef enum {
    TIMER_MODE_A = 0,
    TIMER_MODE_B,
} timer_mode;

/*
 * "date": "2018-08-29", 响铃的日期
 * "time": "14:18:13",   响铃的时间
 * "alarmid": "xxxxx",   闹钟的唯一ID
 * "toneurl": "xxxxx",  预备铃声
 * "audiourl": "xxxx",  铃声/晨间咨询 等内容播报（支持 播放列表）
 * "action": 0, 动作，如定时关机。 0： 普通响铃, x: 表示其他控制指令,参考<coapi_cmd.h>
 * “wcmd”: 时间到了执行控制设备,参考coapi_devctrl.h
 * “msg”: 定时事件，如： msg=“睡觉”
 * "minite": 30 响铃时间
 * "type": 闹钟类型
 * "mode": 闹钟模式
 * "nap": 小睡时间(单位/分钟)
 * "zone": 时区(比如东八区为8; 西八区为-8)
 * "eventid": 播放完音频结束后的动作处理(toneurl & audiourl同时存在，播放完最后一个)
 */
typedef struct {
    char *alarmid;
    char *date;
    char *time;
    char *toneurl;
    char *audiourl;
    char *wcmd;
    char *msg;
    int action;
    int minite;
    int type;
    int mode;
    int nap;
    int zone;
    int eventid;
    int wcmd_len;
} timer_info;

/*
 * 语音上传应答回调函数
 * @info 返回的时间结果信息
 * @event 闹钟事件列表
 * @cnt 返回的状态 0: 无，n: (正整数: 闹钟的个数)正常返回, 其他： 错误码
 * @status: 参考@timer_status
 * @tick: 相对时间：秒
 * @闹钟模式
 **/
typedef void (*timer_resp_cb)(timer_info **info, char *event, int cnt, int status, int tick, unsigned char mode);

/**
 * @brief 初始化时间获取接口
 * @resp_cb 获取时间结果回调函数
 * @mode 设置闹钟模式mode A/B,@timer_mode
 * @return 0: success, -1: fail
 */
int coapi_timer_init(timer_resp_cb cb,int mode);
int coapi_timer_deinit(void);

/**
 * @brief 时间获取接口
 * @cnt 获取时间的个数
 * @status 当前闹钟的状态 参考@timer_status
 * @id 闹钟的id(当通知云端闹钟状态的时需要)
 * @return 0: success, -1: fail
 **/
int coapi_timer_start(int cnt,int status,char *id);

/**
 * @brief 告诉云端铃声在响(便于云端同步新的闹钟到本地或者进行更多的闹钟交互)
 * @alarm_id 当前在响铃的闹钟id,由云端分配，具有唯一性
 * @return 0: success, -1: fail
 */
int coapi_ring_bell_notify(char *alarm_id);

/**
 * @brief 告诉云端设备结束响铃(执行此函数后，进入语音交互不再是问答结束响铃模式)
 * @alarm_id 当前在响铃的闹钟id,由云端分配，具有唯一性
 * @return 0: success, -1: fail
 */
int coapi_ring_stop_notify(char *alarm_id);

/**
 * @brief 告诉云端设备做贪睡操作(执行此函数后，设备结束当前响铃，云端生成新的闹钟并重新同步)
 * @alarm_id 当前在响铃的闹钟id,由云端分配，具有唯一性
 * @return 0: success, -1: fail
 */
int coapi_ring_sleep_notify(char *alarm_id);

/**
 * @brief 清除本设备的所有闹钟
 * @return 0: success, -1: fail
 */
int coapi_timer_clear(void);

/**
 * @brief 删除某个闹钟
 * @alarm_id 要删除的闹钟ID
 * @return 0: success, -1: fail
 */
int coapi_timer_del(char *alarm_id);

/**
 * @brief 关闭某个闹钟
 * @alarm_id 要关闭的闹钟ID
 * @return 0: success, -1: fail
 */
int coapi_timer_close(char *alarm_id);

/**
 * @brief 停止整蛊闹钟
 * @return 0: success, -1: fail
 */
int coapi_ring_prank_stop(void);

/**
 * @brief 设备主动向云端请求闹钟同步到本地，用于闹钟更新，或者本地和云端同步等
 * @cnt 需要同步的闹钟个数，当用户实际设置的闹钟小于请求的个数时，返回总个数
 * @return 0: success, -1: fail
 */
int coapi_timer_req(int cnt);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
