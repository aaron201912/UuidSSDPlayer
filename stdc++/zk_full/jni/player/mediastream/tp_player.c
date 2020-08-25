#ifdef SUPPORT_PLAYER_PROCESS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define CLT_IPC     "/appconfigs/client_input"
#define SVC_IPC     "/appconfigs/server_input"

typedef enum
{
  IPC_COMMAND_OPEN,
  IPC_COMMAND_CLOSE,
  IPC_COMMAND_PAUSE,
  IPC_COMMAND_RESUME,
  IPC_COMMAND_SEEK,
  IPC_COMMAND_SEEK2TIME,
  IPC_COMMAND_GET_POSITION,
  IPC_COMMAND_GET_DURATION,
  IPC_COMMAND_MAX,
  IPC_COMMAND_ACK,
  IPC_COMMAND_SET_VOLUMN,
  IPC_COMMAND_SET_MUTE,
  IPC_COMMAND_ERROR,
  IPC_COMMAND_COMPLETE,
  IPC_COMMAND_CREATE,
  IPC_COMMAND_DESTORY,
  IPC_COMMAND_EXIT
} IPC_COMMAND_TYPE;

typedef struct{
    int x;
    int y;
    int width;
    int height;
    double misc;
    int aodev, volumn;
    int status;
    int rotate;
    bool mute;
    bool audio_only, video_only;
    int  play_mode;
    char filePath[512];
}stPlayerData;

typedef struct {
    unsigned int EventType;
    stPlayerData stPlData;
} IPCEvent;

class IPCOutput {
public:
    IPCOutput(const std::string& file):m_fd(-1), m_file(file) {
    }

    virtual ~IPCOutput() {
        Term();
    }

    bool Init() {
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_WRONLY | O_NONBLOCK);
            printf("IPCOutput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

    int Send(const IPCEvent& evt) {
        if (m_fd >= 0) {
            ret = write(m_fd, &evt, sizeof(IPCEvent));
            //printf("write %d byte to %s\n", ret, m_file.c_str());
        } else {
            ret = -1;
            //printf("%s can't be writed!\n", m_file.c_str());
        }
        return ret;
    }

    void DeInit() {
        m_fd = -1;
        printf("%s deinit!\n", m_file.c_str());
    }

private:
    int m_fd, ret;
    std::string m_file;
};

class IPCNameFifo {
public:
    IPCNameFifo(const char* file): m_file(file) {
        unlink(m_file.c_str());
        m_valid = !mkfifo(m_file.c_str(), 0777);
    }

    ~IPCNameFifo() {
    unlink(m_file.c_str());
}

inline const std::string& Path() { return m_file; }
inline bool IsValid() { return m_valid; }

private:
    bool m_valid;
    std::string m_file;
};

class IPCInput {
public:
    IPCInput(const std::string& file):m_fd(-1),m_file(file),m_fifo(file.c_str()){}

    virtual ~IPCInput() {
        Term();
    }

