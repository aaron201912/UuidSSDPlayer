#ifndef _COAPI_SPEECH_H_
#define _COAPI_SPEECH_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*asr_text_output_func)(char *);
typedef void (*reply_text_output_func)(char *);

void coapi_speech_init(void);
void coapi_speech_start(void);
void coapi_speech_stop(void);
void voice_callback(void *arg,int type);


typedef enum SPEECH_STA_{
    SPEECH_STA_START = 0,
    SPEECH_STA_STOP,
    SPEECH_STA_END
}speech_status_t;

extern speech_status_t speech_status;


typedef enum COAPI_CLARA_PREE_RES_ENUM{
    COAPI_CLARA_PREE_RES_WAIT   = 0,                            /* µÈ´ýÇÀÕ¼ÏìÓ¦ */
    COAPI_PREE_RES,                                             /* CoAPIÇÀÕ¼ÏìÓ¦ */
    CLARA_PREE_RES,                                             /* ClaraÇÀÕ¼ÏìÓ¦ */
    COAPI_CLARA_PREE_RES_END
}coapi_clara_pree_res_t;                                        /* coapi clara ÏìÓ¦ÇÀÕ¼×´Ì¬ */

extern coapi_clara_pree_res_t          coapi_clara_pree_res;

#ifdef __cplusplus
} /* "C" */
#endif

#endif
