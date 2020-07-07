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
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "v4l2.h"
#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_divp.h"
#include "mi_disp.h"
#include "panelconfig.h"
#include "statusbarconfig.h"

#define VDEC_CHN				0
#define DIVP_CHN        		0
#define DISP_DEV        		0
#define DISP_LAYER      		0
#define DISP_INPUTPORT  		0

#define assert(p)\
    do {\
        if(p)\
            printf("%s %d param valid %d\n", __func__, __LINE__, p);\
        else\
        {\
            printf("%s %d abort()\n", __func__, __LINE__);\
            abort();\
        }\
    } while(0)


/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

static int g_width = 1280;
static int g_height = 720;
static DeviceContex_t *ctx = NULL;
static Packet pkt;
static Mutex lLock;
static bool g_bExit = false;

int V4L2_SendRawData(int vdecChn, unsigned char *pu8Buffer, unsigned int len);

class V4L2DisplayThread : public Thread {
protected:
	virtual bool threadLoop() {
		int ret = 0;
		lLock.lock();
		bool bExit = g_bExit;
		lLock.unlock();
		if (bExit)
			return false;

		ret = v4l2_read_packet(ctx, &pkt);
		if(ret == -EAGAIN) {
		   usleep(10000);
		   return true;
		}

		if(ret >=0) {
		   //printf("Get Pkt size=%d\n", pkt.size);
		   //save_file(pkt.data, pkt.size, 1);
		   V4L2_SendRawData(VDEC_CHN, (unsigned char *)pkt.data, pkt.size);
		   v4l2_read_packet_end(ctx, &pkt);
		}
		
		return true;
	}

private:
	struct v4l2_buf_unit *m_pV4L2_buf_unit;
};

static V4L2DisplayThread g_displayThread;

// openUvcDev
void V4L2_OpenDev(int width, int height)
{
	v4l2_dev_init(&ctx,  (char*)"/dev/video0");
	v4l2_dev_set_fmt(ctx, V4L2_PIX_FMT_H264, width, height);
	if (v4l2_read_header(ctx))
	{
		printf("Can't find usb camera\n");
		mTextview_tipsPtr->setVisible(true);
		return;
	}

	// create thread
	if (!g_displayThread.isRunning())
	{
		lLock.lock();
		g_bExit = false;
		lLock.unlock();
		printf("create v4l2 thread\n");
		g_displayThread.run("v4l2");
	}
}

// closeUvcDev
void V4L2_CloseDev()
{
	lLock.lock();
	g_bExit = true;
	lLock.unlock();

	if (g_displayThread.isRunning())
	{
		printf("wait v4l2 thread to exit\n");
		g_displayThread.requestExitAndWait();
	}

	v4l2_read_close(ctx);
	v4l2_dev_deinit(ctx);
}

int V4L2_SendRawData(int vdecChn, unsigned char *buf, unsigned int len)
{
    static MI_U64 u64Pts = 0;
    int s32Ret = -1;
    MI_VDEC_VideoStream_t stVdecStream;
    stVdecStream.pu8Addr = buf;
	stVdecStream.u32Len = len;
	stVdecStream.u64PTS = u64Pts;
	stVdecStream.bEndOfFrame = 1;
	stVdecStream.bEndOfStream = 0;

	while (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, 20)))
	{
		printf("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", vdecChn, s32Ret);
	}

	u64Pts = u64Pts + 33;

    return 0;
}

