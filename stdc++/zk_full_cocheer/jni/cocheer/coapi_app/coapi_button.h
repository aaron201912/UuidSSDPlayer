#ifndef __COAPI_BUTTON__
#define __COAPI_BUTTON__

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_BUTTON
#define TREMBLE_COUNT 1

/**
 * 按键定义
 */
enum {
    E_BTN_VOICE,
    E_BTN_COUNT,
};

/**
 * 按键触发状态
 */
enum {
    BUTTON_UP,
    BUTTON_DOWN,
};

/**
 * 按键处理临时状态枚举
 */
enum button_status {
    BTN_STATUS_NONE,
    BTN_STATUS_SHORT,
    BTN_STATUS_LONG,
};

/**
 * @brief The button difault status.
 */
typedef enum {
    LOW_LEVEL,
    HIGH_LEVEL,
    NULL_LEVEL,
} gpio_default_status;

/**
 * @brief Pins init.
 */
typedef struct {
    int port;
    int pin;
    int potential;
} button_io;

/**
 * 按键通知类型
 */
typedef enum button_notify {
    BTN_NOTIFY_NONE,
    BTN_NOTIFY_SHORT_CLICK,
    BTN_NOTIFY_LONG_START,
    BTN_NOTIFY_LONG_END,
} button_notify_t;

/* 按键通知函数类型 */
typedef void (*btn_notify) (void);

/**
 * 按键结构体类型
 *
 * gpio: 按键管脚
 * button_id: 按键id,非gpio按键 (如矩阵键盘)
 * old_val: 上一轮按键检测的结果
 * new_val: 此轮按键检测的结果
 * cnt: 检测到某个按键状态的次数
 * status: 当前按键过程中的状态
 * demarcation: 长按判断阈值
 * reverse: 保留字段
 * on_short_press: 短按处理函数
 * on_long_press_start: 长按开始
 * on_long_press_end: 长按结束
 */
typedef struct button {

    button_io gpio;
    int button_id;
    int old_val;
    int new_val;
    int cnt;
    int status;
    int demarcation;
    int reverse;
    btn_notify on_short_press;
    btn_notify on_long_press_start;
    btn_notify on_long_press_end;
} button_t;

void coapi_button_init(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
