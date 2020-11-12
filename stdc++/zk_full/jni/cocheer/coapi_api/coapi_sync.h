#ifndef _COAPI_SYNC_H_
#define _COAPI_SYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

 /**
  * @brief 云端推送同步场景
  * @cmdid:
  *    推送命令id与请求命令id一一对应!
  *    1 << 0:  @media_push 推送音频
  *    1 << 1:  @media_daily 每日推送
  *    1 << 2:  @media_status 播放状态
  *    1 << 3:  @media_pause 播放暂停
  *    1 << 4:  @media_next 控制下一曲
  *    1 << 5:  @media_prev 控制上一曲
  *    1 << 6:  @media_stop 控制停止播放
  *    1 << 7:  @recv_chat 收到微聊信息
  *    1 << 8:  @ctrl_vol 音量 (0 —— 100)
  *    1 << 9:  @volume_up 控制音量加
  *    1 << 10: @volume_down 控制音量减法
  *    1 << 11: @childlock 童锁
  *    1 << 12: @led LED
  *    1 << 13: @timer 定时/时间
  *    1 << 14: @battery 获取电量
  *    1 << 15: @storage 获取存储
  *    1 << 16: @song_num 取本地音频数量
  *    1 << 17: @charge 获取充电状态
  *    1 << 18: @ota_status 获取ota状态
  *    1 << 19: @set_ota 请求ota
  *    1 << 20: @recv_json 云端推送json数据
  *    1 << 21: @send_json 设备发送json数据
  *    1 << 22: @onoff 控制设备开关机
  *    1 << 23: @wireless 无线指令
  *    1 << 24: @media_ctrl 控制媒体状态,比如控制播放进度等
  *    1 << 25: @factory 控制恢复出厂设置
  *    1 << 26: @devctrl 设备控制数据点下发
  */
enum sync_scene {
    SYNC_MEDIA_PUSH         = 1 << 0,
    SYNC_MEDIA_DAILY        = 1 << 1,
    SYNC_MEDIA_STATUS       = 1 << 2,
    SYNC_MEDIA_PAUSE        = 1 << 3,
    SYNC_MEDIA_NEXT         = 1 << 4,
    SYNC_MEDIA_PREV         = 1 << 5,
    SYNC_MEDIA_STOP         = 1 << 6,
    SYNC_RECV_CHAT          = 1 << 7,
    SYNC_VOL                = 1 << 8,
    SYNC_VOLUME_UP          = 1 << 9,
    SYNC_VOLUME_DOWN        = 1 << 10,
    SYNC_CHILDLOCK          = 1 << 11,
    SYNC_LED                = 1 << 12,
    SYNC_TIMER              = 1 << 13,
    SYNC_BATTERY            = 1 << 14,
    SYNC_STORAGE            = 1 << 15,
    SYNC_SONG_NUM           = 1 << 16,
    SYNC_CHARGE             = 1 << 17,
    SYNC_OTA_STATUS         = 1 << 18,
    SYNC_SET_OTA            = 1 << 19,
    SYNC_RECV_JSON          = 1 << 20,
    SYNC_SEND_JSON          = 1 << 21,
    SYNC_ONOFF              = 1 << 22,
    SYNC_WIRELESS           = 1 << 23,
    SYNC_MEDIA_CTRL         = 1 << 24,
    SYNC_FACTORY            = 1 << 25,
    SYNC_DEVCTRL            = 1 << 26,
};

/**
 * @charge:
 * 1: 没有充电
 * 2: 充电中
 * 3: 充满
 * 4: 插入充电
 * 5: 拔出充电
 */
enum charge_status {
    CHARGE_NONE = 1,
    CHARGE_ING = 2,
    CHARGE_FULL = 3,
    CHARGE_PLUGIN = 4,
    CHARGE_PLUGOUT = 5,
};

/**
 * @childlock
 * 1: 打开童锁
 * 2: 关闭童锁
 */
enum childlock_opt {
    CHILDLOCK_OPEN = 1,
    CHILDLOCK_CLOSE,
};

/**
 * @volume: 音量大小: 0-100
 * @childlock: 童锁状态: 1: 童锁开启 2: 童锁关闭
 * @battery: 电池电量
 * @storage: 存储容量, 高16位是存储容量，低16位表示剩余容量，1表示有TF卡但是没有计算容量，0表示没有TF卡
 * @song_num: 本地内容总数
 * @charge: 1: 没有充电，2: 充电中, 3: 充满
 */
typedef struct {
    int volume;
    int childlock;
    int battery;
    int storage;
    int song_num;
    int charge;
} state_item;

/**
 * @brief 数据同步相关的结构
 * @cmdid 命令ID
 * @status 状态标记
 * @json 数据
 * @len 数据长度
 * @state 状态相关结构
 */
typedef struct {
    int cmdid;
    int status;
    char *json;
    int len;
    state_item  state;
} sync_item;

typedef void (*sync_resp_cb)(int cmdid, sync_item *state);
typedef void (*json_resp_cb)(char *str, int len);

/*****************************同步指令初始化接口****************************************/

/**
 * @brief 初始化数据同步接口
 * 注意事项: 1. 回调函数不处理耗时长的调用;
 *           2. 不能在回调接口里面调用sync相关的接口(coapi_sync.h里定义的接口)
 */
int coapi_sync_init(sync_resp_cb sync_cb);
int coapi_sync_deinit(void);

/**
 * @brief 初始化JSON数据同步接口
 */
int coapi_json_init(json_resp_cb sync_cb);


/***************************** 推送/同步 基础接口 ***************************************/

/**
 * @brief 推送同步接口
 */
int coapi_push_sync(int cmdid);

/**
 * @brief 推送命令接口
 * @return 0: success, -1: fail
 */
int coapi_sync_cmd(int cmdid,sync_item *item);


/******************************** 设备状态同步接口 *******************************************/

/**
 * @brief 主动通知云端当前的电量状态
 * 同步给手机端端及时更新状态
 * @percent 电量百分比
 * @return 0: success, -1: fail
 */
int coapi_battery_level(int percent);

/**
 * @brief 主动通知云端当前的充电状态
 * 同步给手机端端及时更新状态
 * @onoff 当前的充电状态,参考@charge_status
 * @return 0: success, -1: fail
 */
int coapi_charge_status(int onoff);

/**
 * @brief 主动通知云端当前童锁的状态
 * 同步给手机端端及时更新状态
 * @onoff 当前童锁的状态,参考@childlock_opt
 * @return 0: success, -1: fail
 */
int coapi_childlock_status(int onoff);

/******************************** 发送JSON数据接口 *******************************************/

/**
 * @brief 发送json数据
 * 当定义的通信数据协议无法满足需求时，可通过此接口进行数据透传,扩展更多的数据应用
 * @str 要传输的数据
 * @len 要传输的数据长度
 * @return 0: success, -1: fail
 */
int coapi_sync_json(char *str,int len);

/**
 * @brief 主动查询并接收来自云端的json数据
 * 如：开机后主动查询是否有数据未接收处理
 * @return 0: success, -1: fail
 */
int coapi_json_req(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
