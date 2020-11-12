/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef __AUDIO_H_
#define __AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "mi_common_datatype.h"
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"

void cb_stop_record(void);		/* 在mainLogic.cc实现 */

#if defined(HDMI)
#include "mi_hdmi.h"
#endif

#define WAV_G711A 					(0x06)
#define WAV_G711U 					(0x07)
#define WAV_G726 					(0x45)
#define WAV_PCM  					(0x1)

#define G726_16 					(2)
#define G726_24 					(3)
#define G726_32 					(4)
#define G726_40 					(5)

#define AI_AMIC_CHN_MAX 	        (2)
#define AI_DMIC_CHN_MAX		        (4)
#define AI_I2S_NOR_CHN 		        (2)
#define AI_LINE_IN_CHN_MAX 	        (2)
#define AI_AMIC_AND_I2S_RX_CHN_MAX 	(4)
#define AI_DMIC_AND_I2S_RX_CHN_MAX 	(4)

#define AI_DEV_ID_MAX   			(6)
#define AO_DEV_ID_MAX   			(2)

#define AI_DEV_AMIC             	(0)
#define AI_DEV_DMIC             	(1)
#define AI_DEV_I2S_RX           	(2)
#define AI_DEV_LINE_IN          	(3)
#define AI_DEV_AMIC_AND_I2S_RX  	(4)
#define AI_DEV_DMIC_AND_I2S_RX  	(5)

#define AO_DEV_LINE_OUT         	(0)
#define AO_DEV_I2S_TX           	(1)

#if defined(HDMI)
#define AO_DEV_HDMI   				(2)
#define AO_DEV_HDMI_AND_LINEOUT 	(3)
#endif

#define AI_VOLUME_AMIC_MIN      	(0)
#define AI_VOLUME_AMIC_MAX      	(21)
#define AI_VOLUME_LINEIN_MAX    	(7)

#define AI_VOLUME_DMIC_MIN      	(0)
#define AI_VOLUME_DMIC_MAX      	(4)

#define AO_VOLUME_MIN           	(-60)
#define AO_VOLUME_MAX           	(30)

/* #define MI_AUDIO_SAMPLE_PER_FRAME	(512) */
 
#define DMA_BUF_SIZE_8K     		(8000)
#define DMA_BUF_SIZE_16K    		(16000)
#define DMA_BUF_SIZE_32K    		(32000)
#define DMA_BUF_SIZE_48K    		(48000)

#define AI_DMA_BUFFER_MAX_SIZE		(256 * 1024)
#define AI_DMA_BUFFER_MID_SIZE		(128 * 1024)
#define AI_DMA_BUFFER_MIN_SIZE		(64 * 1024)

#define AO_DMA_BUFFER_MAX_SIZE		(256 * 1024)
#define AO_DMA_BUFFER_MID_SIZE		(128 * 1024)
#define AO_DMA_BUFFER_MIN_SIZE		(64 * 1024)

#define MIU_WORD_BYTE_SIZE			(8)
#define TOTAL_BUF_DEPTH				(8)
#define DEFAULT_AO_VOLUME           (-10) /* 取值范围： [-60,30]*/

#define COCHEER_AUDIO_DIR           "/customer/CoAPI/"
#define DOWNLOAD_TONE               "/customer/CoAPI/download_tone.mp3"
#define DOWNLOAD_AUDIO              "/customer/CoAPI/download_audio.mp3"
#define LEFT_PCM                    "/customer/CoAPI/left_pcm.pcm"
#define RIGHT_PCM                   "/customer/CoAPI/right_pcm.pcm"
#define AEC_PCM                     "/customer/CoAPI/aec_pcm.pcm"

typedef enum
{
	E_AI_SOUND_MODE_MONO = 0,           /* 单声道Mono */
	E_AI_SOUND_MODE_STEREO,             /* 双声道Stereo */
	E_AI_SOUND_MODE_QUEUE,              /* 队列流Queue */
} AiSoundMode_e;

#define ExecFunc(func, _ret_) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
        return 1;\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);	\
}while(0);

