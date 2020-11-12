#ifndef _COAPI_CMD_H_
#define _COAPI_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CMD_TYPE_SPECIAL = 0,
    CMD_TYPE_SWITCH = 101,
    CMD_TYPE_STANDBY,
    CMD_TYPE_VOLUME,
    CMD_TYPE_LEDCTRL,
    CMD_TYPE_BATCHECK,
    CMD_TYPE_MOTION,
    CMD_TYPE_RING,
    CMD_TYPE_PLAYCTRL,
    CMD_TYPE_AICTRL,
    CMD_TYPY_FANCTRL,
    CMD_TYPE_HOST_CHANGCTRL,
    CMD_TYPE_RETURNCTRL,
    CMD_TYPE_TAKEPHOTOCTRL,
    CMD_TYPE_VIDEOCHATCTRL,
    CMD_TYPE_WIRELESS = 0xff,
} cmd_type;

typedef enum {
    CMD_SWITCH_BOOTUP = 1,
    CMD_SWITCH_SHUTDOWN,
    CMD_SWITCH_REBOOT,
} cmd_switch_ctrl;

typedef enum {
    CMD_STANDBY_ON = 1,
    CMD_STANDBY_WAKEUP,
} cmd_standby_ctrl;

typedef enum {
    CMD_VOLUME_UP = 1,
    CMD_VOLUME_DOWN,
    CMD_VOLUME_VAL,
    CMD_VOLUME_MUTE,
    CMD_VOLUME_MAX,
    CMD_VOLUME_MIN,
    CMD_VOLUME_MUTE_CANCEL,
} cmd_volume_ctrl;

typedef enum {
    CMD_LED_ON = 1, 		//开灯
    CMD_LED_OFF,			//关灯
    CMD_LED_UP,				//调亮
    CMD_LED_DOWN,			//调暗
    CMD_LED_BLUE,			//蓝灯
    CMD_LED_RED,			//红灯
    CMD_LED_GREEN,			//绿灯
    CMD_LED_YELLOW,			//黄灯
    CMD_LED_VIOLET,			//紫灯
    CMD_LED_WHITE,			//白灯
    CMD_LED_WARM,			//暖灯光
    CMD_LED_COLD,			//冷灯光
    CMD_LED_SUNLIGHT,		//日光模式
    CMD_LED_MOONLIGHT,		//月光模式
    CMD_LED_BREATING,		//呼吸模式
    CMD_LED_BREATING_UP,	//加速呼吸
    CMD_LED_BREATING_DOWN,	//减慢呼吸
    CMD_LED_COLORFUL,		//七彩模式
    CMD_LED_BRIGHTNESS_VAL,	//亮度值
    CMD_LED_BRIGHTNESS_MAX,	//最大亮度
    CMD_LED_BRIGHTNESS_MIN,	//最小亮度
    CMD_LED_ORANGE,			//橙灯
    CMD_LED_CANDLELIGHT,	//烛光灯
    CMD_LED_RHYTHM,			//彩灯律动
    CMD_LED_NATURE,			//自然灯
    CMD_LED_TUNE_WARM,		//调暖
    CMD_LED_TUNE_COLD,		//调冷
    CMD_LED_SLEEP_MODE,		//睡眠模式
    CMD_LED_READ_MODE,		//阅读模式
    CMD_LED_NIGHT_LIGHT,	//夜灯模式
} cmd_led_ctrl;

typedef enum {
    CMD_BATTERY_CHECK = 1,
} cmd_bat_ctrl;


typedef enum {
   CMD_MOTION_ON = 1,
   CMD_MOTION_OFF,
   CMD_MOTION_SPEED_UP,
   CMD_MOTION_SPEED_DOWN,
   CMD_MOTION_SPEED_HIGH,
   CMD_MOTION_SPEED_LOW,
   CMD_MOTION_SPEED_MID,
   CMD_MOTION_DIREC_LEFT,
   CMD_MOTION_DIREC_RIGHT,
   CMD_MOTION_DIREC_FORWARD,
   CMD_MOTION_DIREC_BACKWARD,
   CMD_MOTION_SPEED_VAL,
   CMD_MOTION_SPEED_MAX,
   CMD_MOTION_SPEED_MIN,
   CMD_MOTION_STAND_AT_ATTENTION,
   CMD_MOTION_STAND_AT_EASE,
   CMD_MOTION_DIREC_LEFT_GO_N,
   CMD_MOTION_DIREC_RIGHT_GO_N,
   CMD_MOTION_DIREC_FORWARD_GO_N,
   CMD_MOTION_DIREC_BACKWARD_GO_N,
   CMD_MOTION_GO_N,
   CMD_MOTION_TRANSFORM
} cmd_motion_ctrl;

typedef enum {
   CMD_WIRELESS_CTRL = 0xff,
   CMD_ALARM_UPDATE = 0xfe,
   CMD_CODE_BIND = 0xfd,
   CMD_DEVCTRL_DPS = 0xfc,
} cmd_spe_ctrl;

typedef enum {
   CMD_RING_START = 1,
   CMD_RING_STOP,
} cmd_ring_ctrl;

typedef enum {
   CMD_AI_VAD_ENTER = 1,
   CMD_AI_VAD_EXIT,
} cmd_ai_ctrl;

typedef enum {
   CMD_PLAYER_START = 1,
   CMD_PLAYER_STOP,
   CMD_PLAYER_PAUSE,
   CMD_PLAYER_CONTINUE,
   CMD_PLAYER_PREV,
   CMD_PLAYER_NEXT,
   CMD_PLAYER_RANDOM,
   CMD_PLAYER_SINGLE,
   CMD_PLAYER_LOOP,
   CMD_PLAYER_LOCAL,
} cmd_player_ctrl;

typedef enum {
   CMD_FAN_OPEN = 1,
   CMD_FAN_CLOSE,
   CMD_FAN_SPEED1,
   CMD_FAN_SPEED2,
   CMD_FAN_SPEED3,
} cmd_fan_ctrl;

typedef enum {
   CMD_TAKE_PHOTO = 1,
} cmd_takephoto_ctrl;

typedef enum {
   CMD_RETURN = 1,
} cmd_return_ctrl;

typedef enum {
   CMD_VIDEO_CHAT_START = 1,
   CMD_VIDEO_CHAT_STOP,
} cmd_video_chat_ctrl;


/*
 * 此回调接口为处理语音控制指令
 * 通过语音控制设备的动作
 * 可参考<coapi_app/src/coapi_ctrl_cmd.c>实现，或者直接使用参考代码
 * 在对应的case里添加逻辑即可
 * 函数里面不能使用过于复杂的逻辑，尽量减少调用时间
 * 使用场景:  语音控制指令(coapi_speech.c)/定时控制指令(coapi_comm.c: timmer)
 */
void coapi_cmd_ctrl_cb(int action, char *data);


int coapi_get_cmd_type(int action);
int coapi_get_ctrl_cmd(int action);
int coapi_get_ctrl_val(int action);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
