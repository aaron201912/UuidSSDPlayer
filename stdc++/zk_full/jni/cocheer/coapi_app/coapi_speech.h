#ifndef _COAPI_SPEECH_H_
#define _COAPI_SPEECH_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*asr_text_output_func)(char *);
typedef void (*reply_text_output_func)(char *);

void coapi_speech_start(void);
void coapi_speech_stop(void);
void voice_callback(void *arg,int type);

void cb_flash_speek_text(char* text);
void cb_flash_ansr_text(char* text);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
