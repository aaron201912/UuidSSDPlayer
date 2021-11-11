#ifndef _COAPI_FUNCHUB_H_
#define _COAPI_FUNCHUB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 对所有功能进行统一定义，并且唯一ID标识，
 * 包括端云交互的功能，音频请求的功能，本地的功能, 运营功能等
 *
 * 本地功能：FUNC_ID_LOC_XXX
 * 端云交互：FUNC_ID_ECI_XXX
 * 音频请求：FUNC_ID_AREQ_XXX
 * 运营服务：FUNC_ID_OPT_SERVICE
 * 视频请求：FUNC_ID_VREQ_XXX
 *
 * connect:                 本地功能： 联网
 * aivoice:                 端云交互： 对话
 * entoch:                  端云交互： 英翻中
 * chtoen:                  端云交互： 中翻英
 * wechat:                  端云交互： 微聊
 * volumeup:                本地功能： 音量加
 * volumedown:              本地功能： 音量减
 * prev:                    本地功能： 上一曲
 * next:                    本地功能： 下一曲
 * pause:                   本地功能： 暂停
 * enevalu:                 端云交互： 英文口语评测
 * snooze:                  端云交互： 贪睡
 * weather:                 端云交互： 天气信息
 * date:                    端云交互： 日期时间
 * story:                   音频请求:  故事
 * song:                    音频请求:  儿歌
 * english:                 音频请求:  英语
 * poetry:                  音频请求:  唐诗
 * sinology:                音频请求:  国学
 * habit:                   音频请求:  习惯
 * reset:                   端云交互： 设备端发起恢复出厂设置，通知云端进行解绑，清除用户相关信息
 * collect:                 端云交互： 收藏/删除收藏音频功能, 收藏当前播放的音频，收藏后在手机端(公众号/app)查看; 如果播放的是收藏列表的音频，在播放期间执行此接口则会从收藏列表删除当前的音频
 *                                    可以通过传入FUNC_ID与序号来指定具体分类以及从第几首开始播放： 如：data: {"funcid": xx, "index": xx } 这个参数非必须，如果没有传默认第一个分类第一曲开始。兼容之前只有“我的收藏”一个分类的做法
 * daliymedia:              音频请求： 每天随机的音频播放
 * playcollect:             端云交互： 播放收藏的内容
 * classical:               音频请求:  古典音乐
 * popular:                 音频请求:  流行音乐
 * crosstalk:               音频请求:  相声
 * sketches:                音频请求:  小品
 * opera:                   音频请求:  戏曲
 * audiobook:               音频请求:  有声书
 * history:                 音频请求:  历史
 * health:                  音频请求:  健康养生
 * edu chinese:             音频请求:  语文教材
 * edu math :               音频请求:  数学教材
 * edu english ：           音频请求:  英语教材
 * edu physics ：           音频请求:  物理教材
 * edu chemistry textbook： 音频请求:  化学教材
 * edu biology textbook：   音频请求:  生物教材
 * edu history  textbook：  音频请求:  历史教材
 * edu geography textbook： 音频请求:  地理教材
 * edu politics textbooks： 音频请求:  政治教材
 * eng_dlg:                 端云交互： 英文对话 
 * aerq_news:               音频请求:  热点新闻
 * radio：                  音频请求:  电台, 电台参数按需选用 data {"类型":"国家台/省台/网络台", "省级": "广东","市级":"深圳","县级":"", "台名": "深圳交通台"} 不存在的参数留空; 通过data参数指定台名获取
 * content：                音频请求:  根据音频ID获取内容
 * push：                   端云交互： 关手机推送到设备端播放
 * poweroff：               端云交互： 关机功能
 * audiores：               音频资源
 * alarmlist：              端云交互： 闹钟列表信息,获取本台设备被设置的的所有闹钟列表信息(包括已关闭，但不包括已删除的)
 * transl_tts:              端云交互： 输入文本,翻译成某个国家的语言并且TTS播报出来
 * opt_service:             运营功能
 */
typedef enum {
   FUNC_ID_LOC_CONNECT = 0,
   FUNC_ID_ECI_AIVOICE,
   FUNC_ID_ECI_ENTOCH,
   FUNC_ID_ECI_CHTOEN,
   FUNC_ID_ECI_WECHAT,
   FUNC_ID_LOC_VOLUMEUP,
   FUNC_ID_LOC_VOLUMEDOWN,
   FUNC_ID_LOC_PREV,
   FUNC_ID_LOC_NEXT,
   FUNC_ID_LOC_PAUSE,
   FUNC_ID_ECI_ENEVALU,
   FUNC_ID_ECI_SNOOZE,
   FUNC_ID_ECI_WEATHER = 14,
   FUNC_ID_ECI_DATE,
   FUNC_ID_AREQ_STORY,
   FUNC_ID_AREQ_SONG,
   FUNC_ID_AREQ_ENGLISH,
   FUNC_ID_AREQ_POETRY,
   FUNC_ID_AREQ_SINOLOGY,
   FUNC_ID_AREQ_HABIT,
   FUNC_ID_ECI_RESET,
   FUNC_ID_ECI_COLLECT,
   FUNC_ID_ECI_DALIYMEDIA = 24,
   FUNC_ID_ECI_PLAYCOLLECT,
   FUNC_ID_AREQ_CLASSICAL,
   FUNC_ID_AREQ_POPULAR,
   FUNC_ID_AREQ_CROSSTALK,
   FUNC_ID_AREQ_SKETCHES,
   FUNC_ID_AREQ_OPERA,
   FUNC_ID_AREQ_AUDIOBOOK,
   FUNC_ID_AREQ_HISTORY,
   FUNC_ID_AREQ_HEALTH,
   FUNC_ID_AREQ_EDU_CHINESE = 34,
   FUNC_ID_AREQ_EDU_MATH,
   FUNC_ID_AREQ_EDU_ENGLISH,
   FUNC_ID_AREQ_EDU_PHYSICS,
   FUNC_ID_AREQ_EDU_CHEMISTRY,
   FUNC_ID_AREQ_EDU_BIOLOGY,
   FUNC_ID_AREQ_EDU_HISTORY,
   FUNC_ID_AREQ_EDU_GEOGRAPHY,
   FUNC_ID_AREQ_EDU_POLITICS,
   FUNC_ID_ECI_ENGDLG,
   FUNC_ID_AREQ_NEWS = 44,
   FUNC_ID_AREQ_RADIO,
   FUNC_ID_AREQ_CONTENT,
   FUNC_ID_LOC_POWEROFF,
   FUNC_ID_ECI_AUDIORES,
   FUNC_ID_ECI_ALARMLIST,
   FUNC_ID_ECI_TRANSL_TTS,
   FUNC_ID_OPT_SERVICE,
   
   FUNC_ID_XXX_XXX_END
} func_hub_id;

/**
 * @func: 功能id
 * @data: 对应功能id的数据json
 */
typedef struct {
    int func;
    char *data;
} funchub_info;

/*
 * 请求功能回调函数
 * @info 返回功能信息
 * @return
 **/
typedef void (*funchub_resp_cb)(funchub_info *info);

/**
 * @brief　功能请求接口
 * @func  要请求的功能ID
 * @data  reserve 参数
 * @return 0: success, -1: fail
 **/
int coapi_funchub_start(int func, char *data);

/**
 * @brief　功能请求接口初始化
 * @cb  请求的功能回调函数
 **/
void coapi_funchub_init(funchub_resp_cb cb);
int coapi_funchub_deinit(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
