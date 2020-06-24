#pragma once
#include "uart/ProtocolSender.h"
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index) 
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress) 
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX() 
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

#ifdef SUPPORT_PLAYER_MODULE
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "mi_sys.h"
#include "mi_divp.h"
#include "mi_disp.h"
#include "mi_vdec_extra.h"
//#include "mi_panel.h"
//#include "mi_gfx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "frame.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "player.h"
#include "panelconfig.h"
#include "hotplugdetect.h"

#include "imageplayer.h"

#define UI_MAX_WIDTH			800
#define UI_MAX_HEIGHT			480

#define ALIGN_DOWN(x, n)        (((x) / (n)) * (n))
#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define DIVP_CHN        		0
#define DISP_DEV        		0
#define DISP_LAYER      		0
#define DISP_INPUTPORT  		0
#define AUDIO_DEV       		0
#define AUDIO_CHN       		0
#define AUDIO_SAMPLE_PER_FRAME  1024
#define AUDIO_MAX_DATA_SIZE     25000
#define MIN_AO_VOLUME           -60
#define MAX_AO_VOLUME           30
#define MIN_ADJUST_AO_VOLUME    -10
#define MAX_ADJUST_AO_VOLUME    20

#define VOL_ADJUST_FACTOR		2
#define PROGRESS_UPDATE_TIME_INTERVAL	500000		// 0.5s

#define DISPLAY_PIC_DURATION	5000000				// 5s

typedef enum
{
    E_PLAY_FORWARD,
    E_PLAY_BACKWARD
}PlayDirection_e;

typedef enum
{
    E_PLAY_NORMAL_MODE,
    E_PLAY_FAST_MODE,
    E_PLAY_SLOW_MODE
}PlayMode_e;

typedef enum
{
    E_NORMAL_SPEED = 0,
    E_2X_SPEED,
    E_4X_SPEED,
    E_8X_SPEED,
    E_16X_SPEED,
    E_32X_SPEED
}PlaySpeedMode_e;

typedef enum
{
	FILE_REPEAT_MODE,
	LIST_REPEAT_MODE
}RepeatMode_e;

typedef enum
{
	NO_SKIP,
	SKIP_NEXT,
	SKIP_PREV
}SkipMode_e;

// playing page
static bool g_bShowPlayToolBar = FALSE;         // select file list page or playing page
static bool g_bPlaying = FALSE;					// 正在播放状态
static bool g_bPause = FALSE;					// 播放暂停状态
static bool g_bMute = FALSE;
static int g_s32VolValue = 20;
static bool g_ePlayDirection = E_PLAY_FORWARD;
static PlayMode_e g_ePlayMode = E_PLAY_NORMAL_MODE;
static PlaySpeedMode_e g_eSpeedMode = E_NORMAL_SPEED;
static unsigned int g_u32SpeedNumerator = 1;
static unsigned int g_u32SpeedDenomonator = 1;

// playViewer size
static int g_playViewWidth = PANEL_MAX_WIDTH;
static int g_playViewHeight = PANEL_MAX_HEIGHT;

// streamplayer & imagePlayer
static std::string g_fileName;
static player_stat_t *g_pstPlayStat = NULL;
static ImagePlayer_t *g_pstImagePlayer = NULL;

// play pos
static long long g_firstPlayPos = PLAY_INIT_POS;
static long long g_duration = 0;
static long long int g_lastpos = 0;

// play video/audio or picture
static int g_playStream = 0;
static pthread_t g_playFileThread = 0;
static bool g_bPlayFileThreadExit = false;
static bool g_bPlayCompleted = false;
static bool g_bPlayError = false;
static RepeatMode_e g_eRepeatMode = LIST_REPEAT_MODE;
static SkipMode_e g_eSkipMode = NO_SKIP;
static pthread_mutex_t g_playFileMutex;

extern void GetPrevFile(char *pCurFileFullName, char *pPrevFileFullName, int prevFilePathLen);
extern void GetNextFile(char *pCurFileFullName, char *pNextFileFullName, int nextFilePathLen);
extern int IsMediaStreamFile(char *pCurFileFullName);

void ShowToolbar(bool bShow)
{
	mWindow_playBarPtr->setVisible(bShow);
	mWindow_mediaInfoPtr->setVisible(bShow);

	if (g_playStream)
	{
		mTextview_volTitlePtr->setVisible(true);
		mTextview_volumePtr->setVisible(true);
		mSeekbar_progressPtr->setVisible(true);
		mButton_voicePtr->setVisible(true);
		mSeekbar_volumnPtr->setVisible(true);
		mTextview_slashPtr->setVisible(true);
		mTextview_durationPtr->setVisible(true);
		mTextview_curtimePtr->setVisible(true);
	}
	else
	{
		mTextview_volTitlePtr->setVisible(false);
		mTextview_volumePtr->setVisible(false);
		mSeekbar_progressPtr->setVisible(false);
		mButton_voicePtr->setVisible(false);
		mSeekbar_volumnPtr->setVisible(false);
		mTextview_slashPtr->setVisible(false);
		mTextview_durationPtr->setVisible(false);
		mTextview_curtimePtr->setVisible(false);
	}
}

class ToolbarHideThread : public Thread {
public:
	void setCycleCnt(int cnt, int sleepMs) { nCycleCnt = cnt; nSleepMs = sleepMs; }

protected:
	virtual bool threadLoop() {
		if (!nCycleCnt)
		{
			ShowToolbar(false);
			return false;
		}

		sleep(nSleepMs);
		nCycleCnt--;

		return true;
	}

private:
	int nCycleCnt;
	int nSleepMs;
};

static ToolbarHideThread g_hideToolbarThread;