#define ExecFuncNoExit(func, _ret_, __ret) \
do{	\
	MI_S32 s32TmpRet;	\
    printf("%d Start test: %s\n", __LINE__, #func);\
    s32TmpRet = func;	\
    __ret = s32TmpRet;	\
    if (s32TmpRet != _ret_)\
    {\
        printf("AUDIO_TEST [%d] %s exec function failed, result:0x%x\n",__LINE__, #func, s32TmpRet);\
    }\
    else\
    {\
        printf("AUDIO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
        printf("%d End test: %s\n", __LINE__, #func);	\
    }\
}while(0);

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
typedef enum
{
    E_SOUND_MODE_MONO = 0,         /* mono 单声道*/
    E_SOUND_MODE_STEREO = 1,       /* stereo 双声道*/
} SoundMode_e;/*声道*/

typedef enum
{
    E_SAMPLE_RATE_8000  = 8000,    /* 8kHz sampling rate */
    E_SAMPLE_RATE_16000 = 16000,   /* 16kHz sampling rate */
    E_SAMPLE_RATE_32000 = 32000,   /* 32kHz sampling rate */
    E_SAMPLE_RATE_48000 = 48000,   /* 48kHz sampling rate */
} SampleRate_e;/*采样率*/

typedef enum
{
    E_AENC_TYPE_G711A = 0,
    E_AENC_TYPE_G711U,
    E_AENC_TYPE_G726_16,
    E_AENC_TYPE_G726_24,
    E_AENC_TYPE_G726_32,
    E_AENC_TYPE_G726_40,
    PCM,
} AencType_e;/*编码类型g711a/g711u/g726_16/g726_24/g726_32/g726_40/PCM*/

typedef struct WAVE_FORMAT
{
    signed short                wFormatTag;
    signed short                wChannels;
    unsigned int                dwSamplesPerSec;
    unsigned int                dwAvgBytesPerSec;
    signed short                wBlockAlign;
    signed short                wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char                        chRIFF[4];
    unsigned int                dwRIFFLen;
    char                        chWAVE[4];
    char                        chFMT[4];
    unsigned int                dwFMTLen;
    WaveFormat_t                wave;
    char                        chDATA[4];
    unsigned int                dwDATALen;
} WaveFileHeader_t;

typedef struct AiOutFilenName_s
{
    MI_S8                       *ps8OutputPath;
    MI_S8                       *ps8OutputFile;
    MI_S8                       *ps8SrcPcmOutputFile;
    MI_AUDIO_DEV                AiDevId;
    MI_AUDIO_Attr_t             stAiAttr;
    MI_BOOL                     bSetVqeWorkingSampleRate;
    MI_AUDIO_SampleRate_e       eVqeWorkingSampleRate;
    MI_BOOL                     bEnableRes;
    MI_AUDIO_SampleRate_e       eOutSampleRate;
    MI_BOOL                     bEnableAenc;
    MI_AI_AencConfig_t          stSetAencConfig;
    MI_AI_VqeConfig_t           stSetVqeConfig;
    MI_BOOL                     bEnableBf;
} AiOutFileName_t;

typedef struct AiChnPriv_s
{
    MI_AUDIO_DEV                AiDevId;
    MI_AI_CHN                   AiChn;
    // MI_S32                      s32Fd;
    MI_U32                      u32TotalSize;
    MI_U32                      u32SrcPcmTotalSize;
    MI_U32                      u32ChnCnt;
    MI_BOOL                     bEnableAed;
    pthread_t                   tid;
    MI_S32                      s32SrcPcmFd;
} AiChnPriv_t;

/*
 * 音频输入配置参数
 */
typedef struct audio_ai_cfg_s
{
    MI_BOOL                     bEnableAI;                  /* AI使能 标记 */
    MI_U32                      u32AiChnCnt;                /* AI 采音通道数 */
    MI_AUDIO_DEV                AiDevId;                    /* AI 设备描述符 */
    MI_U32                      u32AiRunTime;               /* 录音时间（秒数）*/
    //MI_U8*                      pu8AiOutputPath;            /* AI录音保存路径 */
    MI_BOOL                     bAiSetVolume;               /* AI SetVolume 使能 设定音量 标记 */
    MI_S32                      s32AiVolume;                /* AI 音量 */
    MI_U32                      u32MicDistance;             /* AI MIC 间距离，步长为1cm*/
    AiSoundMode_e               eAiSoundMode;               /* AI 采音模式(单声道Mono[0] 双声道Stereo[1] 队列流Queue[2]) */
    MI_AUDIO_SampleRate_e       eAiSampleRate;              /* AI 采样率 */
    MI_BOOL                     bAiEnableResample;          /* AI 重采样 使能 标记 */
    MI_AUDIO_SampleRate_e       eAiOutputResampleRate;      /* AI 重采样率 */
    MI_BOOL                     bAiEnableAec;               /* AI Aec 使能 自适应回声消除 标记 */   
    MI_BOOL                     bAiEnableAed;               /* AI Aed 使能 音频事件检测 标记                不常用 */
    MI_BOOL                     bAiEnableVqe;               /* AI Vqe 使能 语音质量增强 标记 */
    MI_BOOL                     bAiEnableHpf;               /* AI Hpf 使能 标记 */
    MI_BOOL                     bAiEnableNr;                /* AI Nr  使能 降噪 标记 */
    MI_BOOL                     bAiEnableAgc;               /* AI Agc 使能 自动增益控制 标记 */
    MI_BOOL                     bAiEnableEq;                /* AI Eq  使能 均衡器 标记 */
    MI_BOOL                     bAiEnableSsl;               /* AI Ssl 使能 标记 */
    MI_BOOL                     bAiEnableBf;                /* AI Bf  使能 标记 */
    MI_BOOL                     bAiSetBfDoa;
    MI_S32                      s32AiBfDoa;
    MI_BOOL                     bAiEnableAenc;              /* AI Aenc 使能 编码 标记 */
    MI_AUDIO_AencType_e         eAiAencType;                /* Ai Aenc 编码类型 g711a/g711u */
    MI_AUDIO_G726Mode_e         eAiAencG726Mode;            /* Ai Aenc 编码类型 g726_16/g726_24/g726_32/g726_40 */
    MI_U32                      u32VqeWorkingSampleRate;
    MI_BOOL                     bAiDumpPcmData;
}audio_ai_cfg_t;

/*
 * 音频输出配置参数
 */
typedef struct audio_ao_cfg_s
{
    MI_BOOL                     bEnableAO;                  /* AO使能 标记 */
    MI_AUDIO_DEV                AoDevId;                    /* AO设备描述符 */
    MI_U32                      u32AoChnCnt;                /* AO输出通道数 */
    MI_AO_CHN                   AoChn;                      /* AO 通道 */
    MI_S32                      s32AoVolume;                /* AO 音量 */
    MI_AUDIO_SampleRate_e       eAoOutSampleRate;           /* AO 输出采样率 */
    MI_BOOL                     bAoEnableHpf;
    MI_BOOL                     bAoEnableNr;
    MI_BOOL                     bAoEnableAgc;
    MI_BOOL                     bAoEnableEq;  
}audio_ao_cfg_t;

typedef enum{
    E_AMIC_MONO_MODE = 0,       /* 使用模拟MIC单声道模式/模拟单Mic + 回采  */
    E_DMIC_STEREO_MODE,         /* 使用数字MIC双声道模式/数字双Mic + 回采  */
} MicInputType_e;

/*
* 音频设定初始化
*/
MI_BOOL audioInit(MicInputType_e eMicType);
/*
* 音频设定去初始化
*/
MI_BOOL audioDeinit(void);

/*
* 获取AO 播放音量
*/
int audio_get_volume(void);
/*
* 设置AO 播放音量 0~10
*/
int audio_set_volume(int vol);
/*
* 获取AO 播放音量 单位 db
*/
int audio_get_volume_db(void);
/*
* 设置AO 播放音量 单位 db
*/
int audio_set_volume_db(int vol);
/******************************************************** 录音 播放相关 ****************************************************/
typedef struct chn_pcm_adta_s
{
    unsigned short *left_pcm_data_addr;
    unsigned short *right_pcm_data_addr;
    unsigned short *aec_pcm_data_addr;
}chn_pcm_adta_t;
typedef void (*output_chn_pcm_data_cb)(chn_pcm_adta_t pcm_data_addr, unsigned int pcm_frame_len/*PCM帧长度*/, unsigned int chn/*通道数*/, unsigned char type/*数据结束标记*/, MicInputType_e eMicType/*mic输入音频类型*/);

/* start_ai 的参数 */
typedef struct start_ai_arg_type{
    output_chn_pcm_data_cb  pcm_output_cb;
}start_ai_arg_t;
/*
* ai 开启 （开始录音）
*/
void* start_ai(void* start_ai_arg);
/*
* ai 停止
*/
void stop_ai(void);
/******************************************************** MP3 播放相关 ****************************************************/
/*
* 播放mp3
*/
int mp3_play(char *mp3_file);
/*
* 停止播放mp3
*/
void mp3_play_stop(void);
/*
* 查询mp3是否处于播放状态
*/
int mp3_is_play(void);
/*
* 暂停MP3播放
*/
int mp3_play_push(void);
/*
* 回复MP3播放
*/
int mp3_play_resume(void);

/******************************************************** WAV 播放相关 ****************************************************/
int wav_play(char *wav_file);
/*
* 停止播放wav
*/
void wav_play_stop(void);
/*
* 查询wav是否处于播放状态
*/
int wav_is_play(void);
/*
* 暂停wav播放
*/
int wav_play_push(void);
/*
* 回复wav播放
*/
int wav_play_resume(void);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