MI_S32 CreateV4L2Pipe()
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    MI_SYS_ChnPort_t stDivpChnPort;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;

    MI_DISP_DEV dispDev = DISP_DEV;
    MI_DISP_LAYER dispLayer = DISP_LAYER;
    MI_U32 u32InputPort = DISP_INPUTPORT;
    MI_SYS_ChnPort_t stDispChnPort;
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_RotateConfig_t stRotateConfig;
    MI_PANEL_LinkType_e eLinkType;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    stDivpChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId = 0;
    stDivpChnPort.u32ChnId = DIVP_CHN;
    stDivpChnPort.u32PortId = 0;
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = 0;
    stDivpChnAttr.stCropRect.u16Height  = 0;
    stDivpChnAttr.u32MaxWidth           = 1920;	//PANEL_MAX_WIDTH;
    stDivpChnAttr.u32MaxHeight			= 1080;	//PANEL_MAX_HEIGHT;
    stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stOutputPortAttr.u32Width           = PANEL_MAX_WIDTH;
    stOutputPortAttr.u32Height          = PANEL_MAX_HEIGHT;

    printf("divp maxW=%d, maxH=%d, w=%d, h=%d\n", stDivpChnAttr.u32MaxWidth, stDivpChnAttr.u32MaxHeight,
    		stOutputPortAttr.u32Width, stOutputPortAttr.u32Height);

    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    stDispChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId = DISP_DEV;
    stDispChnPort.u32ChnId = 0;
    stDispChnPort.u32PortId = DISP_INPUTPORT;

    stRotateConfig.eRotateMode = E_MI_DISP_ROTATE_NONE;
    MI_DISP_SetVideoLayerRotateMode(dispLayer, &stRotateConfig);

    MI_DISP_GetInputPortAttr(dispLayer, u32InputPort, &stInputPortAttr);
    stInputPortAttr.stDispWin.u16X      = 0;
    stInputPortAttr.stDispWin.u16Y      = 0;
	stInputPortAttr.stDispWin.u16Width  = PANEL_MAX_WIDTH;
	stInputPortAttr.stDispWin.u16Height = PANEL_MAX_HEIGHT;

    if (stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_NONE || stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_180)
    {
		stInputPortAttr.u16SrcWidth = PANEL_MAX_WIDTH;
		stInputPortAttr.u16SrcHeight = PANEL_MAX_HEIGHT;
		stOutputPortAttr.u32Width = PANEL_MAX_WIDTH;
		stOutputPortAttr.u32Height = PANEL_MAX_HEIGHT;
    }
    else
    {
		stInputPortAttr.u16SrcWidth = PANEL_MAX_HEIGHT;
		stInputPortAttr.u16SrcHeight = PANEL_MAX_WIDTH;
		stOutputPortAttr.u32Width = PANEL_MAX_HEIGHT;
		stOutputPortAttr.u32Height = PANEL_MAX_WIDTH;
    }

    printf("disp input: w=%d, h=%d\n", stInputPortAttr.u16SrcWidth, stInputPortAttr.u16SrcHeight);

    MI_DIVP_CreateChn(DIVP_CHN, &stDivpChnAttr);
	MI_DIVP_StartChn(DIVP_CHN);
	MI_DIVP_SetOutputPortAttr(DIVP_CHN, &stOutputPortAttr);
	MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);

    MI_DISP_DisableInputPort(dispLayer, u32InputPort);
    MI_DISP_SetInputPortAttr(dispLayer, u32InputPort, &stInputPortAttr);
    MI_DISP_GetInputPortAttr(dispLayer, u32InputPort, &stInputPortAttr);

    MI_DISP_EnableInputPort(dispLayer, u32InputPort);
    MI_DISP_SetInputPortSyncMode(dispLayer, u32InputPort, E_MI_DISP_SYNC_MODE_FREE_RUN);
    MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

    return MI_SUCCESS;
}

void DestroyV4L2Pipe()
{
	MI_DISP_LAYER dispLayer = DISP_LAYER;
	MI_U32 u32InputPort = DISP_INPUTPORT;
    MI_SYS_ChnPort_t stDivpChnPort;
    MI_SYS_ChnPort_t stDispChnPort;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId = 0;
    stDivpChnPort.u32ChnId = DIVP_CHN;
    stDivpChnPort.u32PortId = 0;
    stDispChnPort.eModId = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId = DISP_DEV;
    stDispChnPort.u32ChnId = 0;
    stDispChnPort.u32PortId = DISP_INPUTPORT;
    MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);
    MI_DISP_DisableInputPort(dispLayer, u32InputPort);
    MI_DIVP_StopChn(DIVP_CHN);
    MI_DIVP_DestroyChn(DIVP_CHN);
}


MI_S32 StartCapture()
{
    MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);
	return 0;
}