    bool Init() {
        if (!m_fifo.IsValid()){
            printf("%s non-existent!!!! \n",m_fifo.Path().c_str());
            return false;
        }
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_RDWR | O_NONBLOCK);
            printf("IPCInput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    int Read(IPCEvent& evt) {
        if (m_fd >= 0) {
            return read(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("read %s failed!\n", m_file.c_str());
        return 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

private:
    int m_fd, ret;
    std::string m_file;
    IPCNameFifo m_fifo;
};

IPCEvent i_recvevt;
IPCEvent o_sendevt;
IPCInput  tp_server(SVC_IPC);
IPCOutput tp_client(CLT_IPC);

static int g_status = 0;

#endif
#if defined(SUPPORT_CLOUD_PLAY_MODULE) || defined(SUPPORT_PLAYER_MODULE)
#include "tp_player.h"
#include "player.h"
#include "frame.h"
#include "packet.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "blitutil.h"
#include "mi_vdec_extra.h"
#include "mi_gfx.h"

#define AUDIO_MAX_DATA_SIZE     25000
#define AUDIO_SAMPLE_PER_FRAME  1024
#define AUDIO_DEV               0

int g_loop_flag = 0;
player_stat_t *g_is = NULL;

#ifndef SUPPORT_PLAYER_PROCESS
static int sstar_video_init(void)
{
    MI_DISP_RotateConfig_t stRotateConfig;
    if (g_is->decode_type == SOFT_DECODING)
    {
        MI_DIVP_ChnAttr_t stDivpChnAttr;
        MI_DIVP_OutputPortAttr_t stOutputPortAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_SYS_ChnPort_t stDispChnPort;
        MI_SYS_ChnPort_t stDivpChnPort;

        MI_GFX_Open();

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(g_is->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(g_is->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = g_is->pos_x;
        stInputPortAttr.stDispWin.u16Y      = g_is->pos_y;
        stInputPortAttr.stDispWin.u16Width  = g_is->out_width;
        stInputPortAttr.stDispWin.u16Height = g_is->out_height;

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

        MI_DIVP_CreateChn(0, &stDivpChnAttr);
        MI_DIVP_SetChnAttr(0, &stDivpChnAttr);

        memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
        stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
        stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stOutputPortAttr.u32Width           = ALIGN_BACK(g_is->src_width , 32);
        stOutputPortAttr.u32Height          = ALIGN_BACK(g_is->src_height, 32);
        MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);
        MI_DIVP_StartChn(0);

        MI_DISP_DisableInputPort(0, 0);
        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = 0;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = 0;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
        MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);
    
        stRotateConfig.eRotateMode = E_MI_DISP_ROTATE_NONE;
    }
    else
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(g_is->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(g_is->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = g_is->pos_x;
        stInputPortAttr.stDispWin.u16Y      = g_is->pos_y;
        stInputPortAttr.stDispWin.u16Width  = g_is->out_width;
        stInputPortAttr.stDispWin.u16Height = g_is->out_height;

        MI_DISP_DisableInputPort(0, 0);
        MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
        MI_DISP_EnableInputPort(0, 0);
        MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

        stRotateConfig.eRotateMode = (MI_DISP_RotateMode_e)g_is->display_mode;
    }

    MI_DISP_SetVideoLayerRotateMode(0, &stRotateConfig);

    return 0;
}

static int sstar_video_deinit(void)
{
    if (g_is->decode_type == SOFT_DECODING)
    {
        MI_SYS_ChnPort_t stDivpChnPort;
        MI_SYS_ChnPort_t stDispChnPort;

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = 0;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = 0;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

        MI_DIVP_StopChn(0);
        MI_DIVP_DestroyChn(0);
        MI_GFX_Close();
    }

    MI_DISP_DisableInputPort(0, 0);

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

static int SetVideoRotate(MI_PHY yAddr, MI_PHY uvAddr)
{
    Surface_t srcY, dstY;
    Surface_t srcUV, dstUV;
    RECT_t r;
    srcY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    srcY.h              = g_is->p_vcodec_ctx->height;
    srcY.phy_addr       = g_is->phy_addr;
    srcY.pitch          = g_is->p_vcodec_ctx->width;
    srcY.w              = g_is->p_vcodec_ctx->width;
    srcY.BytesPerPixel  = 1;

    dstY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    dstY.h              = srcY.w;
    dstY.phy_addr       = yAddr;
    dstY.pitch          = ALIGN_UP(srcY.h, 16);
    dstY.w              = srcY.h;
    dstY.BytesPerPixel  = 1;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcY.h;
    r.right  = srcY.w;

    if (g_is->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcY, &dstY, &r);
    }
    else if (g_is->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcY, &dstY, &r);
    }

    srcUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    srcUV.h             = g_is->p_vcodec_ctx->height / 2;
    srcUV.phy_addr      = g_is->phy_addr + g_is->p_vcodec_ctx->width * g_is->p_vcodec_ctx->height;
    srcUV.pitch         = g_is->p_vcodec_ctx->width;
    srcUV.w             = g_is->p_vcodec_ctx->width / 2;
    srcUV.BytesPerPixel = 2;

    dstUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    dstUV.h             = srcUV.w;
    dstUV.phy_addr      = uvAddr;
    dstUV.pitch         = ALIGN_UP(srcY.h, 16);
    dstUV.w             = srcUV.h;
    dstUV.BytesPerPixel = 2;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcUV.h;
    r.right  = srcUV.w;

    if (g_is->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcUV, &dstUV, &r);
    } else if (g_is->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcUV, &dstUV, &r);
    }

    return MI_SUCCESS;
}


// MI display video
static int sstar_video_display(void *pData, bool bState)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t stInputChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    AVFrame *pFrame = (AVFrame *)pData;