// auto hide toolbar after displaying 5s
void AutoDisplayToolbar()
{
	if (!g_hideToolbarThread.isRunning())
	{
		printf("start hide toolbar thread\n");
		g_hideToolbarThread.setCycleCnt(100, 50);
		g_hideToolbarThread.run("hideToolbar");
	}
	else
	{
		printf("wait hideToolBarthread exit\n");
		g_hideToolbarThread.requestExitAndWait();
		g_hideToolbarThread.setCycleCnt(100, 50);
		g_hideToolbarThread.run("hideToolbar");
	}

	ShowToolbar(true);
}

void SetPlayingStatus(bool bPlaying)
{
	mButton_playPtr->setSelected(bPlaying);
}

void SetMuteStatus(bool bMute)
{
	mButton_voicePtr->setSelected(bMute);
}

int SetPlayerVolumn(int vol)
{
	char volInfo[8];

	memset(volInfo, 0, sizeof(volInfo));
	sprintf(volInfo, "%d%%", vol);
	mSeekbar_volumnPtr->setProgress(vol);
	mTextview_volumePtr->setText(volInfo);
	return 0;
}

int GetPlayerVolumn()
{
	int vol = mSeekbar_volumnPtr->getProgress();
	char volInfo[8];

	memset(volInfo, 0, sizeof(volInfo));
	sprintf(volInfo, "%d%%", vol);
	mTextview_volumePtr->setText(volInfo);
	return vol;
}

MI_S32 CreatePlayerDev()
{
    MI_SYS_ChnPort_t stDivpChnPort;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;

//    MI_U32 u32InputPort = DISP_INPUTPORT;
//    MI_DISP_DEV dispDev = DISP_DEV;
    MI_DISP_LAYER dispLayer = DISP_LAYER;
    MI_SYS_ChnPort_t stDispChnPort;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_RotateConfig_t stRotateConfig;

    system("echo 12 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio12/direction");
    //system("echo 1 > /sys/class/gpio/gpio12/value");

    // 1.初始化DISP模块
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
    stInputPortAttr.u16SrcWidth         = ALIGN_DOWN(g_playViewWidth , 32);
    stInputPortAttr.u16SrcHeight        = ALIGN_DOWN(g_playViewHeight, 32);
    stInputPortAttr.stDispWin.u16X      = 0;
    stInputPortAttr.stDispWin.u16Y      = 0;
    stInputPortAttr.stDispWin.u16Width  = g_playViewWidth;
    stInputPortAttr.stDispWin.u16Height = g_playViewHeight;

    printf("disp input: w=%d, h=%d\n", stInputPortAttr.u16SrcWidth, stInputPortAttr.u16SrcHeight);

    // 2.初始化DIVP模块
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

    MI_DIVP_CreateChn(DIVP_CHN, &stDivpChnAttr);
    MI_DIVP_StartChn(DIVP_CHN);
    
    memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
    stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stOutputPortAttr.u32Width           = ALIGN_DOWN(g_playViewWidth , 32);
    stOutputPortAttr.u32Height          = ALIGN_DOWN(g_playViewHeight, 32);

    // 3.配置旋转属性
    stRotateConfig.eRotateMode          = E_MI_DISP_ROTATE_NONE;
    MI_DISP_SetVideoLayerRotateMode(dispLayer, &stRotateConfig);

    if (stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_NONE || stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_180)
    {
        stInputPortAttr.u16SrcWidth     = ALIGN_DOWN(g_playViewWidth , 32);
        stInputPortAttr.u16SrcHeight    = ALIGN_DOWN(g_playViewHeight, 32);
        stOutputPortAttr.u32Width       = ALIGN_DOWN(g_playViewWidth , 32);
        stOutputPortAttr.u32Height      = ALIGN_DOWN(g_playViewHeight, 32);
    }
    else
    {
        stInputPortAttr.u16SrcWidth     = ALIGN_DOWN(g_playViewHeight, 32);
        stInputPortAttr.u16SrcHeight    = ALIGN_DOWN(g_playViewWidth , 32);
        stOutputPortAttr.u32Width       = ALIGN_DOWN(g_playViewHeight, 32);
        stOutputPortAttr.u32Height      = ALIGN_DOWN(g_playViewWidth , 32);
    }

    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
    MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);

    MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);

    //4.绑定DIVP与DISP
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = DISP_DEV;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = DISP_INPUTPORT;
    
    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = DIVP_CHN;
    stDivpChnPort.u32PortId             = 0;

    MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
    MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

    return 0;
}

void DestroyPlayerDev()
{
//    MI_DISP_LAYER dispLayer = DISP_LAYER;
//    MI_U32 u32InputPort = DISP_INPUTPORT;
    MI_SYS_ChnPort_t stDivpChnPort;
    MI_SYS_ChnPort_t stDispChnPort;
    
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = DISP_DEV;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = DISP_INPUTPORT;
    
    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = DIVP_CHN;
    stDivpChnPort.u32PortId             = 0;
    
    MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

    MI_DIVP_StopChn(0);
    MI_DIVP_DestroyChn(0);

    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
}


MI_S32 StartPlayVideo()
{
    MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);
	return 0;
}

void StopPlayVideo()
{
	MI_DISP_ClearInputPortBuffer(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_HideInputPort(DISP_LAYER, DISP_INPUTPORT);
}

MI_S32 StartPlayAudio()
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    system("echo 1 > /sys/class/gpio/gpio12/value");

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
    if (g_s32VolValue)
    	s32SetVolumeDb = g_s32VolValue * (MAX_ADJUST_AO_VOLUME-MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
    else
    	s32SetVolumeDb = MIN_AO_VOLUME;

    MI_AO_SetVolume(AoDevId, s32SetVolumeDb);
    MI_AO_SetMute(AoDevId, g_bMute);
    SetMuteStatus(g_bMute);

    /* get AO volume */
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);

    return 0;
}

