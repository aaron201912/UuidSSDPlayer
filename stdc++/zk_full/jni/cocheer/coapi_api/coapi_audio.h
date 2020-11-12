#ifndef _COAPI_AUDIO_H_
#define _COAPI_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 音频类型
 *  0: 当前正在播放的类型
 *  1: AI点播续播
 *  2: 手动推送
 *  3: 开机推送
 *  4: 家庭圈微聊
 *  5: 朋友圈微聊
 *  6: 定时播放续播
 *  7: 功能集列表
 *  8: TTS合成推送
 */
typedef enum {
	TYPE_CUR = 0,
	TYPE_AI,
	TYPE_PUSH,
	TYPE_DAILY,
	TYPE_FMLYCHAT,
	TYPE_GUYCHAT,
	TYPE_TIMER,
	TYPE_FUNCHUB,
	TYPE_PUSHTTS,
} audio_type;

/*
 * 微聊信息类型
 *
 *  0: 文字信息
 *  1: 音频信息
 *  2: 图片信息
 *  3: 视频信息
 *  4: 表情信息
 */
typedef enum {
    CHAT_TYPE_TEXT = 0,
    CHAT_TYPE_AUDIO,
    CHAT_TYPE_PIC,
    CHAT_TYPE_VIDEO,
    CHAT_TYPE_EMOJI,
} chat_type;

/**
 * @none 没有事件类型
 * @media_prev 上一曲
 * @media_next 下一曲
 * @media_seed 切到某一首
 */
typedef enum {
	EVENT_NONE = 0,
	EVENT_MEDIA_PREV = 1,
	EVENT_MEDIA_NEXT = 2,
	EVENT_MEDIA_SEED = 3,
} audio_event;

/**
 * @audiourl: 有声资源url
 * @toneurl: 提示语url
 * @out: 音频信息,如:
 *    音频信息,如: {"album":"不可不信缘","duration":195,"output":"这就为你播放刘德华的怒剑啸狂沙","title":"怒剑啸狂沙"} 与AI response的out类似
 *    微聊信息,如:
 *     {
 *       “nickname”:"小明"，
 *       “type”: 0,
 *       "avatar": "http:xxxxxx.png", //头像
 *       “url”:"http://xxxx.mp3", //音频
 *       “pic”: "http://xxxx.png", //图片
 *       “video”:"http://xxxx.avr" //视频
 *       “emoji”:"xxx"， //表情
 *       “text”:"你好你好"，  //文字消息
 *       "timestamp": xxxxx //时间戳
 *     }
 * @seq: 音频在列表里的序号
 * @type: 音频类型,参考@audio_type
 * @chattype: 微聊的信息类型，参考@chat_type
 * @position: 默认0，表示audiourl从什么时间开始播放（断点续播）,单位秒
 * @audioid: 音频ID 
 * @eventid: 播放完音频结束后的动作处理(toneurl & audiourl同时存在，播放完最后一个)
 */
typedef struct {
    char *audiourl;
    char *toneurl;
    char *out;
    int type;
    int seq;
    int chattype;
    int position;
    char *audioid;
    int eventid;
} audio_info;

/*
 * 语音上传应答回调函数
 * @audio 返回的音频结果信息
 * @status 返回的状态 0: 无音频，1: 正常返回, 其他： 错误码
 **/
typedef void (*audio_resp_cb)(audio_info *audio, int status);

/**
 * @brief 初始化音频获取接口
 * @resp_cb 获取音频结果回调函数
 * @return 0: success, -1: fail
 */
int coapi_audio_init(audio_resp_cb cb);


/**
 * @brief 反初始化音频获取接口
 */
 int coapi_audio_deinit(void);

/**
 * @brief 音频获取接口
 * @type 音频的类型,参考@audio_type,当前播放的列表使用0
 * @event 获取事件,参考@audio_event
 * @sed 请求列表里的第几首音频, 当event＝EVENT_MEDIA_SEED时生效,否则置为-1
 * @automode　用于单曲循环模式下,是播放结束后自动切换的下一曲还是手动操作进入下一曲
 *            automode = 1: 单曲循环模式 —— 自动切  =>  播放原来的歌曲
 *            automode = 0: 单曲循环模式 —— 手动切  =>  播放下一曲
 * @return 0: success, -1: fail
 **/
int coapi_audio_start(int type, int event,int index, int automode);

/**
 * @brief 每日推送，开机推送播放,建议在开机注册完成后调用
 * @return
 *   0: success
 *   -1: fail
 */
int coapi_daily_play(void);

/**
 * @brief 请求当前播放音频的 上一曲
 * @return
 *   0: success
 *   -1: fail
 */
int coapi_audio_prev(void);

/**
 * @brief 请求当前播放音频的 下一曲
 * @return
 *   0: success
 *   -1: fail
 */
int coapi_audio_next(void);

/**
 * @brief 用于单曲循环模式下,音频自动播放结束后,调用此接口云端会继续返回原音频
 * 播放模式状态可以参考coapi_app/src/coapi_comm.c　media_callback获取
 * 当状态有变化时会通过此接口回调回来
 *
 * @return
 *   0: success
 *   -1: fail
 */
int coapi_audio_single_next(void);

/**
 * @brief 根据index请求云端列表里面的某一首音频
 * @return
 *   0: success
 *   -1: fail
 */
int coapi_audio_seed(int index);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