    if (g_is->decode_type == SOFT_DECODING)
    {
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId    = E_MI_MODULE_ID_DIVP;
        stInputChnPort.u32ChnId  = 0;
        stInputChnPort.u32DevId  = 0;
        stInputChnPort.u32PortId = 0;

        memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
        memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

        stBufConf.eBufType       = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.u64TargetPts   = 0;
        stBufConf.u32Flags       = MI_SYS_MAP_VA;
        stBufConf.stFrameCfg.eFormat        = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConf.stFrameCfg.stFrameBufExtraConf.u16BufHAlignment = 16;

        if (g_is->display_mode == E_MI_DISP_ROTATE_NONE) {
            stBufConf.stFrameCfg.u16Width  = pFrame->width;
            stBufConf.stFrameCfg.u16Height = pFrame->height;
        } else {
            stBufConf.stFrameCfg.u16Width  = pFrame->height;
            stBufConf.stFrameCfg.u16Height = pFrame->width;
        }

        //MI_SYS_SetChnMMAConf(E_MI_MODULE_ID_DIVP, 0, 0, (MI_U8 *)"MMU_MMA");
        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stInputChnPort,&stBufConf,&stBufInfo,&hHandle, -1))
        {
            if (g_is->p_frm_yuv->width * g_is->p_frm_yuv->height < 1024 * 600) {
                MI_SYS_FlushInvCache(g_is->vir_addr, g_is->buf_size);
            }

            if (g_is->display_mode == E_MI_DISP_ROTATE_NONE)
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType    = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream              = FALSE;

                int length = g_is->p_frm_yuv->width * g_is->p_frm_yuv->height;
                for (int index = 0; index < stBufInfo.stFrameData.u16Height; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                    g_is->phy_addr + index * g_is->p_frm_yuv->width, g_is->p_frm_yuv->width);
                }
                for (int index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                    g_is->phy_addr + length + index * g_is->p_frm_yuv->width, g_is->p_frm_yuv->width);
                }
            }
            else
            {
                SetVideoRotate(stBufInfo.stFrameData.phyAddr[0], stBufInfo.stFrameData.phyAddr[1]);
            }

            MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
        }
        sstar_buffer_putback(pData);
    }
    else
    {
        MI_SYS_ChnPort_t  stInputChnPort;
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId                    = E_MI_MODULE_ID_DISP;
        stInputChnPort.u32ChnId                  = 0;
        stInputChnPort.u32DevId                  = 0;
        stInputChnPort.u32PortId                 = 0;

        SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)pFrame->opaque;

        if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(stVdecBuf->stVdecHandle, &stInputChnPort)) {
            printf("MI_SYS_ChnPortInjectBuf failed!\n");
        }
    }

    return MI_SUCCESS;
}