void StopPlayAudio()
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    system("echo 0 > /sys/class/gpio/gpio12/value");

    /* disable ao channel of */
    MI_AO_DisableChn(AoDevId, AoChn);

    /* disable ao device */
    MI_AO_Disable(AoDevId);
}

static void ResetSpeedMode()
{
    g_ePlayDirection = E_PLAY_FORWARD;
    g_ePlayMode = E_PLAY_NORMAL_MODE;
    g_eSpeedMode = E_NORMAL_SPEED;
    g_u32SpeedNumerator = 1;
    g_u32SpeedDenomonator = 1;
}

// duration, format, width, height, I-frame/P-frame, etc.
MI_S32 GetStreamFileInfo()
{
    return 0;
}

MI_S32 GetStreamFileDuration(long long duration)
{
	char totalTime[32];
	long long durationSec = duration / AV_TIME_BASE;

	if (durationSec / 3600 > 99)
	{
		printf("file size is limited\n");
		return -1;
	}

	memset(totalTime, 0, sizeof(totalTime));
	sprintf(totalTime, "%02lld:%02lld:%02lld", durationSec/3600, (durationSec%3600)/60, durationSec%60);
	mTextview_durationPtr->setText(totalTime);
	g_duration = duration;

	return 0;
}

MI_S32 GetStreamFilePlayPos(long long currentPos, long long frame_duration)
{
    char curTime[32];
    long long curSec = 0;
    int trackPos = 0;

    g_lastpos = currentPos;

    if (currentPos > g_duration)
    {
        printf("curPos exceed duration, curPos:%lld, duration:%lld\n", currentPos, g_duration);
        currentPos = g_duration;
    }

    // update playtime static
    if (g_firstPlayPos < 0)
        curSec = 0;
    else
    {
        long long pos = currentPos % PROGRESS_UPDATE_TIME_INTERVAL;
        //printf("pos:%lld, frame_duration:%lld, curPos:%lld, firstPos:%lld\n", pos, frame_duration, currentPos, g_firstPlayPos);
        if (pos > frame_duration/2 && pos <= (PROGRESS_UPDATE_TIME_INTERVAL - frame_duration/2))
            return 0;
    }

    curSec = currentPos / AV_TIME_BASE;

    memset(curTime, 0, sizeof(curTime));
    sprintf(curTime, "%02lld:%02lld:%02lld", curSec/3600, (curSec%3600)/60, curSec%60);
    mTextview_curtimePtr->setText(curTime);

    // update progress bar
    trackPos  = (currentPos * mSeekbar_progressPtr->getMax()) / g_duration;
    mSeekbar_progressPtr->setProgress(trackPos);

    if (g_firstPlayPos < 0)
        g_firstPlayPos = currentPos;

    return 0;
}


MI_S32 VideoPutBufBack(void *pData)
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
MI_S32 DisplayVideo(void *pData, bool bState)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t stInputChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

	AVFrame *pFrame = (AVFrame *)pData;

    memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stInputChnPort.eModId    = E_MI_MODULE_ID_DIVP;
    stInputChnPort.u32ChnId  = 0;
    stInputChnPort.u32DevId  = 0;
    stInputChnPort.u32PortId = 0;

    memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
    memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts = 0;
    stBufConf.stFrameCfg.u16Width  = pFrame->width;
    stBufConf.stFrameCfg.u16Height = pFrame->height;
    stBufConf.stFrameCfg.eFormat   = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;

    if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stInputChnPort,&stBufConf,&stBufInfo,&hHandle, -1))
    {
        stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        stBufInfo.stFrameData.eFieldType = E_MI_SYS_FIELDTYPE_NONE;
        stBufInfo.stFrameData.eTileMode = E_MI_SYS_FRAME_TILE_MODE_NONE;
        stBufInfo.bEndOfStream = FALSE;

		if (!bState)
		{
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
        }
        else
        {
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

                VideoPutBufBack(pFrame);
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
                VideoPutBufBack(pFrame);
                #endif
            }
        }

        MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
    }

    return 0;
}

