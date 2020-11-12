#ifndef _COAPI_LED_H_
#define _COAPI_LED_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @led_mode led灯光模式
 *
 * @lighting 照明模式
 * @RGB RGB模式
 * @colorful 七彩变换模式
 * @sinbreak 单色呼吸模式
 */
enum led_mode {
    LED_LIGHTING = 1,
    LED_RGB,
    LED_COLORFUL,
    LED_SINBREAK,
};

/**
 * LED控制状态结构
 * @led_red 红灯（0 -- 255）
 * @led_green 绿灯（0 -- 255）
 * @led_blue 蓝灯（0 -- 255）
 *    以上0表示灭灯
 * @led_speed 灯呼吸的速度
 * @led_brightness 亮度
 * @led_temperature 色温
 * @led_mode 模式
 *   1: 照明模式
 *   2: RGB模式
 *   3: 七彩变换模式
 *   4: 单色呼吸模式
 */
typedef struct {
    int led_red;
    int led_green;
    int led_blue;
    int led_speed;
    int led_brightness;
    int led_temperature;
    int led_mode;
} led_info;

/*
 * led控制回调函数
 * @led 返回的LED结果信息
 * @status 返回的状态 0: 无，1: 正常返回, 其他： 错误码
 **/
typedef void (*led_resp_cb)(led_info *info, int status);

/**
 * @brief 初始化led同步/控制
 * @resp_cb 获取时间结果回调函数
 * @return 0: success, -1: fail
 */
int coapi_led_init(led_resp_cb cb);

/**
 * @brief 时间获取接口
 * @info 同步状态信息
 * @return 0: success, -1: fail
 **/
int coapi_led_start(led_info *info);

/**
 * @brief 主动通知云端当前LED状态
 * 同步给手机端端及时更新状态
 * @mode 灯光模式,参考@led_mode
 * @red 红色成分(0-255)
 * @green 绿色成分(0-255)
 * @green 绿色成分(0-255)
 * @blue 蓝色成分(0-255)
 * @temp 色温(0-255)
 * @speed 呼吸速度(0-15),数值越大呼吸越慢
 * @bri 亮度(0-255)
 */
void coapi_led_status(int mode, int red,
    int green, int blue, int temp, int speed,int bri);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
