/*
 * audioplayer.c
 *
 *  Created on: 2020年1月2日
 *      Author: koda.xu
 */

#include <stdio.h>
#include <string.h>
#include "mi_ao.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "audioplayer.h"


#ifndef ExecFunc
#define ExecFunc(_func_, _ret_) \
    do{ \
        MI_S32 s32Ret = MI_SUCCESS; \
        s32Ret = _func_; \
        if (s32Ret != _ret_) \
        { \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret; \
        } \
        else \
        { \
            printf("[%s %d]exec function pass\n", __func__, __LINE__); \
        } \
    } while(0)
#endif

#define MI_AUDIO_SAMPLE_PER_FRAME 1024
#define MI_AO_QUEUE_SIZE    (50 * 1024)

static MI_S32 g_AoDevId = 0;
static MI_S32 g_AoChn = 0;
static MI_S32 g_s32NeedSize = 0;	// no needed

int AudioPlayerDeinit(void)
{
    ExecFunc(MI_AO_DisableChn(g_AoDevId, g_AoChn), MI_SUCCESS);
    ExecFunc(MI_AO_Disable(g_AoDevId), MI_SUCCESS);

    return 0;
}

int AudioPlayerInit(int chnCnt, int nBits, int nSampleRate, int s32AoVolume)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Attr_t stAoSetAttr, stAoGetAttr;
    MI_AO_AdecConfig_t stAoSetAdecConfig, stAoGetAdecConfig;
    MI_AO_VqeConfig_t stAoSetVqeConfig, stAoGetVqeConfig;
    MI_S32 s32AoGetVolume;
    MI_AO_ChnParam_t stAoChnParam;
    MI_U32 u32DmaBufSize;
    MI_AUDIO_SampleRate_e eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_INVALID;

    memset(&stAoSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));

    if (chnCnt != 1)
    {
    	printf("only support mono mode\n");
    	return -1;
    }

    switch (nBits)
    {
    case 16:
    	stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    	break;
    case 24:
    	stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_24;
    	break;
    default:
    	printf("only support 16 & 24 bits\n");
    	return -1;
    }

    switch (nSampleRate)
    {
    case 8000:
    	stAoSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_8000;
    	eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
		break;
	case 16000:
		stAoSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
		eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
		break;
	default:
		printf("samplerate only support 8000 & 16000\n");
		return -1;
    }

    stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAoSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    stAoSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stAoSetAttr.u32ChnCnt = 1;
    stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
//    stAoSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
//    eAoInSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;

    stAoSetVqeConfig.bAgcOpen = FALSE;
    stAoSetVqeConfig.bAnrOpen = FALSE;
    stAoSetVqeConfig.bEqOpen = FALSE;
    stAoSetVqeConfig.bHpfOpen = FALSE;
    stAoSetVqeConfig.s32FrameSample = 128;
    stAoSetVqeConfig.s32WorkSampleRate = eAoInSampleRate;

    ExecFunc(MI_AO_SetPubAttr(g_AoDevId, &stAoSetAttr), MI_SUCCESS);
    ExecFunc(MI_AO_GetPubAttr(g_AoDevId, &stAoGetAttr), MI_SUCCESS);
    ExecFunc(MI_AO_Enable(g_AoDevId), MI_SUCCESS);
    ExecFunc(MI_AO_EnableChn(g_AoDevId, g_AoChn), MI_SUCCESS);

    if(FALSE)
    {
        ExecFunc(MI_AO_SetVqeAttr(g_AoDevId, g_AoChn, &stAoSetVqeConfig), MI_SUCCESS);
        ExecFunc(MI_AO_GetVqeAttr(g_AoDevId, g_AoChn, &stAoGetVqeConfig), MI_SUCCESS);
        ExecFunc(MI_AO_EnableVqe(g_AoDevId, g_AoChn), MI_SUCCESS);
    }

    ExecFunc(MI_AO_SetVolume(g_AoDevId, s32AoVolume), MI_SUCCESS);
    ExecFunc(MI_AO_GetVolume(g_AoDevId, &s32AoGetVolume), MI_SUCCESS);

    g_s32NeedSize = MI_AUDIO_SAMPLE_PER_FRAME * 2 * (stAoSetAttr.u32ChnCnt);
    if (g_s32NeedSize > MI_AO_QUEUE_SIZE)
    	g_s32NeedSize = MI_AO_QUEUE_SIZE;

    return 0;
}

int AudioPlayerSendFrame(PlayAudioData_t* data)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;

	memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
	stAoSendFrame.u32Len = data->dwBufferLength * sizeof(SHORT);
	stAoSendFrame.apVirAddr[0] = data->lpsData;
	stAoSendFrame.apVirAddr[1] = NULL;

//	printf("AudioPlayerSendFrame: frameLen=%d\n", data->dwBufferLength);

	do{
		s32Ret = MI_AO_SendFrame(g_AoDevId, g_AoChn, &stAoSendFrame, 20);
	}while(s32Ret == MI_AO_ERR_NOBUF);

	if(s32Ret != MI_SUCCESS)
	{
		printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
	}

    return s32Ret;
}

int AudioPlayerPause()
{
	return MI_AO_PauseChn(g_AoDevId, g_AoChn);
}

int AudioPlayerResume()
{
	return MI_AO_ResumeChn(g_AoDevId, g_AoChn);
}

void AudioPlayerSetVolume(int vol)
{
    MI_AO_SetVolume(0, vol - 60);
}