// MI play audio
MI_S32 PlayAudio(MI_U8 *pu8AudioData, MI_U32 u32DataLen)
{
    MI_S32 data_idx = 0, data_len = u32DataLen;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_S32 s32RetSendStatus = 0;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

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
MI_S32 PauseAudio()
{
    MI_AO_PauseChn(AUDIO_DEV, AUDIO_CHN);
    return 0;
}

// resume audio
MI_S32 ResumeAudio()
{
    MI_AO_ResumeChn(AUDIO_DEV, AUDIO_CHN);
    return 0;
}

// stay in playing page, clear play status
MI_S32 PlayStreamFileComplete()
{
	SetPlayingStatus(false);
	mTextview_speedPtr->setText("");
	g_bShowPlayToolBar = FALSE;

	pthread_mutex_lock(&g_playFileMutex);
	g_bPlayCompleted = true;
	pthread_mutex_unlock(&g_playFileMutex);
    return 0;
}

// stay in playing page , clear play status,
MI_S32 PlayStreamFileError(int error)
{
    if (error == -101)
        mTextview_msgPtr->setText("请检查网络连接！");
    else if (error == -2)
        mTextview_msgPtr->setText("不支持播放720P以上的视频！");
    else if (error == -3)
        mTextview_msgPtr->setText("解码速度不够，请降低视频帧率！");
    else if (error == -4)
        mTextview_msgPtr->setText("硬件解码器卡死！");
    else
        mTextview_msgPtr->setText("Other Error Occur!");
        
    mWindow_errMsgPtr->setVisible(true);

    pthread_mutex_lock(&g_playFileMutex);
	g_bPlayError = true;
	pthread_mutex_unlock(&g_playFileMutex);

    return 0;
}

int GetImageFileDuration(long long duration)
{
	char totalTime[32] = {0};
	long long durationSec = 0;

	g_duration = duration;
	durationSec = g_duration / AV_TIME_BASE;
	sprintf(totalTime, "%02lld:%02lld:%02lld", durationSec/3600, (durationSec%3600)/60, durationSec%60);
	mTextview_durationPtr->setText(totalTime);
	mTextview_curtimePtr->setText("00:00:00");
	mSeekbar_progressPtr->setProgress(0);

	return 0;
}

int GetImageFilePlayPos(long long currentPos)
{
	char curTime[32];
	long long curSec = 0;
	int trackPos = 0;

	printf("Enter GetImageFilePlayPos\n");
	printf("currentPos is %lld\n", currentPos);
	curSec = currentPos / AV_TIME_BASE;
	trackPos  = (currentPos * mSeekbar_progressPtr->getMax()) / g_duration;
	memset(curTime, 0, sizeof(curTime));
	sprintf(curTime, "%02lld:%02lld:%02lld", curSec/3600, (curSec%3600)/60, curSec%60);
	printf("image curTime: %s\n", curTime);
	mTextview_curtimePtr->setText(curTime);
	mSeekbar_progressPtr->setProgress(trackPos);
	printf("Leave GetImageFilePlayPos\n");

	return 0;
}

MI_S32 PlayImageFileComplete()
{
	printf("Enter PlayImageFileComplete\n");
	SetPlayingStatus(false);
	mTextView_picPtr->setVisible(false);
	mTextview_curtimePtr->setText("00:00:00");
	mSeekbar_progressPtr->setProgress(0);

	pthread_mutex_lock(&g_playFileMutex);
	g_bPlayCompleted = true;
	pthread_mutex_unlock(&g_playFileMutex);
	printf("Leave PlayImageFileComplete\n");

	return 0;
}

static void SetStreamPlayerControlCallBack(player_stat_t *is)
{
	is->playerController.fpGetMediaInfo = GetStreamFileInfo;
	is->playerController.fpGetDuration = GetStreamFileDuration;
	is->playerController.fpGetCurrentPlayPos = GetStreamFilePlayPos;
	is->playerController.fpGetCurrentPlayPosFromVideo = NULL;
	is->playerController.fpGetCurrentPlayPosFromAudio = NULL;
	is->playerController.fpDisplayVideo = DisplayVideo;
	is->playerController.fpVideoPutBufBack = VideoPutBufBack;
	is->playerController.fpPlayAudio = PlayAudio;
	is->playerController.fpPauseAudio = PauseAudio;
	is->playerController.fpResumeAudio = ResumeAudio;
	is->playerController.fpPlayComplete = PlayStreamFileComplete;
	is->playerController.fpPlayError = PlayStreamFileError;
}

static void AdjustVolumeByTouch(int startPos, int endPos)
{
	int progress = mSeekbar_volumnPtr->getProgress();
	char volInfo[16];

	memset(volInfo, 0, sizeof(volInfo));
	// move up, vol++; move down, vol--
	progress -= (endPos - startPos) / VOL_ADJUST_FACTOR;

	progress = (progress > mSeekbar_volumnPtr->getMax())? mSeekbar_volumnPtr->getMax() : progress;
	progress = (progress < 0)? 0 : progress;
	mSeekbar_volumnPtr->setProgress(progress);

	sprintf(volInfo, "%d%%", progress);
	mTextview_volumePtr->setText(volInfo);
	printf("set progress: %d\n", progress);
}

void DetectUsbHotplug(UsbParam_t *pstUsbParam)		// action 0, connect; action 1, disconnect
{
	if (!pstUsbParam->action)
	{
		g_bPlaying = FALSE;
		g_bPause = FALSE;

		if (g_hideToolbarThread.isRunning())
		{
			printf("stop hideToolBarthread\n");
			g_hideToolbarThread.requestExitAndWait();
		}

		g_bPlayFileThreadExit = true;
		if (g_playFileThread)
		{
			pthread_join(g_playFileThread, NULL);
			g_playFileThread = NULL;
		}

		SetPlayingStatus(false);
		ResetSpeedMode();
		mTextview_speedPtr->setText("");
		g_bShowPlayToolBar = FALSE;

		EASYUICONTEXT->goHome();
	}
}

static void StartPlayStreamFile(char *pFileName)
{
	// ffmpeg_player初始化 & ui初始化
	mWindow_errMsgPtr->setVisible(false);
	// init player
	ResetSpeedMode();
	StartPlayVideo();
	StartPlayAudio();

	g_pstPlayStat = player_init(pFileName);
	if (!g_pstPlayStat)
	{
		StopPlayAudio();
		StopPlayVideo();
		printf("Initilize player failed!\n");
		return;
	}
	g_pstPlayStat->out_width  = g_playViewWidth;
	g_pstPlayStat->out_height = g_playViewHeight;
	printf("video file name is : %s\n", g_pstPlayStat->filename);

	SetStreamPlayerControlCallBack(g_pstPlayStat);
	printf("open_demux\n");
	open_demux(g_pstPlayStat);
	printf("open_video\n");
	open_video(g_pstPlayStat);
	printf("open_audio\n");
	open_audio(g_pstPlayStat);
	SetPlayerVolumn(g_s32VolValue);
}

static void StopPlayStreamFile()
{
	printf("Start to StopPlayStreamFile\n");

	// ffmpeg_player反初始化 & ui反初始化
	g_bPlaying = false;
	g_bPause = false;
	player_deinit(g_pstPlayStat);
	StopPlayAudio();
	StopPlayVideo();
	ResetSpeedMode();

	SetPlayingStatus(false);
	mTextview_speedPtr->setText("");
	mTextview_curtimePtr->setText("00:00:00");
	mSeekbar_progressPtr->setProgress(0);

	// reset pts
	g_firstPlayPos = PLAY_INIT_POS;

	printf("End of StopPlayStreamFile\n");
}

static void TogglePlayStreamFile()
{
	toggle_pause(g_pstPlayStat);
}

static void StartDisplayImage(char *pFileName)
{
	ImagePlayerCtrl_t stPlayerCtrl;

	printf("Enter StartDisplayImage, fileName is %s\n", pFileName);
	stPlayerCtrl.fpGetDuration = GetImageFileDuration;
	stPlayerCtrl.fpGetCurrentPlayPos = GetImageFilePlayPos;
	stPlayerCtrl.fpDisplayComplete = PlayImageFileComplete;
	StartPlayAudio();

	g_pstImagePlayer = ImagePlayer_Init(&stPlayerCtrl);
	if (!g_pstImagePlayer)
	{
		StopPlayAudio();
		printf("ImagePlayer_Init exec failed\n");
		return;
	}

	int imgWidth = 0;
	int imgHeight = 0;
	int colorBits = 0;
	unsigned char *data = stbi_load(pFileName, &imgWidth, &imgHeight, &colorBits, 0);
	printf("img width=%d, height=%d, colorbits=%d\n", imgWidth, imgHeight, colorBits);
	stbi_image_free(data);

	LayoutPosition imgPosition = mTextView_picPtr->getPosition();
	imgPosition.mLeft = imgPosition.mLeft + (imgPosition.mWidth - imgWidth) / 2;
	imgPosition.mTop = imgPosition.mTop + (imgPosition.mHeight - imgHeight) / 2;
	imgPosition.mWidth = imgWidth;
	imgPosition.mHeight = imgHeight;
	mTextView_picPtr->setPosition(imgPosition);
	mTextView_picPtr->setBackgroundPic(pFileName);

	mTextView_picPtr->setVisible(true);
	printf("Leave StartDisplayImage\n");
}

static void StopDisplayImage()
{
	printf("Enter StopDisplayImage\n");
	ImagePlayer_Deinit(g_pstImagePlayer);
	StopPlayAudio();

	mTextView_picPtr->setVisible(false);
	mTextview_curtimePtr->setText("00:00:00");
	mSeekbar_progressPtr->setProgress(0);

	printf("Leave StopDisplayImage\n");
}

static void TogglePlayImageFile()
{
	ImagePlayer_TogglePause(g_pstImagePlayer);
}

static void StopPlayFile()
{
	if (g_playStream)
		StopPlayStreamFile();
	else
		StopDisplayImage();
}

static void StartPlayFile(char *pFileName)
{
	g_bPlayCompleted = false;
	g_playStream = IsMediaStreamFile(pFileName);

	if (g_playStream)
		StartPlayStreamFile(pFileName);
	else
		StartDisplayImage(pFileName);

	g_bPlaying = TRUE;
	g_bPause   = FALSE;

	char filePath[256];
	char *p = NULL;
	memset(filePath, 0, sizeof(filePath));
	strcpy(filePath, pFileName);
	p = strrchr(filePath, '/');
	*p = 0;
	mTextview_fileNamePtr->setText(pFileName+strlen(filePath)+1);
	SetPlayingStatus(true);
	//AutoDisplayToolbar();
}

static void TogglePlayFile()
{
	if (g_bPlaying)
	{
		g_bPause = !g_bPause;
		SetPlayingStatus(!g_bPause);

		if (g_playStream)
			TogglePlayStreamFile();
		else
			TogglePlayImageFile();
	}
}

static void PlayNextFile()
{
	pthread_mutex_lock(&g_playFileMutex);
	g_eSkipMode = SKIP_NEXT;
	pthread_mutex_unlock(&g_playFileMutex);
}

static void PlayPrevFile()
{
	pthread_mutex_lock(&g_playFileMutex);
	g_eSkipMode = SKIP_PREV;
	pthread_mutex_unlock(&g_playFileMutex);
}

RepeatMode_e operator++(RepeatMode_e& cmd)
{
	RepeatMode_e t = cmd;
	cmd = static_cast<RepeatMode_e>(cmd+1);
	return t;
}

static void PollRepeatMode()
{
	RepeatMode_e eRepeatMode = FILE_REPEAT_MODE;
	pthread_mutex_lock(&g_playFileMutex);
	if (g_eRepeatMode < LIST_REPEAT_MODE)
		g_eRepeatMode++;
	else
		g_eRepeatMode = FILE_REPEAT_MODE;

	eRepeatMode = g_eRepeatMode;
	pthread_mutex_unlock(&g_playFileMutex);

	switch (eRepeatMode)
	{
	case FILE_REPEAT_MODE:
		mButton_circlemodePtr->setBackgroundPic("player/singlerepeat.png");
		break;
	case LIST_REPEAT_MODE:
		mButton_circlemodePtr->setBackgroundPic("player/listrepeat.png");
		break;
	default:
		printf("invalid repeat mode, mode=%d\n", (int)eRepeatMode);
		break;
	}
}

static void *PlayFileProc(void *pData)
{
	char *pFileName = (char*)pData;
	char curFileName[256] = {0};
	SkipMode_e eSkipMode = NO_SKIP;
	bool bPlayCompleted = false;
	bool bPlayError = false;

	strncpy(curFileName, pFileName, sizeof(curFileName));
	RepeatMode_e eRepeatMode = LIST_REPEAT_MODE;
	StartPlayFile(curFileName);
	AutoDisplayToolbar();

	while (!g_bPlayFileThreadExit)
	{
		pthread_mutex_lock(&g_playFileMutex);
		eRepeatMode = g_eRepeatMode;
		if (eSkipMode != g_eSkipMode)
		{
			eSkipMode = g_eSkipMode;
		}
		g_eSkipMode = NO_SKIP;
		if (bPlayError != g_bPlayError)
		{
			bPlayError = g_bPlayError;
			g_bPlayError = false;
		}
		if (bPlayCompleted != g_bPlayCompleted)
		{
			bPlayCompleted = g_bPlayCompleted;
			g_bPlayCompleted = false;
		}
		pthread_mutex_unlock(&g_playFileMutex);

		if (bPlayError)
		{
			printf("occur error when playing file\n");
			break;
		}

		if (bPlayCompleted)
		{
			if (eRepeatMode == LIST_REPEAT_MODE)
			{
				char nextFileName[256] = {0};
				GetNextFile(curFileName, nextFileName, sizeof(nextFileName));
				memset(curFileName, 0, sizeof(curFileName));
				strncpy(curFileName, nextFileName, sizeof(curFileName));
			}

			StopPlayFile();
			StartPlayFile(curFileName);
		}
		else
		{
			if (eSkipMode == SKIP_NEXT)
			{
				char nextFileName[256] = {0};
				GetNextFile(curFileName, nextFileName, sizeof(nextFileName));
				if (strcmp(curFileName, nextFileName))	// if only one file, not change
				{
					memset(curFileName, 0, sizeof(curFileName));
					strncpy(curFileName, nextFileName, sizeof(curFileName));
					StopPlayFile();
					StartPlayFile(curFileName);
				}
			}
			else if (eSkipMode == SKIP_PREV)
			{
				char prevFileName[256] = {0};
				GetPrevFile(curFileName, prevFileName, sizeof(prevFileName));
				if (strcmp(curFileName, prevFileName))	// if only one file, not change
				{
					memset(curFileName, 0, sizeof(curFileName));
					strncpy(curFileName, prevFileName, sizeof(curFileName));
					StopPlayFile();
					StartPlayFile(curFileName);
				}
			}
		}

		usleep(100000);
	}

	StopPlayFile();
	g_fileName = curFileName;

	return NULL;
}

#endif
/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	printf("create player dev\n");

#ifdef SUPPORT_PLAYER_MODULE
	// init play view real size
	LayoutPosition layoutPos = mVideoview_videoPtr->getPosition();
	g_playViewWidth = layoutPos.mWidth * PANEL_MAX_WIDTH / UI_MAX_WIDTH;
	g_playViewHeight = ALIGN_DOWN(layoutPos.mHeight * PANEL_MAX_HEIGHT / UI_MAX_HEIGHT, 2);
	printf("play view size: w=%d, h=%d\n", g_playViewWidth, g_playViewHeight);

	SSTAR_RegisterUsbListener(DetectUsbHotplug);
	// init pts
	g_firstPlayPos = PLAY_INIT_POS;

	// divp use window max width & height default, when play file, the inputAttr of divp will be set refer to file size.
	CreatePlayerDev();

	pthread_mutex_init(&g_playFileMutex, NULL);		// playFile mutex init
#endif
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
#ifdef SUPPORT_PLAYER_MODULE
    g_fileName = intentPtr->getExtra("filepath");

    g_bPlayFileThreadExit = false;
    pthread_create(&g_playFileThread, NULL, PlayFileProc, (void*)g_fileName.c_str());
#endif
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {

}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {

}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	printf("destroy player dev\n");
#ifdef SUPPORT_PLAYER_MODULE
	pthread_mutex_destroy(&g_playFileMutex);
	DestroyPlayerDev();
	g_firstPlayPos = PLAY_INIT_POS;

	printf("start to UnRegisterUsbListener\n");
	SSTAR_UnRegisterUsbListener(DetectUsbHotplug);
	printf("end of UnRegisterUsbListener\n");
#endif
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id){
	switch (id) {

		default:
			break;
	}
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onplayerActivityTouchEvent(const MotionEvent &ev) {
#ifdef SUPPORT_PLAYER_MODULE
	static SZKPoint touchDown;
	static SZKPoint touchMove;
	static SZKPoint lastMove;
	static bool bValidMove = false;	// on the first move, delt y should be larger than delt x, or update touchDown point

    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			//printf("down: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
			touchDown.x = ev.mX;
			touchDown.y = ev.mY;
			bValidMove = false;

			// judge if the model window is visible
			if (mWindow_errMsgPtr->isVisible() && !mWindow_errMsgPtr->getPosition().isHit(ev.mX, ev.mY))
			{
				return true;
			}

			// show play bar when touch down
			AutoDisplayToolbar();

			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			//printf("move: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
			touchMove.x = ev.mX;
			touchMove.y = ev.mY;

			if (!bValidMove)
			{
				if (touchMove.y == touchDown.y)
				{
					touchDown = touchMove;
				}
				else if (touchMove.x == touchDown.x)
				{
					bValidMove = true;
					AdjustVolumeByTouch((int)touchDown.y, (int)touchMove.y);
					lastMove = touchMove;
				}
				else if ((touchMove.y-touchDown.y) > 0 && (touchMove.x-touchDown.x) > 0
						&& (touchMove.y-touchDown.y) >= (touchMove.x-touchDown.x))
				{
					bValidMove = true;
					AdjustVolumeByTouch((int)touchDown.y, (int)touchMove.y);
					lastMove = touchMove;
				}
				else if ((touchMove.y-touchDown.y) < 0 && (touchMove.x-touchDown.x) < 0
						&& (touchMove.y-touchDown.y) <= (touchMove.x-touchDown.x))
				{
					bValidMove = true;
					AdjustVolumeByTouch((int)touchDown.y, (int)touchMove.y);
					lastMove = touchMove;
				}
				else if ((touchMove.y-touchDown.y) > 0 && (touchMove.x-touchDown.x) < 0
						&& (touchMove.y-touchDown.y) >= (touchDown.x-touchMove.x))
				{
					bValidMove = true;
					AdjustVolumeByTouch((int)touchDown.y, (int)touchMove.y);
					lastMove = touchMove;
				}
				else if ((touchMove.y-touchDown.y) < 0 && (touchMove.x-touchDown.x) > 0
						&& (touchDown.y-touchMove.y) >= (touchMove.x-touchDown.x))
				{
					bValidMove = true;
					AdjustVolumeByTouch((int)touchDown.y, (int)touchMove.y);
					lastMove = touchMove;
				}
				else
				{
					touchDown = touchMove;
				}
			}
			else
			{
				//printf("lastY:%d, curY:%d\n", lastMove.y, touchMove.y);
				AdjustVolumeByTouch(lastMove.y, touchMove.y);
				lastMove = touchMove;
			}

			AutoDisplayToolbar();
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			//printf("up: time=%ld, x=%d, y=%d\n", ev.mEventTime, ev.mX, ev.mY);
			break;
		default:
			break;
	}
#endif
	return false;
}
static void onProgressChanged_Seekbar_progress(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged Seekbar_progress %d !!!\n", progress);
}