#if 0
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
                memcpy(stBufInfo.stFrameData.pVirAddr[0] + index * stBufInfo.stFrameData.u32Stride[0], 
                       pFrame->data[0] + index * stBufInfo.stFrameData.u16Width,
                       stBufInfo.stFrameData.u16Width);
            }

            for (int index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++)
            {
                memcpy(stBufInfo.stFrameData.pVirAddr[1] + index * stBufInfo.stFrameData.u32Stride[1], 
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
#endif

// MI play audio
static int sstar_audio_play(MI_U8 *pu8AudioData, MI_U32 u32DataLen, MI_S32 *s32BusyNum)
{
    MI_S32 data_idx = 0, data_len = u32DataLen;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_S32 s32RetSendStatus = 0;
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_AO_ChnState_t stState;

    MI_AO_QueryChnStat(AoDevId, AoChn, &stState);
    *s32BusyNum = stState.u32ChnBusyNum + 1024;

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
        is->playerController.fpSetVideoDisplay            = sstar_video_init;
        is->playerController.fpResetVideoDisplay          = sstar_video_deinit;

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
#else
static bool enable_listen = false, g_exit = false;
static pthread_t idle_tid;
static void * tp_idle_thread(void *arg)
{
    player_control_t *func_t = (player_control_t *)arg;

    printf("tp_idle_thread start\n");

    while (!g_exit)
    {
        if (enable_listen) {
            if (tp_server.Read(i_recvevt) > 0) {
                if (i_recvevt.EventType == IPC_COMMAND_COMPLETE) {
                    printf("receive message of playing completely!\n");
                    func_t->fpPlayComplete();
                } else if (i_recvevt.EventType == IPC_COMMAND_ERROR) {
                    printf("receive message of error in playing!\n");
                    func_t->fpPlayError(i_recvevt.stPlData.status);
                }
            }
        }
        usleep(100 * 1000);
    }

    printf("### tp_idle_thread exit ###\n");
    return NULL;
}
#endif

#define USE_POPEN       1
FILE *tp_fd = NULL;

int tp_player_open(char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *parg)
{
#ifdef SUPPORT_PLAYER_PROCESS
    printf("tp_player_open start!\n");

    struct timeval time_start, time_wait;

    player_control_t *func_t = (player_control_t *)parg;

    if(!tp_server.Init()) {
        printf("[%s %d]create i_server fail!\n", __FILE__, __LINE__);
        return -1;
    }

    #if USE_POPEN
    tp_fd = popen("./MyPlayer &", "w");
    if (tp_fd == NULL) {
        printf("my_player is not exit!\n");
        return -1;
    }
    printf("popen myplayer progress done!\n");

    memset(&i_recvevt, 0, sizeof(IPCEvent));
    gettimeofday(&time_start, NULL);
    while (tp_server.Read(i_recvevt) <= 0
           || (i_recvevt.EventType != IPC_COMMAND_CREATE)) {
        usleep(10 * 1000);
        gettimeofday(&time_wait, NULL);
        if (time_wait.tv_sec - time_start.tv_sec > 2) {
            printf("myplayer progress destory failed!\n");
            break;
        }
    }
    if (i_recvevt.EventType == IPC_COMMAND_CREATE) {
        printf("myplayer progress create success!\n");
    } else {
        func_t->fpPlayError(-1);
        return -1;
    }
    #endif

    if(!tp_client.Init()) {
        printf("[%s %d]my_player is not start!\n", __FILE__, __LINE__);
        return -1;
    }

    g_exit = false;
    int ret = pthread_create(&idle_tid, NULL, tp_idle_thread, (void *)parg);
    if (ret != 0) {
        printf("[%s %d]create tp_idle_thread failed!\n", __FILE__, __LINE__);
        return -1;
    }

    memset(&o_sendevt, 0, sizeof(IPCEvent));
    o_sendevt.EventType = IPC_COMMAND_OPEN;
    strcpy(o_sendevt.stPlData.filePath, fp);
    // 旋转开关
    #if ENABLE_ROTATE
    o_sendevt.stPlData.rotate = E_MI_DISP_ROTATE_270;
    #else
    o_sendevt.stPlData.rotate = E_MI_DISP_ROTATE_NONE;
    #endif
    o_sendevt.stPlData.x = x;
    o_sendevt.stPlData.y = y;
    o_sendevt.stPlData.width  = width;
    o_sendevt.stPlData.height = height;
    o_sendevt.stPlData.aodev = AUDIO_DEV;
    o_sendevt.stPlData.audio_only = false;
    o_sendevt.stPlData.video_only = false;
    o_sendevt.stPlData.play_mode  = 0;    // 0: 单次播放,1: 循环播放(seek to start)
    tp_client.Send(o_sendevt);
    printf("tp_player try to open file: %s\n", fp);

    memset(&i_recvevt, 0, sizeof(IPCEvent));
    while (tp_server.Read(i_recvevt) <= 0
           || ((i_recvevt.EventType != IPC_COMMAND_ACK)
           && (i_recvevt.EventType != IPC_COMMAND_ERROR))) {
        usleep(10 * 1000);
        gettimeofday(&time_wait, NULL);
        if (time_wait.tv_sec - time_start.tv_sec > 10) {
            memset(&o_sendevt, 0, sizeof(IPCEvent));
            #if USE_POPEN
            o_sendevt.EventType = IPC_COMMAND_EXIT;
            #else
            o_sendevt.EventType = IPC_COMMAND_CLOSE;
            #endif
            tp_client.Send(o_sendevt);
            break;
        }
    }
    if (i_recvevt.EventType == IPC_COMMAND_ACK) {
        printf("receive ack from my_player!\n");
    } else if (i_recvevt.EventType == IPC_COMMAND_ERROR) {
        g_status = i_recvevt.stPlData.status;
        printf("my_player occur error [%d]!\n", g_status);
        func_t->fpPlayError(g_status);
    }

    enable_listen = true;
    return 0;
#else
    int ret = -1;
    player_control_t *func_t = (player_control_t *)parg;

    if (g_is != NULL) {
        printf("\033[31;2mtp_player_open failed!\033[0m\n");
        return -1;
    }

    if ((x + width) > MAINWND_W || (y + height) > MAINWND_H || !width || !height) {
        printf("parameter is invalid!\n");
        return -1;
    }

    g_is = player_init(fp);
    if (g_is == NULL) {
        printf("player init failed\n");
        return -1;
    }

    g_is->in_width  = width;
    g_is->in_height = height;
    printf("tp_player_open w/h = [%d %d]\n", g_is->in_width, g_is->in_height);

    sstar_audio_init();
    //sstar_video_init(x, y, width, height);

    player_control_callback(g_is, func_t);

    ret = open_demux(g_is);
    if (ret < 0) {
        goto fail;
    }

    ret = open_video(g_is);
    if (ret < 0) {
        goto fail;
    }

    ret = open_audio(g_is);
    if (ret < 0) {
        goto fail;
    }

    g_loop_flag = 1;

    return 0;
fail:
    tp_player_close();
    return ret;
#endif
}

int tp_player_close(void)
{
#ifdef SUPPORT_PLAYER_PROCESS
    struct timeval time_start, time_wait;

    if(!tp_client.Init()) {
        printf("my_player is not start!\n");
        tp_server.Term();
        return -1;
    }
    memset(&o_sendevt, 0, sizeof(IPCEvent));
    #if USE_POPEN
    o_sendevt.EventType = IPC_COMMAND_EXIT;
    #else
    o_sendevt.EventType = IPC_COMMAND_CLOSE;
    #endif
    tp_client.Send(o_sendevt);

    #if USE_POPEN
    memset(&i_recvevt, 0, sizeof(IPCEvent));
    gettimeofday(&time_start, NULL);
    while (tp_server.Read(i_recvevt) <= 0
           || (i_recvevt.EventType != IPC_COMMAND_DESTORY)) {
        usleep(10 * 1000);
        gettimeofday(&time_wait, NULL);
        if (time_wait.tv_sec - time_start.tv_sec > 2) {
            printf("myplayer progress destory failed!\n");
            break;
        }
    }
    if (i_recvevt.EventType == IPC_COMMAND_DESTORY) {
        printf("myplayer progress destory done!\n");
    }
    pclose(tp_fd);
    tp_fd = NULL;
    #endif

    printf("tp_player_close done!\n");

    g_exit = true;
    pthread_join(idle_tid, NULL);
    enable_listen = false;

    tp_server.Term();
    tp_client.Term();

    return 0;
#else
    int ret = -1;

    if (!g_is) {
        printf("\033[31;2mtp_player_close failed!\n\033[0m");
        return -1;
    }

    g_loop_flag = 0;
    player_deinit(g_is);
    //sstar_video_deinit();
    sstar_audio_deinit();
    g_is = NULL;
    ret = 0;

    return ret;
#endif
}

int tp_player_status(int *status)
{
#ifdef SUPPORT_PLAYER_PROCESS
    memset(&i_recvevt, 0, sizeof(IPCEvent));
    if (tp_server.Read(i_recvevt) > 0) {
        if (i_recvevt.EventType == IPC_COMMAND_ERROR) {
            printf("my_player occur error in playing!\n");
        } else if (i_recvevt.EventType == IPC_COMMAND_COMPLETE){
            printf("my_player has played completely!\n");
        }
        *status = i_recvevt.stPlData.status;
    } else {
        *status = 0;
    }

    if (g_status < 0) {
        *status = g_status;
    }
    return 0;
#else
    if (g_is != NULL)
    {
        //stream_seek(g_is, g_is->p_fmt_ctx->start_time, 0, 0);
        if (g_is->audio_complete && g_is->video_complete) {
            return 1;
        } else {
            return 0;
        }
    }
    return -1;
#endif
}
#endif
