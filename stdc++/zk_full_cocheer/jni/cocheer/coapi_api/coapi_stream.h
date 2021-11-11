#ifndef _COAPI_STREAM_H_
#define _COAPI_STREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*  STREAM回应类型 */
typedef enum {
    STREAM_RSQ_ERR = -1,
    STREAM_RSQ_MID = 0,
    STREAM_RSQ_FINAL,
} stream_rsp_id;

/* 录音状态  */
typedef enum {
	RECORD_CONTINUES = 0,
	RECORD_END       = 1,
	RECORD_CANCEL    = 2,
} record_status;

/**
 * @brief 针对音频流,vad配置情况
 * @ai_no_vad: 不使用vad
 * @ai_vad: 使用设备本地vad
 * @engine_vad: 云端通用vad
 * @asr_vad: 云端asr一体化vad
 */
typedef enum {
	AI_NO_VAD = 0,
	AI_VAD,
	ENGINE_VAD,
	ASR_VAD,
} conf_vad;

typedef enum {
   VAD_INIT         = 1, 	/* 静音阶段 */
   VAD_VOICE		= 2,	/* 语音阶段 */
   VAD_STOP 		= 3,	/* 结束阶段 */
} vad_state;

/* STREAM数据格式 */
typedef enum {
	TYPE_PCM = 0,
	TYPE_AMR,
	TYPE_WNVC,
	TYPE_SBC,
	TYPE_ATSOPUS,
	TYPE_OGG,
	TYPE_SPEEX,
	TYPE_RAW,
	TYPE_OPUS,
} stream_format;

/**
 * @brief STREAM状态
 * @idle 引擎已经初始化,还未上传数据或者数据上传结束等待结果返回
 * @run 正在上传数据
 * @stop 没有进行STREAM应用
 */
typedef enum {
    FEED_IDLE = 0,
    FEED_RUN,
    FEED_STOP,
} state_t;

/**
 * @breif 上传语音的功能定义
 * @speech AI对话
 * @wechat 微聊
 * @trasce 中译英
 * @trasec 英译中
 * @engeva 英语口语评测
 * @listen 单向聆听
 * @brawave 脑电
 * @text 文本对话
 */
typedef enum {
    FUNC_SPEECH = 0,
    FUNC_WECHAT,
    FUNC_TRASCE,
    FUNC_TRASEC,
    FUNC_ENGEVA,
    FUNC_LISTEN,
    FUNC_BRAWAVE,
    FUNC_TEXT,
} func_type;

/**
 * @breif 返回结果定义
 * @sds 对话内容
 * @asr 识别结果
 * @nlu nlu
 * @nlu_sds nlu+对话均有
 * @null 无所谓返回，比如发送微聊消息
 */
typedef enum {
    RESULT_SDS = 0,
    RESULT_ASR,
    RESULT_NLU,
    RESULT_NLU_SDS,
    RESULT_NULL,
} result_para;

/**
 * @offset: 发送音频的偏移量
 * @cid: 客户端消息id
 * @sid: 服务端消息id
 * @type: data应用场景数据类型, 如type=1,data为实时asr流
 * @data: 数据
 * @data_len: 数据长度
 * @vad: 云端vad,状态结果
 */
typedef struct {
    int offset;
    int cid;
    int sid;
    int type;
    char *data;
    int data_len;
    int vad;
} stream_req;

/**
 * @domain: 技能ID
 * @action: 命令
 * @data: 音频流/序列化数据点
 * @audiourl: 有声资源url
 * @toneurl: 提示语url
 * @audioid: 音频id
 * @positon: 默认0表示从头开始播放，从position时间开始播放，用于断点续播
 * @eventid: 播放完音频结束后的动作处理(toneurl & audiourl同时存在，播放完最后一个)
 */
typedef struct {
    char *domain;
    char *data;
    char *audiourl;
    char *toneurl;
    int  action;
    int  param;
    char *in;
    char *out;
    char *audioid;
    int  position;
    int eventid;
    char *nlu;
    int data_len;
	unsigned char has_sid;
	int sid;
} stream_t;

/* 语音上传应答回调函数 */
typedef void (*stream_resp_cb)(void *arg, int type);

/*
 * @brief 释放ai_normal 结构体
 * @nor 要释放的结构体
 */
void coapi_stream_free(stream_t *nor);

/**
 * @brief ai_normal结构体拷贝内容拷贝
 * @dst 目标结构体
 * @src 源结构体
 */
void coapi_stream_cpy(stream_t *dst,stream_t *src);

/**
 * @brief 初始化语音上传功能功能，一次上传调用一次
 * @resp_cb 语音上传结果回调函数
 * @func 语音上传的功能定义,参考@func_type
 * @result 结果返回定义,参考@result_para
 * @vad 是否使用vad,参考@conf_vad
 * @stream_format 参考@stream_format,　指传入coapi_stream_feed接口的数据格式
 */
int coapi_stream_init(stream_resp_cb resp_cb,int func, int result,int vad,int stream_format);

/**
 * @brief 文本对话
 * @resp_cb 文本上传结果回调函数
 * @text 传入文本内容(如: asr识别结果)
 * @param 结果返回定义,参考@result_para
 */
int coapi_text_dial(stream_resp_cb resp_cb,char *text, int param);

/**
 * @brief 上传语音函数
 * 音频stream,格式要求：16KHZ 16bit 单声道 PCM数据
 * @data 数据
 * @size 数据大小，必须是256的整数倍，一次不能传超过3072，推荐2560
 * @eof 0: 继续传入数据 1:表示此次对话最后一次调用此接口
 * @return: 当使用vad时,-1: 接口调用出错,其他参考＠vad_state enum定义; 没有使用vad时，1: 正常返回，-1: 接口调用出错
 */
int coapi_stream_feed(char *data,int size, int eof);

/**
 * @brief 返回STREAM的状态,参考@state_t
 * @return 状态
 */
state_t coapi_stream_status(void);

/**
 * @brief 取消本轮对话,取消后不会继续返回结果
 * @return
 */
void coapi_stream_cancel(void);

/**
 * 设置vad参数
 * lowlimit   低于该值，表示静音阶段
 * hightlimit   高于该值，表示语音阶段
 */
void coapi_stream_setvad_threshold(unsigned int lowlimit, unsigned int hightlimit);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