static void onStartTrackingTouch_Seekbar_progress(ZKSeekBar *pSeekBar) {
    //LOGD(" ProgressChanged Seekbar_progress %d !!!\n", progress);
#ifdef SUPPORT_PLAYER_MODULE

	printf("onStartTrackingTouch_Seekbar_progress\n");
	if (g_playStream)
	{
		if (!g_bPause)
			toggle_pause(g_pstPlayStat);
	}
	else
	{
		if (!g_bPause)
			ImagePlayer_TogglePause(g_pstImagePlayer);
	}
#endif
}

static void onStopTrackingTouch_Seekbar_progress(ZKSeekBar *pSeekBar) {
    //LOGD(" ProgressChanged Seekbar_progress %d !!!\n", progress);
#ifdef SUPPORT_PLAYER_MODULE
	printf("onStopTrackingTouch_Seekbar_progress\n");

	int progress = pSeekBar->getProgress();
	long long curPos = progress * g_duration / mSeekbar_progressPtr->getMax();
	printf("progress value is %d, max value is %d, duration is %lld, curPos is %lld\n", progress, mSeekbar_progressPtr->getMax(),
			g_duration, curPos);

	if (g_playStream)
	{
		//stream_seek(g_pstPlayStat, curPos, (curPos - g_lastpos), 0);
		stream_seek(g_pstPlayStat, curPos, 0, 0);		// fot test
		if (!g_bPause)
			toggle_pause(g_pstPlayStat);
	}
	else
	{
		printf("UI: image seek pos is %lld\n", curPos);
		ImagePlayer_Seek(g_pstImagePlayer, curPos);
		if (!g_bPause)
			ImagePlayer_TogglePause(g_pstImagePlayer);
	}
#endif
}

