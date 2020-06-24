#if defined(SUPPORT_CLOUD_PLAY_MODULE) || defined(SUPPORT_PLAYER_MODULE)
#include "tp_player.h"
#include "player.h"
#include "frame.h"
#include "packet.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "mi_vdec_extra.h"

#define AUDIO_MAX_DATA_SIZE     25000
#define AUDIO_SAMPLE_PER_FRAME  1024

static int sstar_video_init(uint16_t x, uint16_t y, uint16_t dstWidth, uint16_t dstHeight)
{
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_DIVP_ChnAttr_t stDivpChnAttr;

    MI_SYS_ChnPort_t stDispChnPort;
    MI_SYS_ChnPort_t stDivpChnPort;

    //Init IDVP module
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = 0;
    stDivpChnAttr.stCropRect.u16Height  = 0;
    stDivpChnAttr.u32MaxWidth           = 1920;
    stDivpChnAttr.u32MaxHeight          = 1080;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = 0;
    stDivpChnPort.u32PortId             = 0;

    MI_DIVP_CreateChn(0, &stDivpChnAttr);
    MI_DIVP_StartChn(0);
    
    memset(&stDivpOutAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
    stDivpOutAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stDivpOutAttr.u32Width           = ALIGN_BACK(dstWidth , 32);
    stDivpOutAttr.u32Height          = ALIGN_BACK(dstHeight, 32);
    MI_DIVP_SetOutputPortAttr(0, &stDivpOutAttr);

    // Init DISP module
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
    stInputPortAttr.stDispWin.u16X          = x;
    stInputPortAttr.stDispWin.u16Y          = y;
    stInputPortAttr.stDispWin.u16Width      = dstWidth;
    stInputPortAttr.stDispWin.u16Height     = dstHeight;
    stInputPortAttr.u16SrcWidth             = ALIGN_BACK(dstWidth , 32);
    stInputPortAttr.u16SrcHeight            = ALIGN_BACK(dstHeight, 32);

    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                    = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId                  = 0;
    stDispChnPort.u32ChnId                  = 0;
    stDispChnPort.u32PortId                 = 0;

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    
    MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

    return 0;
}

static int sstar_video_deinit(void)
{
    MI_DISP_DEV dispDev = 0;
    MI_DISP_LAYER dispLayer = 0;
    MI_U32 u32InputPort = 0;

    MI_SYS_ChnPort_t stDispChnPort;
    MI_SYS_ChnPort_t stDivpChnPort;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = 0;
    stDivpChnPort.u32PortId             = 0;

    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = 0;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = 0;

    MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

    MI_DIVP_StopChn(0);
    MI_DIVP_DestroyChn(0);
    MI_DISP_DisableInputPort(dispLayer, u32InputPort);

    return 0;
}

static int sstar_buffer_putback(void *pData)
{
    MI_S32 s32Ret;
    AVFrame *pFrame = (AVFrame *)pData;

    if (pFrame->opaque)
    {
        SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)pFrame->opaque;
        //printf("frame->opaque addr : %p\n", pFrame->opaque);
        s32Ret = MI_SYS_ChnOutputPortPutBuf(stVdecBuf->stVdecHandle);
        if (MI_SUCCESS != s32Ret)
            printf("MI_SYS_ChnOutputPortPutBuf Failed!\n");
        return s32Ret;
    }

    return -1;
}

