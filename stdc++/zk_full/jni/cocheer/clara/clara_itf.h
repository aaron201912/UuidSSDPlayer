
/***********************************************************************

    Copyright (C) 2019-2020 Vox-AI, Inc.
    All rights reserved

***********************************************************************/   

/*!
 * \file clara_itf.h
 * \brief
 *      Clara keyword search application interface declaration.
 *      Only mono-channel audio data in 16khz sampling rate and 16bit size is supported.
 *      
 */
#ifndef CLARA_ITF_
#define CLARA_ITF_

#ifdef __cplusplus
extern "C" {
#endif

/* Common constant definition*/
#define MS_PER_FRAME 16                         /* Length of input audio frame in ms */
#define SAMPLES_PER_FRAME (MS_PER_FRAME * 16)   /*  Sample numbers of input audio frame */
#define MEM_POOL_SIZE (140 * 1024)              /* KB */
#define DEFAULT_DURATION_IN_SEC_AFTER_WUW (7)   /* 7s */

/* Models supported in this release*/
typedef enum clara_model_type {
    e_wuw,
    e_cmd,
    e_model_num
} Clara_model_type;

/* keyword result structure */
#define KWS_BUF_LEN 64      /* keyword buffer length (Do not change) */
typedef struct Clara_result
{
    char kws[KWS_BUF_LEN];  /* keyword by utf-8 encoding */
    int kws_len;            /* length of keyword by utf-8 encoding */
    int kws_id;             /* id of keyword */
    int score;              /* score of spotted keyword */
    int duration;           /* duration in ms of spotted keyword */
    int confidence;         /* confidence of spotted keyword */
    int digit;              /* store digit if any found in keyword */    
} Clara_result_t;

/* User can use pre-defined clara_result struct to check keyword info*/
extern Clara_result_t clara_result; 
/* PS: this pre-defined variable is only available when using static clara lib. User shall define 
       own clara_result when invoking shared clara lib (so) */
       

/*! \brief Create a Clara instance with user provided memory pool.
    \param[in] p_mem_pool - The memory pool is a user provided pointer of a memory buffer which is used by 
               Clara internally. 
    \param[in] mem_pool_size - The size of memory pool in bytes
    \param[in] samples_per_frame - The number of samples in a frame
    \param[in] active_time_after_wakeup - Set the maximum duration (unit: second) after Clara wakeup.
               This parameter is only valid when Clara works in two-phases mode, i.e. invoking 
               Clara_put_audio_two_phases().
    \return int - nonezero value if any error
*/
int Clara_create(void * p_mem_pool, int mem_pool_size, int samples_per_frame, int active_time_after_wakeup);


/*! \brief Free all memories used by a Clara instance.
    \return int - nonezero value if any error
*/
int Clara_destruct(void);


/*! \brief Switch Clara internal working model to detection specified keyword group
    \param[in] model_type - Model type enumeration variable 
    \return int - nonezero value if switching model fail
*/
int Clara_model_switch(Clara_model_type model_type);


/*! \brief Use this interface to query Clara version information
    \return char * - string of Clara version information
*/
char *Clara_get_version(void);


/*! \brief Pass in audio samples to Clara, and if none zero result returned, user can retrieve the returned 
           keyword detection result directly.
    \param[in] p_audio_frame - Pointer to a frame of audio sample data buffer
    \param[in] frame_size - The number of audio samples in a frame
    \param[out] result - This struct stored the keyword detection result of Clara. It is only valid when 
                the return value is none zero. User can pass in the address of predefined clara_result.
    \return int - nonezero value if any keyword detected
*/
int Clara_VAD_put_audio(short * p_audio_frame, int frame_size, Clara_result_t *result);


/*! \brief Pass in audio samples to Clara, this interface works in two phases. At the 1st phase, it detects 
           specified wakeup word only, and if wakeup word is detected, it continues detect command word 
           in the 2nd phase, until it reaches the timeout setting in creation. Then it goes back to detect 
           wakeup word only as in the 1st phase.
    \param[in] p_audio_frame - Pointer to a frame of audio sample data buffer
    \param[in] frame_size - The number of audio samples in a frame
    \param[out] result - This struct stored the keyword detection result of Clara. It is only valid when 
                the return value is none zero. User can pass in the address of predefined clara_result.
    \return int - nonezero value if any keyword detected
*/
int Clara_put_audio_two_phases(short * p_audio_frame, int frame_size, Clara_result_t *result);


#define FP_MIC_CHANNEL_NUM 2
#define FP_REF_CHANNEL_NUM 1
#define FP_OUT_CHANNEL_NUM 1


/*! \brief Create a Clara front processing instance 
    \return int - nonezero value if any error
*/
int Clara_create_FP(void);


/*! \brief Pass in audio samples to Clara, which should contain all microphone and reference audio
           with channel numbers specified by above macros.  
    \param[in] p_audio_inframe - Pointer to a frame of audio sample data buffer which contains all mic/ref data
    \param[in] frame_size - The number of audio samples in a frame for each mic/ref channel 
    \param[out] p_audio_outframe - Pointer to a processed output data buffer 
    \return int - nonezero value if any keyword detected*/
int Clara_FP_enhance(short * p_audio_inframe, int frame_size, short *p_audio_outframe);


/*! \brief Pass in audio samples to Clara, this interface works in two phases. At the 1st phase, it detects 
           specified wakeup word only, and if wakeup word is detected, it continues detect command word 
           in the 2nd phase, until it reaches the timeout setting in creation. Then it goes back to detect 
           wakeup word only as in the 1st phase.
    \param[in] p_audio_frame - Pointer to a frame of audio sample data buffer
    \param[in] frame_size - The number of audio samples in a frame
    \param[out] result - This struct stored the keyword detection result of Clara. It is only valid when 
                the return value is none zero. User can pass in the address of predefined clara_result.
    \return int - nonezero value if any keyword detected
*/
int Clara_FP_put_audio_two_phases(short * p_audio_frame, int frame_size, Clara_result_t *result);


/*! \brief Free all memories used by a Clara front processing instance.
    \return int - nonezero value if any error
*/
int Clara_FP_destruct(void);


/*! \brief Create a Clara instance enabled front processing unit with user provided memory pool.
    \param[in] p_mem_pool - The memory pool is a user provided pointer of a memory buffer which is used by 
               Clara internally. 
    \param[in] mem_pool_size - The size of memory pool in bytes
    \param[in] samples_per_frame - The number of samples in a frame
    \param[in] active_time_after_wakeup - Set the maximum duration (unit: second) after Clara wakeup.
               This parameter is only valid when Clara works in two-phases mode, i.e. invoking 
               Clara_put_audio_two_phases().
    \return int - nonezero value if any error
*/
int Clara_create_with_FP(void * p_mem_pool, int mem_pool_size, int samples_per_frame, int active_time_after_wakeup);


/*! \brief Free all memories used by a Clara instance enabled with front processing 
    \return int - nonezero value if any error
*/
int Clara_destruct_with_FP(void);


/*! \brief Pass in audio samples to Clara, which should contain all microphone and reference audio
           with channel numbers specified by above macros. If none zero result returned, user can 
           retrieve the returned keyword detection result directly. 
    \param[in] p_audio_frame - Pointer to a frame of audio sample data buffer which contains all mic/ref data
    \param[in] frame_size - The number of audio samples in a frame for each mic/ref channel 
    \param[out] result - This struct stored the keyword detection result of Clara. It is only valid when 
                the return value is none zero. User can pass in the address of predefined clara_result.
    \return int - nonezero value if any keyword detected
*/
int Clara_FP_put_audio(short * p_audio_frame, int total_frame_size, Clara_result_t *result);


/*! \brief Create a multi-threaded Clara instance.
    \param[in] p_mem_pool - The memory pool is a user provided pointer of a memory buffer which is used by 
               Clara internally. 
    \param[in] mem_pool_size - The size of memory pool in bytes
    \param[in] samples_per_frame - The number of samples in a frame
    \param[in] active_time_after_wakeup - Set the maximum duration (unit: second) after Clara wakeup.
               This parameter is only valid when Clara works in two-phases mode, i.e. invoking 
               Clara_put_audio_two_phases().
    \param[in] callback_wakeup_inform - Set the callback function which will be invoked by Clara to inform
               user when keyword is detected and the details is stored in its parameter p_result.
    \param[in]  p_result - set the Clara wakeup result pointer, which will be set when keyword detected, and pass
                back to user in the callback function.               
    \return int - nonezero value if any error
*/
int Threaded_Clara_create(void * p_mem_pool, int mem_pool_size, int samples_per_frame,
	int active_time_after_wakeup, void(*callback_wakeup_inform)(Clara_result_t *p_result), Clara_result_t *p_result);


/*! \brief Pass in mic & ref audio samples to threaded Clara and retrieve processed audio samples of Clara.
    \param[in] p_mic_frame - Pointer to multi-channel of microphone audio sample data buffers
    \param[in] mic_num - Set the number of microphone audio channels
    \param[in] p_ref_frame - Pointer to multi-channel of reference audio sample data buffers
    \param[in] ref_num - Set the number of reference audio channels
    \param[in] frame_size - The number of mic/ref audio samples in a frame
    \param[out] p_out_frame - Pointer to a buffer which provided by user to store Clara output audio 
                with signal enhancement 
    \param[out] out_frame_size - The number of audio samples in p_out_frame buffer
    \return int - nonezero value if any error
*/
int Threaded_Clara_put_audio(short ** p_mic_frame, int mic_num, short ** p_ref_frame, int ref_num,
	int frame_size, short *p_out_frame, int *out_frame_size);


#ifdef __cplusplus
}
#endif

#endif /* CLARA_ITF_ */

