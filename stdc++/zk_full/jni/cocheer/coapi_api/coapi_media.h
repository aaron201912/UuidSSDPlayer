#ifndef _COAPI_MEDIA_H_
#define _COAPI_MEDIA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 0: 当前云端的播放列表
 * 1: AI点播续播
 * 2: 手动推送
 * 3: 开机推送
 * 4: 家庭圈微聊
 * 5: 朋友圈微聊
 * 6: 定时播放续播
 */
enum media_type {
    AUDIO_PLAY_CUR = 0,
    AUDIO_PLAY_AI = 1,
    AUDIO_PLAY_PUSH,
    AUDIO_PLAY_DALIY,
    AUDIO_PLAY_FMLYCHAT,
    AUDIO_PLAY_GUYCHAT,
    AUDIO_PLAY_TIMER,
};

/**
 * 1. 播放器停止状态
 * 2. 播放器播放状态
 * 3. 播放器暂停状态
 */
enum media_status {
    AUDIO_PLAYER_STOP = 1,
    AUDIO_PLAYER_PLAY,
    AUDIO_PLAYER_PAUSE,
};

/**
 * 1. 顺序播放
 * 2. 列表循环
 * 3. 单曲循环
 * 4. 随机播放
 */
enum media_mode {
    PLAY_MODE_ORDER = 1,
    PLAY_MODE_LOOP,
    PLAY_MODE_CIRCUL,
    PLAY_MODE_RANDOM,
};

/**
 * @type:
 *     0: 当前云端的播放列表
 *     1: AI点播续播
 *     2: 手动推送
 *     3: 开机推送
 *     4: 家庭圈微聊
 *     5: 朋友圈微聊
 *     6: 定时播放续播
 * @seq: 当前播放的音乐在云端列表中的位置
 * @status 播放状态: 1: 停止播放，2: 播放中: 3: 暂停播放
 * @mode 播放状态: 1: 列表顺序, 2: 列表循环: 3: 单曲循环 4:随机播放
 * @position 播放的秒数
 * @duration 音频总时长
 * @action 0: 设备同步给云端; 1：云端同步给设备
 * @event 事件: 某个音频被中断的原因
 */
typedef struct {
    int type;
    int seq;
    int status;
    int mode;
    int position;
    int duration;
    int action;
    int event;
} media_info;

/*
 * 同步media信息给云端回调函数
 * @info 云端控制设备状态,比如控制播放进度
 * @status 返回的状态码
 **/
typedef void (*media_resp_cb)(media_info *info,int status);

/**
 * @brief 初始化media同步/控制
 * @resp_cb 返回结果回调函数
 * @return 0: success, -1: fail
 */
int coapi_media_init(media_resp_cb cb);
int coapi_media_deinit(void);

/**
 * @brief 同步媒体状态给云端
 * @info 同步状态信息
 * @return 0: success, -1: fail
 **/
int coapi_media_start(media_info *info);

/**
 * @brief 通知云端停止播放,手机端会更新显示状态
 * 当设备端停止播放音频时调用此接口
 */
void coapi_media_stop(void);

/**
 * @brief 通知云端，当前播放期处于暂停状态,手机端会更新显示状态
 * 当设备端执行暂停操作时调用此接口
 */
void coapi_media_pause(void);

/**
 * @brief 通知云端设备在播放，手机端会更新显示状态
 * 当设备端执行暂停恢复操作时调用此接口
 */
void coapi_media_play(void);

/**
 * @brief 设置云端列表播放模式,参考＠enum_mode
 * 修改播放模式调用此接口
 */
void coapi_media_mode(int mode);

/**
 * @brief 通知云端停止播放,并且告知云端播放停止在哪个位置
 * @pos: 结束播放时,该音频在此设备上的播放时长, 单位秒
 * @eventid: 导致音频结束的事件，参考func_Id, coapi_funchub.h. 0表示播放完成自然结束
 * 1. 手机端会更新显示状态
 * 2. 云端根据用户对此音频的播放情况做智能算法
 *
 * 当设备端停止播放音频时调用此接口
 */
void coapi_media_pos_stop(int eventid, int pos);

/**
 * @brief 主动通知云端当前的播放状态
 * 同步给手机端端及时更新状态
 * @type: 播放类型, 参考@play_type
 * @seq: 音频在云端列表中的序号,可直接使用云端返回音频时的序号
 * @status: 当前的播放状态，参考@play_status
 * @mode: 当前的播放模式，参考@play_mode
 * @position: 当前的播放进度，单位秒
 * @duration: 当前的播放音频总时长，单位秒
 * @action: 默认0
 * @event: 表示音频被中断的事件，参考func_id, coapi_funhub.h ,默认0
 * 注意事项: 不需要更新的值置为-1，而非0
 */
void coapi_media_status(int type, int seq, int status, int mode, int position,int duration,int action, int event);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
