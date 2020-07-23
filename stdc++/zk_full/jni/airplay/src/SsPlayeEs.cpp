#include "airplayer.h"

#ifdef __cplusplus
extern "C" {
#endif
#define VDEC_CHN_ID     0
int    gDbgLevel;
#define NANOX_ICAST(_fmt,_args...)   if(gDbgLevel & 0x40){printf("\033[40;35m###%s line %d ",__func__, __LINE__);printf(_fmt, ##_args);printf("\033[0m");}
#define ALIGN_BACK(x, align)        (((x) / (align)) * (align))
#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
    }

static int gWidth = 0;
static int gHeight = 0;

int Ss_Player_SetlayerSize(int w, int h)
{
	gWidth = w;
	gHeight = h;

	return 0;
}

void mvdec_dev_init()
{
    MI_VDEC_InitParam_t stVdecInitParam;
    memset(&stVdecInitParam, 0, sizeof(MI_VDEC_InitParam_t));
    stVdecInitParam.bDisableLowLatency = false;
    STCHECKRESULT(MI_VDEC_InitDev(&stVdecInitParam));
}

int ssd20x_scaler_init(int pos_x, int pos_y, int src_width, int src_height, int *dst_width, int *dst_height)
{
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);

	float f = (float)src_width/(float)src_height;
	if(f > 1.7f)//16:9
	{
		*dst_width  = (gWidth  > src_width ) ? src_width  : gWidth;
		*dst_height = (gHeight > src_height) ? src_height : gHeight;
	    stInputPortAttr.stDispWin.u16X      = pos_x;
	    stInputPortAttr.stDispWin.u16Y      = pos_y;
	    stInputPortAttr.stDispWin.u16Width  = gWidth;
	    stInputPortAttr.stDispWin.u16Height = gHeight;
	    stInputPortAttr.u16SrcWidth         = ALIGN_BACK(*dst_width , 32);
	    stInputPortAttr.u16SrcHeight        = ALIGN_BACK(*dst_height, 32);
	}
	else  //816*1088 or /1440x1080
	{
		int x = 0;
		int w = 0;
		w = (gHeight * src_width) / src_height;
		w = ALIGN_BACK(w, 8) + 8;
		x = (gWidth - w) / 2;
		*dst_width  = (w > src_width ) ? src_width : w;
		*dst_height = (gHeight > src_height) ? src_height : gHeight;
	    stInputPortAttr.stDispWin.u16X      = x;
	    stInputPortAttr.stDispWin.u16Y      = pos_y;
	    stInputPortAttr.stDispWin.u16Width  = w;//scaler to height full
	    stInputPortAttr.stDispWin.u16Height = gHeight;//1080
	    stInputPortAttr.u16SrcWidth         = ALIGN_BACK(*dst_width , 32);
	    stInputPortAttr.u16SrcHeight        = ALIGN_BACK(*dst_height, 32);
	}
    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
    printf("ssd20x_scaler output w/h = [%d %d]\n", *dst_width, *dst_height);
    return MI_SUCCESS;
}

int Ss_Player_Init(int input_w,int input_h,int flag)
{
    //init vdec
    MI_VDEC_ChnAttr_t stVdecChnAttr;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
    MI_VDEC_CHN stVdecChn = VDEC_CHN_ID;
    MI_S32 s32Width, s32Height;

    if(flag) mvdec_dev_init();
    NANOX_ICAST("resolution[%dx%d]\n",input_w,input_h);

    ssd20x_scaler_init(0,0,input_w,input_h,&s32Width,&s32Height);

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.eCodecType   = E_MI_VDEC_CODEC_TYPE_H264;
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 5;
    stVdecChnAttr.eVideoMode   = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize   = 1 * 1920 * 1080;
    stVdecChnAttr.u32PicWidth  = input_w;
    stVdecChnAttr.u32PicHeight = input_h;
    stVdecChnAttr.eDpbBufMode  = E_MI_VDEC_DPB_MODE_NORMAL;
    stVdecChnAttr.u32Priority  = 0;

    STCHECKRESULT(MI_VDEC_CreateChn(stVdecChn, &stVdecChnAttr));
    STCHECKRESULT(MI_VDEC_StartChn(stVdecChn));
    memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
    stOutputPortAttr.u16Width  = ALIGN_BACK(s32Width ,32);
    stOutputPortAttr.u16Height = ALIGN_BACK(s32Height,32);
    STCHECKRESULT(MI_VDEC_SetOutputPortAttr(0, &stOutputPortAttr));
	MI_SYS_ChnPort_t  stChnPort;
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId  = 0;
    stChnPort.u32ChnId  = stVdecChn;
    stChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));


	//bind vdec 2 disp
	MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
    MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 30, 30);

	NANOX_ICAST("leave\n");
	return 0;
}

int Ss_Player_DeInit(int flag)
{
	NANOX_ICAST("enter\n");
	//Unbind vdec 2 disp
	MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    stSrcChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;

    stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDstChnPort.u32DevId = 0;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
	STCHECKRESULT(MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort));
	STCHECKRESULT(MI_VDEC_StopChn(0));
    STCHECKRESULT(MI_VDEC_DestroyChn(0));
	if(flag) MI_VDEC_DeInitDev();
	NANOX_ICAST("leave\n");
	return 0;
}

#ifdef __cplusplus
}
#endif