// MI display video
static int sstar_video_display(void *pData, bool bState)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t stInputChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    AVFrame *pFrame = (AVFrame *)pData;

    memset(&stInputChnPort , 0 , sizeof(MI_SYS_ChnPort_t));
    stInputChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stInputChnPort.u32ChnId  = 0;
    stInputChnPort.u32DevId  = 0;
    stInputChnPort.u32PortId = 0;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

    stBufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts              = 0;
    stBufConf.stFrameCfg.u16Width       = pFrame->width;
    stBufConf.stFrameCfg.u16Height      = pFrame->height;
    stBufConf.stFrameCfg.eFormat        = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stInputChnPort,&stBufConf,&stBufInfo,&hHandle, -1))
    {
        stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufInfo.stFrameData.eFieldType    = E_MI_SYS_FIELDTYPE_NONE;
        stBufInfo.stFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stBufInfo.bEndOfStream = FALSE;

        //printf("frame width : %d, height : %d\n", s32DispWidth, s32DispHeight);
        //向DIVP中填数据时必须按照stride大小填充
        if (!bState) {
            for (int index = 0; index < stBufInfo.stFrameData.u16Height; index ++)
            {
                memcpy((char*)stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                       pFrame->data[0] + index * stBufInfo.stFrameData.u16Width,
                       stBufInfo.stFrameData.u16Width);
            }

            for (int index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++)
            {
                memcpy((char*)stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                       pFrame->data[1] + index * stBufInfo.stFrameData.u16Width,
                       stBufInfo.stFrameData.u16Width);    
            }
        } else {
            SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)pFrame->opaque;
            MI_S32 s32Len = pFrame->width * pFrame->height;

			// bframe buf is meta data, inject function isn't supported, so using memory copy
			if (stVdecBuf->bType)
			{
				mi_vdec_DispFrame_t *pstVdecInfo = (mi_vdec_DispFrame_t *)stVdecBuf->stVdecBufInfo.stMetaData.pVirAddr;

				#if 1
				MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[0],
								pstVdecInfo->stFrmInfo.phyLumaAddr ,
								s32Len);
				MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[1],
								pstVdecInfo->stFrmInfo.phyChromaAddr,
								s32Len / 2);
				#else
				void *vdec_vir_addr;
				MI_SYS_Mmap(pstVdecInfo->stFrmInfo.phyLumaAddr, ALIGN_UP(s32Len + s32Len / 2, 4096), &vdec_vir_addr, FALSE);
				memcpy(stBufInfo.stFrameData.pVirAddr[0], vdec_vir_addr, s32Len);
				memcpy(stBufInfo.stFrameData.pVirAddr[1], vdec_vir_addr + s32Len, s32Len / 2);
				MI_SYS_Munmap(vdec_vir_addr, ALIGN_UP(s32Len + s32Len / 2, 4096));
				#endif

				sstar_buffer_putback(pFrame);
			}
			else
			{
				#if 0
				if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(stVdecBuf->stVdecHandle, &stInputChnPort))
					av_log(NULL, AV_LOG_ERROR, "MI_SYS_ChnPortInjectBuf failed!\n");
				#else
				MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[0],
								stVdecBuf->stVdecBufInfo.stFrameData.phyAddr[0],
								s32Len);
				MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[1],
								stVdecBuf->stVdecBufInfo.stFrameData.phyAddr[1],
								s32Len / 2);
				sstar_buffer_putback(pFrame);
				#endif
			}
        }

        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
    }

    return 0;
}

// MI play audio
static int sstar_audio_play(MI_U8 *pu8AudioData, MI_U32 u32DataLen)
{
    MI_S32 data_idx = 0, data_len = u32DataLen;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_S32 s32RetSendStatus = 0;
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

    //read data and send to AO module
    do {
        if (data_len <= AUDIO_MAX_DATA_SIZE)
            stAoSendFrame.u32Len = data_len;
        else
            stAoSendFrame.u32Len = AUDIO_MAX_DATA_SIZE;

        stAoSendFrame.apVirAddr[0] = &pu8AudioData[data_idx];
        stAoSendFrame.apVirAddr[1] = NULL;

        data_idx += AUDIO_MAX_DATA_SIZE;
        data_len -= AUDIO_MAX_DATA_SIZE;

        //printf("PlayAudio\n");
        do{
            s32RetSendStatus = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, 128);
        }while(s32RetSendStatus == MI_AO_ERR_NOBUF);

        if(s32RetSendStatus != MI_SUCCESS)
        {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
        }
    }while(data_len > 0);

    return 0;
}

// pause audio
static int sstar_audio_pause(void)
{
    MI_AO_PauseChn(0, 0);
    return 0;
}

// resume audio
static int sstar_audio_resume(void)
{
    MI_AO_ResumeChn(0, 0);
    return 0;
}

