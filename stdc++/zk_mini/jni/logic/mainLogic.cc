#pragma once
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
*mTextXXX->setText("****") 在控件TextXXX上显示文字****
*mButton1->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBar->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/
//#include "SAT070CP50_1024x600.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_gfx.h"
#include "panelconfig.h"
#include "statusbarconfig.h"
#include "hotplugdetect.h"

static int g_curPageIdx = 0;
/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

#define USE_MIPI	0
#define MAKE_YUYV_VALUE(y,u,v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0,128,128)
#define DISP_INPUT_WIDTH        1024
#define DISP_INPUT_HEIGHT       600
#define DISP_OUTPUT_X           0
#define DISP_OUTPUT_Y           0
#define DISP_OUTPUT_WIDTH       1024
#define DISP_OUTPUT_HEIGHT      600

static void Enter_STR_SuspendMode()
{
	printf("disp disable 00\n");
    MI_DISP_DisableInputPort(0, 0);
    printf("disp disable 11\n");
    MI_DISP_DisableVideoLayer(0);
    printf("disp disable 22\n");
    MI_DISP_UnBindVideoLayer(0, 0);
    printf("disp disable 33\n");
    MI_DISP_Disable(0);
    MI_DISP_DeInitDev();
    printf("disp disable 55\n");
    MI_PANEL_DeInit();
    printf("panel disable\n");
    MI_GFX_DeInitDev();
    printf("gfx disable\n");
}

static void Enter_STR_ResumeMode()
{
    MI_PANEL_LinkType_e eLinkType;
	MI_DISP_InputPortAttr_t stInputPortAttr;

	MI_DISP_VideoLayerAttr_t stLayerAttr;
	MI_DISP_PubAttr_t stDispPubAttr;

	stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
	stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    if (stDispPubAttr.eIntfType == E_MI_DISP_INTF_LCD)
	{
		stDispPubAttr.stSyncInfo.u16Vact = stPanelParam.u16Height;
		stDispPubAttr.stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
		stDispPubAttr.stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
																	  stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
		stDispPubAttr.stSyncInfo.u16Hact = stPanelParam.u16Width;
		stDispPubAttr.stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
		stDispPubAttr.stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
																	  stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
		stDispPubAttr.stSyncInfo.u16Bvact = 0;
		stDispPubAttr.stSyncInfo.u16Bvbb = 0;
		stDispPubAttr.stSyncInfo.u16Bvfb = 0;
		stDispPubAttr.stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
		stDispPubAttr.stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
		stDispPubAttr.stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
		stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
		stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
		stDispPubAttr.u32BgColor = YUYV_BLACK;
	#if USE_MIPI
		eLinkType = E_MI_PNL_LINK_MIPI_DSI;
	#else
		eLinkType = E_MI_PNL_LINK_TTL;
	#endif
		stInputPortAttr.u16SrcWidth = DISP_INPUT_WIDTH;
		stInputPortAttr.u16SrcHeight = DISP_INPUT_HEIGHT;
		stInputPortAttr.stDispWin.u16X = DISP_OUTPUT_X;
		stInputPortAttr.stDispWin.u16Y = DISP_OUTPUT_Y;

		stInputPortAttr.stDispWin.u16Width = DISP_OUTPUT_WIDTH;
		stInputPortAttr.stDispWin.u16Height = DISP_OUTPUT_HEIGHT;

		//MI_DISP_Disable(0);
		//MI_DISP_DisableInputPort(0, 0);
		MI_DISP_SetPubAttr(0, &stDispPubAttr);

		MI_DISP_Enable(0);
		MI_DISP_BindVideoLayer(0, 0);
		memset(&stLayerAttr, 0, sizeof(stLayerAttr));

		stLayerAttr.stVidLayerSize.u16Width  = DISP_OUTPUT_WIDTH;
		stLayerAttr.stVidLayerSize.u16Height = DISP_OUTPUT_HEIGHT;

		stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
		stLayerAttr.stVidLayerDispWin.u16X      = DISP_OUTPUT_X;
		stLayerAttr.stVidLayerDispWin.u16Y      = DISP_OUTPUT_Y;

		stLayerAttr.stVidLayerDispWin.u16Width  = DISP_OUTPUT_WIDTH;
		stLayerAttr.stVidLayerDispWin.u16Height = DISP_OUTPUT_HEIGHT;

		MI_DISP_SetVideoLayerAttr(0, &stLayerAttr);
		MI_DISP_EnableVideoLayer(0);

		MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
		MI_DISP_EnableInputPort(0, 0);
		MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);
		//usleep(30*1000);
		//MI_DISP_Enable(0);
		//MI_DISP_EnableInputPort(0, 0);

	}
	if (stDispPubAttr.eIntfType == E_MI_DISP_INTF_LCD)
	{
		MI_PANEL_Init(eLinkType);
		MI_PANEL_SetPanelParam(&stPanelParam);
		if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
		{
#if USE_MIPI
			MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
#endif
		}
	}
	MI_GFX_Open();
}
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
	ShowStatusBar(1, 0, 0);
}

static void onUI_quit() {
}

static void onUI_show() {
	ShowStatusBar(1, 0, 0);
	printf("%s: enter show\n", __FILE__);
}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {

}

static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
    return true;
}


static bool onmainActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上

	switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//printf("down: x=%d, y=%d\n", ev.mX, ev.mY);

			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			//printf("move: x=%d, y=%d\n", ev.mX, ev.mY);

			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			//printf("up: x=%d, y=%d\n", ev.mX, ev.mY);
			break;
		default:
			break;
	}

    return false;
}
const char* IconTab[]={
		"testTextActivity",
		"testSliderActivity",
		"testButtonActivity",
		"inputtextActivity",
		"waveViewActivity",
		"windowActivity",
		"tesListActivity",
		"video2Activity",
		"voicedetectActivity",
		"qrcodeActivity",
		"animationActivity",
		"uartActivity",
		"networkSettingActivity"
};

static void onSlideItemClick_Slidewindow1(ZKSlideWindow *pSlideWindow, int index) {
	ShowStatusBar(0, 1, 1);		// 默认子页面hide statusbar，子页面需要显示时在子页面中设置

	printf("select idx is %d\n", index);
	EASYUICONTEXT->openActivity(IconTab[index]);
}

static void onSlidePageChange_Slidewindow1(ZKSlideWindow *pSlideWindow, int page) {
//	int totalPage = pSlideWindow->getPageSize();
	g_curPageIdx = pSlideWindow->getCurrentPage();
	//printf("Logic: param page is %d, total page is %d, cur page is %d\n", page, totalPage, g_curPageIdx);
	mListview_indicatorPtr->refreshListView();
}

static int getListItemCount_Listview_indicator(const ZKListView *pListView) {
    //LOGD("getListItemCount_Listview_indicator !\n");
    return 2;
}

static void obtainListItemData_Listview_indicator(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ Listview_indicator  !!!\n");
	if (index == g_curPageIdx)
		pListItem->setBackgroundPic("slider_/indicator_focus.png");
	else
		pListItem->setBackgroundPic("slider_/indicator.png");
}

static void onListItemClick_Listview_indicator(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ Listview_indicator  !!!\n");
	int curPageIdx =  g_curPageIdx;
	//printf("click idx is %d, curPageIdx is %d\n", index, g_curPageIdx);

	while (curPageIdx < index)
	{
		mSlidewindow1Ptr->turnToNextPage();
		curPageIdx++;
	}

	while (curPageIdx > index)
	{
		mSlidewindow1Ptr->turnToPrevPage();
		curPageIdx--;
	}
}
