#ifndef __COAPI_TTS_H_
#define __COAPI_TTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TTS回调函数
 * @data TTS结果,URL，
 */
typedef void (*tts_rsp_cb)(int status,char *data);

/**
 * @brief tts 合成请求接口
 * @cb tts结果回调函数
 * @ttsid 指定的tts合成音。0 为使用产品本身的TTS
 * @text 需要合成的文本
 * @return  -1: fail   0: success
 */
int coapi_tts_start(tts_rsp_cb cb, int ttsid, char *text);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