static int sstar_audio_init(void)
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.u32ChnCnt = 1;

    if(stSetAttr.u32ChnCnt == 2)
    {
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    }
    else if(stSetAttr.u32ChnCnt == 1)
    {
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    }

    stSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;

    /* set ao public attr*/
    MI_AO_SetPubAttr(AoDevId, &stSetAttr);

    /* get ao device*/
    MI_AO_GetPubAttr(AoDevId, &stGetAttr);

    /* enable ao device */
    MI_AO_Enable(AoDevId);

    /* enable ao channel of device*/
    MI_AO_EnableChn(AoDevId, AoChn);

    /* if test AO Volume */
    s32SetVolumeDb = -10;
    MI_AO_SetVolume(AoDevId, s32SetVolumeDb);
    MI_AO_SetMute(AoDevId, false);
    /* get AO volume */
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);

    return 0;
}

void sstar_audio_deinit(void)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

    /* disable ao channel of */
    MI_AO_DisableChn(AoDevId, AoChn);

    /* disable ao device */
    MI_AO_Disable(AoDevId);
}


static void player_control_callback(player_stat_t *is, player_control_t *func)
{
    if (is)
    {
        is->playerController.fpVideoPutBufBack            = sstar_buffer_putback;
        is->playerController.fpDisplayVideo               = sstar_video_display;
        is->playerController.fpPlayAudio                  = sstar_audio_play;
        is->playerController.fpPauseAudio                 = sstar_audio_pause;
        is->playerController.fpResumeAudio                = sstar_audio_resume;
        is->playerController.fpGetCurrentPlayPosFromVideo = NULL;
        is->playerController.fpGetCurrentPlayPosFromAudio = NULL;

        if (func == NULL)
        {
            is->playerController.fpGetMediaInfo           = NULL;
            is->playerController.fpGetDuration            = NULL;
            is->playerController.fpGetCurrentPlayPos      = NULL;
            is->playerController.fpPlayComplete           = NULL;
            is->playerController.fpPlayError              = NULL;
        }
        else
        {
            is->playerController.fpGetMediaInfo           = func->fpGetMediaInfo;
            is->playerController.fpGetDuration            = func->fpGetDuration;
            is->playerController.fpGetCurrentPlayPos      = func->fpGetCurrentPlayPos;
            is->playerController.fpPlayComplete           = func->fpPlayComplete;
            is->playerController.fpPlayError              = func->fpPlayError;
        }
    }
}


int g_loop_flag = 0;
player_stat_t *g_is = NULL;

int tp_player_open(char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *parg)
{
    int ret = -1;
    player_control_t *func_t = (player_control_t *)parg;
    
    //printf("\033[31;2mget in tp_player_open!\033[0m\n");

    if (g_is == NULL)
    {
        if ((x + width) > MAINWND_W || (y + height) > MAINWND_H)
        {
            printf("parameter is invalid!\n");
            return -1;
        }

        if (width % 16)
        {
            printf("width is not 16 alignment!\n");
            return -1;
        }
        
        g_is = player_init(fp);
        if (g_is == NULL)
        {
            printf("player init failed\n");
            return -1;
        }

        player_control_callback(g_is, func_t);
        
        ret = open_demux(g_is);
        if (ret < 0)
        {
            player_deinit(g_is);
            g_is = NULL;
            return -1;
        }

        g_is->out_width = width;
        g_is->out_width = height;
        sstar_audio_init();
        sstar_video_init(x, y, width, height);

        ret = open_video(g_is);
        ret = open_audio(g_is);

        g_loop_flag = 1;
    }
    //printf("\033[31;2mgtp_player_open exit!\033[0m\n");
    
    return ret;
}

int tp_player_close(void)
{
    int ret = -1;

    //printf("\033[31;2mget in tp_player_close!\n\033[0m");

    if (g_is)
    {
        g_loop_flag = 0;
        player_deinit(g_is);
        sstar_video_deinit();
        sstar_audio_deinit();
        g_is = NULL;
        ret = 0;
    }
        
    return ret;
}

int tp_player_loop(player_stat_t *parg)
{
    int ret = -1;
    
    if (g_loop_flag && !g_is->seek_req)
    {
        stream_seek(g_is, g_is->p_fmt_ctx->start_time, 0, 0);
        if (parg != NULL)
        {
            parg->audio_complete = 0;
            parg->video_complete = 0;
        }
        ret = 0;
    }
    
    return ret;
}


#endif