static bool onButtonClick_Button_play(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_play !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	TogglePlayFile();
#endif
    return false;
}

static bool onButtonClick_Button_stop(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_stop !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	if (g_hideToolbarThread.isRunning())
	{
		printf("stop hideToolBarthread\n");
		g_hideToolbarThread.requestExitAndWait();
	}

	g_bPlayFileThreadExit = true;
	if (g_playFileThread)
	{
		pthread_join(g_playFileThread, NULL);
		g_playFileThread = NULL;
	}

	EASYUICONTEXT->goBack();
#endif
    return false;
}

static bool onButtonClick_Button_slow(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_slow !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	char speedMode[16] = {0};

	if (!g_playStream)
		return false;

	if (g_bPlaying)
	{
		if (g_ePlayDirection == E_PLAY_FORWARD)
		{
			// slow down
			if (g_ePlayMode == E_PLAY_FAST_MODE)
			{
				g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode - 1);
				g_u32SpeedNumerator = 1 << (int)g_eSpeedMode;
				g_u32SpeedDenomonator = 1;

				if (g_eSpeedMode == E_NORMAL_SPEED)
					g_ePlayMode = E_PLAY_NORMAL_MODE;
			}
			else
			{
				if (g_eSpeedMode < E_32X_SPEED)
				{
					g_ePlayMode = E_PLAY_SLOW_MODE;
					g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode + 1);
				}
				else    // turn to play backward
				{
					g_ePlayDirection = E_PLAY_BACKWARD;
					g_ePlayMode = E_PLAY_NORMAL_MODE;
					g_eSpeedMode = E_NORMAL_SPEED;
				}

				g_u32SpeedNumerator = 1;
				g_u32SpeedDenomonator = 1 << (int)g_eSpeedMode;
			}
		}
		else
		{
			// speed up
			if (g_ePlayMode == E_PLAY_SLOW_MODE)
			{
				g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode - 1);
				g_u32SpeedNumerator = 1;
				g_u32SpeedDenomonator = 1 << (int)g_eSpeedMode;

				if (g_eSpeedMode == E_NORMAL_SPEED)
				{
					g_ePlayMode = E_PLAY_NORMAL_MODE;
				}
			}
			else
			{
				if (g_eSpeedMode < E_32X_SPEED)
				{
					g_ePlayMode = E_PLAY_FAST_MODE;
					g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode + 1);
					g_u32SpeedNumerator = 1 << (int)g_eSpeedMode;
					g_u32SpeedDenomonator = 1;
				}
			}
		}

		memset(speedMode, 0, sizeof(speedMode));
		if (g_u32SpeedNumerator == g_u32SpeedDenomonator)
			//sprintf(speedMode, "", g_u32SpeedNumerator);
			memset(speedMode, 0, sizeof(speedMode));
		else if (g_u32SpeedNumerator > g_u32SpeedDenomonator)
			sprintf(speedMode, "%s %dX", ((g_ePlayDirection == E_PLAY_FORWARD) ? ">>" : "<<"), g_u32SpeedNumerator);
		else
			sprintf(speedMode, "%s 1/%dX", ((g_ePlayDirection == E_PLAY_FORWARD) ? ">>" : "<<"), g_u32SpeedDenomonator);

		mTextview_speedPtr->setText(speedMode);

		// sendmessage to adjust speed
	}