void StopCapture()
{
	MI_DISP_ClearInputPortBuffer(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_HideInputPort(DISP_LAYER, DISP_INPUTPORT);
}

int V4L2_EnableVdec(int vdecChn, int divpChn, int width, int height, MI_VDEC_CodecType_e eCodecType)
{
	MI_SYS_ChnPort_t stVdecChnPort;
	MI_SYS_ChnPort_t stDivpChnPort;
    MI_VDEC_ChnAttr_t stVdecChnAttr;
	MI_VDEC_OutputPortAttr_t stOutputPortAttr;
	MI_S32 s32Ret = -1;
	int outWidth = width;
	int outHeight = height;

	if (outWidth > 1920)
		outWidth = 1920;

	if (outHeight > 1080)
		outHeight = 1080;

	memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
	stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
	stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
	stVdecChnAttr.u32BufSize    = 1 * 1024 * 1024;
	stVdecChnAttr.u32PicWidth   = width;
	stVdecChnAttr.u32PicHeight  = height;
	stVdecChnAttr.u32Priority   = 0;
	stVdecChnAttr.eCodecType    = eCodecType;
	stVdecChnAttr.eDpbBufMode = (MI_VDEC_DPB_BufMode_e)0;

	s32Ret = MI_VDEC_CreateChn(vdecChn, &stVdecChnAttr);
	if (s32Ret)
	{
		printf("create vdec chn failed\n");
		return -1;
	}

	s32Ret = MI_VDEC_StartChn(vdecChn);
	if (s32Ret)
	{
		printf("start vdec chn failed\n");
		return -1;
	}

	stOutputPortAttr.u16Width = outWidth;
	stOutputPortAttr.u16Height = outHeight;
	MI_VDEC_SetOutputPortAttr(vdecChn, &stOutputPortAttr);

    memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stVdecChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stVdecChnPort.u32DevId = 0;
    stVdecChnPort.u32ChnId = vdecChn;
    stVdecChnPort.u32PortId = 0;

    stDivpChnPort.eModId = E_MI_MODULE_ID_DIVP;
	stDivpChnPort.u32DevId = 0;
	stDivpChnPort.u32ChnId = divpChn;
	stDivpChnPort.u32PortId = 0;

	s32Ret = MI_SYS_BindChnPort(&stVdecChnPort, &stDivpChnPort, 0, 0);
	if (s32Ret)
	{
		printf("bind vdec & divp failed\n");
		return -1;
	}

    return 0;
}

int V4L2_DisableVdec(int vdecChn, int divpChn)
{
	MI_SYS_ChnPort_t stVdecChnPort;
	MI_SYS_ChnPort_t stDivpChnPort;
	MI_S32 s32Ret = MI_SUCCESS;

	memset(&stVdecChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stVdecChnPort.eModId = E_MI_MODULE_ID_VDEC;
	stVdecChnPort.u32DevId = 0;
	stVdecChnPort.u32ChnId = vdecChn;
	stVdecChnPort.u32PortId = 0;

	stDivpChnPort.eModId = E_MI_MODULE_ID_DIVP;
	stDivpChnPort.u32DevId = 0;
	stDivpChnPort.u32ChnId = divpChn;
	stDivpChnPort.u32PortId = 0;

	MI_SYS_UnBindChnPort(&stVdecChnPort, &stDivpChnPort);

	s32Ret = MI_VDEC_StopChn(vdecChn);
	if (MI_SUCCESS != s32Ret)
	{
		printf("%s %d, MI_VENC_StopRecvPic %d error, %X\n", __func__, __LINE__, vdecChn, s32Ret);
	}
	s32Ret |= MI_VDEC_DestroyChn(vdecChn);
	if (MI_SUCCESS != s32Ret)
	{
		printf("%s %d, MI_VENC_StopRecvPic %d error, %X\n", __func__, __LINE__, vdecChn, s32Ret);
	}

    return 0;
}
/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	CreateV4L2Pipe();
	V4L2_EnableVdec(VDEC_CHN, DIVP_CHN, g_width, g_height, E_MI_VDEC_CODEC_TYPE_H264);
	StartCapture();
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }

    V4L2_OpenDev(g_width, g_height);
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

	V4L2_CloseDev();
	StopCapture();
	V4L2_DisableVdec(VDEC_CHN, DIVP_CHN);
	DestroyV4L2Pipe();
	ShowStatusBar(1, 0, 0);
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
static bool oncameraActivityTouchEvent(const MotionEvent &ev) {
    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			break;
		default:
			break;
	}
	return false;
}
static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
