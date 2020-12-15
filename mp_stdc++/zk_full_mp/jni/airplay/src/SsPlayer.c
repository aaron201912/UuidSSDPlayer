#include "airplayer.h"

//sdk audio input/outout param
#define     AUDIO_INPUT_SAMPRATE       48000
#define     AUDIO_INPUT_CHLAYOUT       AV_CH_LAYOUT_MONO
#define     AUDIO_INPUT_SAMPFMT        AV_SAMPLE_FMT_S16

#define     AUDIO_OUTPUT_SAMPRATE       E_MI_AUDIO_SAMPLE_RATE_44100
#define     AUDIO_OUTPUT_CHLAYOUT       E_MI_AUDIO_SOUND_MODE_MONO
#define     AUDIO_OUTPUT_SAMPFMT        E_MI_AUDIO_BIT_WIDTH_16

//func return value
#define     SUCCESS     0
#define     FAIL        1

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }\
    else\
    {\
        printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__);\
    }

#define ExecFunc(func, _ret_) \
    printf("%d Start test: %s\n", __LINE__, #func);\
    if (func != _ret_)\
    {\
        printf("AO_TEST [%d] %s exec function failed\n",__LINE__, #func);\
        return 1;\
    }\
    else\
    {\
        printf("AO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);

#define MI_AUDIO_SAMPLE_PER_FRAME 1024

int Ss_AO_Deinit(void)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

    /* disable ao channel of */
    ExecFunc(MI_AO_DisableChn(AoDevId, AoChn), MI_SUCCESS);

    /* disable ao device */
    ExecFunc(MI_AO_Disable(AoDevId), MI_SUCCESS);

    return 0;

}
int Ss_AO_Init(void)
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = AUDIO_OUTPUT_SAMPFMT;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.u32ChnCnt = 2;
    if(stSetAttr.u32ChnCnt == 2)
    {
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    }
    else if(stSetAttr.u32ChnCnt == 1)
    {
        stSetAttr.eSoundmode = AUDIO_OUTPUT_CHLAYOUT;
    }

    stSetAttr.eSamplerate = AUDIO_OUTPUT_SAMPRATE;

    /* set ao public attr*/
    ExecFunc(MI_AO_SetPubAttr(AoDevId, &stSetAttr), MI_SUCCESS);

    /* get ao device*/
    ExecFunc(MI_AO_GetPubAttr(AoDevId, &stGetAttr), MI_SUCCESS);

    /* enable ao device */
    ExecFunc(MI_AO_Enable(AoDevId), MI_SUCCESS);

    /* enable ao channel of device*/
    ExecFunc(MI_AO_EnableChn(AoDevId, AoChn), MI_SUCCESS);

    /* if test AO Volume */
    s32SetVolumeDb = -10;
    ExecFunc(MI_AO_SetVolume(AoDevId, s32SetVolumeDb), MI_SUCCESS);
    ExecFunc(MI_AO_SetVolume(AoDevId, s32SetVolumeDb), MI_SUCCESS);
    /* get AO volume */
    ExecFunc(MI_AO_GetVolume(AoDevId, &s32GetVolumeDb), MI_SUCCESS);

    return 0;
}