#endif
    return false;
}

static bool onButtonClick_Button_fast(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_fast !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	char speedMode[16] = {0};

	if (!g_playStream)
		return false;

	if (g_bPlaying)
	{
		if (g_ePlayDirection == E_PLAY_FORWARD)
		{
			// speed up
			if (g_ePlayMode == E_PLAY_SLOW_MODE)
			{
				g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode - 1);
				g_u32SpeedNumerator = 1;
				g_u32SpeedDenomonator = 1 << (int)g_eSpeedMode;

				if (g_eSpeedMode == E_NORMAL_SPEED)
					g_ePlayMode = E_PLAY_NORMAL_MODE;
			}
			else
			{
				if (g_eSpeedMode < E_32X_SPEED)
				{
					g_ePlayMode = E_PLAY_FAST_MODE;
					g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode + 1);
					g_u32SpeedNumerator = 1 << (int)g_eSpeedMode;
					g_u32SpeedDenomonator = 1;
				}
			}
		}
		else
		{
			// slow down
			if (g_ePlayMode == E_PLAY_FAST_MODE)
			{
				g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode - 1);
				g_u32SpeedNumerator = 1 << (int)g_eSpeedMode;
				g_u32SpeedDenomonator = 1;

				if (g_eSpeedMode == E_NORMAL_SPEED)
					g_ePlayMode = E_PLAY_NORMAL_MODE;
			}
			else
			{
				// 1/32X speed backward to normal speed forward
				if (g_eSpeedMode == E_32X_SPEED)
				{
					g_eSpeedMode = E_NORMAL_SPEED;
					g_ePlayMode = E_PLAY_NORMAL_MODE;
					g_ePlayDirection = E_PLAY_FORWARD;
				}
				else
				{
					g_ePlayMode = E_PLAY_SLOW_MODE;
					g_eSpeedMode = (PlaySpeedMode_e)((int)g_eSpeedMode + 1);
				}

				g_u32SpeedNumerator = 1;
				g_u32SpeedDenomonator = 1 << (int)g_eSpeedMode;
			}
		}

		memset(speedMode, 0, sizeof(speedMode));
		if (g_u32SpeedNumerator == g_u32SpeedDenomonator)
			memset(speedMode, 0, sizeof(speedMode));
		else if (g_u32SpeedNumerator > g_u32SpeedDenomonator)
			sprintf(speedMode, "%dX %s", g_u32SpeedNumerator, ((g_ePlayDirection == E_PLAY_FORWARD) ? ">>" : "<<"));
		else
			sprintf(speedMode, "1/%dX %s", g_u32SpeedDenomonator, ((g_ePlayDirection == E_PLAY_FORWARD) ? ">>" : "<<"));

		mTextview_speedPtr->setText(speedMode);

		// sendmessage to adjust speed
	}
#endif
    return false;
}
static bool onButtonClick_Button_voice(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_voice !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	g_bMute = !g_bMute;
	MI_AO_SetMute(AUDIO_DEV, g_bMute);
	SetMuteStatus(g_bMute);
	printf("set mute to %d\n", g_bMute);
#endif
    return false;
}

static void onProgressChanged_Seekbar_volumn(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged Seekbar_volumn %d !!!\n", progress);
#ifdef SUPPORT_PLAYER_MODULE
	MI_S32 vol = 0;
	MI_AO_ChnState_t stAoState;

	printf("voice changed!!!!!!!!\n");
	g_s32VolValue = GetPlayerVolumn();
	if (g_s32VolValue)
		vol = g_s32VolValue * (MAX_ADJUST_AO_VOLUME - MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
	else
		vol = MIN_AO_VOLUME;

	memset(&stAoState, 0, sizeof(MI_AO_ChnState_t));
	if (MI_SUCCESS == MI_AO_QueryChnStat(AUDIO_DEV, AUDIO_CHN, &stAoState))
	{
		MI_AO_SetVolume(AUDIO_DEV, vol);
		MI_AO_SetMute(AUDIO_DEV, g_bMute);
	}
#endif
}
static bool onButtonClick_Button_confirm(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_confirm !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	mWindow_errMsgPtr->setVisible(false);
	EASYUICONTEXT->goBack();
#endif
    return false;
}
static bool onButtonClick_Button_prev(ZKButton *pButton) {
    LOGD(" ButtonClick Button_prev !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    PlayPrevFile();
#endif
    return false;
}

static bool onButtonClick_Button_next(ZKButton *pButton) {
    LOGD(" ButtonClick Button_next !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    PlayNextFile();
#endif
    return false;
}

static bool onButtonClick_Button_circlemode(ZKButton *pButton) {
    LOGD(" ButtonClick Button_circlemode !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
    PollRepeatMode();
#endif
    return false;
}
