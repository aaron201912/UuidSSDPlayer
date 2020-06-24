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

#include "appconfig.h"
#include "tts.h"
#include "statusbarconfig.h"

static bool g_bStopped = true;
static bool g_bPaused = false;
static HANDLE g_hTts = NULL;
static bool g_bInit = false;
static const char g_textStr[] = "和泰车的TOYOTA与LEXUS品牌在4月共销售9786辆新车，虽较3月下滑8.7%，但仍保有34.2%市占率；但其4月营收51.95亿元，较3月的88亿元大幅衰退将近4成。";

static int *g_pLangID = NULL;
static int g_nLangCnt = 0;
static int g_nSpeakerCnt = 0;
static int g_nLangIdx = 0;
static int g_nSpeakerIdx = 0;

extern Language g_language[LANG_SUPPORT_NUM];
extern Speaker g_speaker[SPEAKER_SUPPORT_NUM];

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
	// get text from m_pchText
	g_pLangID = (int*)malloc(sizeof(int) * TTS_GetLanguageMaxNum());
	if (!g_pLangID)
	{
		printf("alloc memory fail\n");
		return;
	}

	g_nLangCnt = TTS_GetAvailableLangID(CREADER_DATA_PATH, g_pLangID);
	if (g_nLangCnt <= 0)
	{
		printf("Get lang ID fail\n");
		return;
	}

	g_nSpeakerCnt = TTS_GetAvailableSpeaker(CREADER_LIB_PATH, CREADER_DATA_PATH, g_pLangID[g_nLangIdx]);
	if (g_nSpeakerCnt <= 0)
	{
		printf("Get speaker fail\n");
		return;
	}

	mButton_langPtr->setText(g_language[g_pLangID[g_nLangIdx]].Lang);
	mButton_speakerPtr->setText(g_speaker[g_nSpeakerIdx].speaker);
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }

    mTextview_textPtr->setText(g_textStr);
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
	g_bInit = false;

	if(g_hTts != NULL)
	{
		if (!g_bStopped)
		{
			g_bStopped = true;
			TTS_Stop(g_hTts);
		}

		TTS_Release(g_hTts);
		g_hTts = NULL;
	}


	if (g_pLangID)
	{
		free(g_pLangID);
		g_pLangID = NULL;
	}

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
static bool onttsActivityTouchEvent(const MotionEvent &ev) {
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

static bool onButtonClick_Button_getVol(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_getVol !!!\n");
	if (!g_bInit)
		return false;

    return false;
}

static bool onButtonClick_Button_play(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_play !!!\n");
	if (!g_bInit)
		return false;

	if (g_bStopped)
	{
		g_bStopped = false;
		g_bPaused = false;
		TTS_Start(g_hTts, CREADER_SAVE_FILE);

		printf("click play btn: play\n");
	}
	else
	{
		g_bPaused = !g_bPaused;

		if (g_bPaused)
			TTS_Pause(g_hTts);
		else
			TTS_Resume(g_hTts);

		printf("click play btn: %s\n", g_bPaused?"pause":"resume");
	}

	mButton_initPtr->setInvalid(true);
	mButton_playPtr->setSelected(!g_bPaused);

    return false;
}

static bool onButtonClick_Button_stop(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_stop !!!\n");
	if (!g_bInit)
		return false;

	if (!g_bStopped)
	{
		g_bStopped = true;
		g_bPaused = false;
		TTS_Stop(g_hTts);
	}

	printf("click btn: stop\n");

	mButton_initPtr->setInvalid(false);
	mButton_playPtr->setSelected(g_bPaused);

    return false;
}

static bool onButtonClick_Button_setVol(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_setVol !!!\n");
	if (!g_bInit)
		return false;

    return false;
}

static bool onButtonClick_Button_getSpeed(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_getSpeed !!!\n");
	if (!g_bInit)
		return false;

    return false;
}

static bool onButtonClick_Button_setSpeed(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_setSpeed !!!\n");
	if (!g_bInit)
		return false;

    return false;
}

static bool onButtonClick_Button_getPitch(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_getPitch !!!\n");
	if (!g_bInit)
		return false;

    return false;
}

static bool onButtonClick_Button_setPitch(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_setPitch !!!\n");
	if (!g_bInit)
		return false;

    return false;
}
static bool onButtonClick_Button_lang(ZKButton *pButton) {
    LOGD(" ButtonClick Button_lang !!!\n");
    bool bVisible = mListView_langPtr->isVisible();
    if (!bVisible)
    	pButton->setText("");
    mListView_langPtr->setVisible(!bVisible);
    mListView_langPtr->setSelection(g_nLangIdx);
    return false;
}

static bool onButtonClick_Button_speaker(ZKButton *pButton) {
    LOGD(" ButtonClick Button_speaker !!!\n");
    bool bVisible = mListView_speakerPtr->isVisible();
    if (!bVisible)
    	pButton->setText("");
    mListView_speakerPtr->setVisible(!bVisible);
    mListView_speakerPtr->setSelection(g_nSpeakerIdx);
    return false;
}

static int getListItemCount_ListView_lang(const ZKListView *pListView) {
    //LOGD("getListItemCount_ListView_lang !\n");
	return g_nLangCnt;
}

static void obtainListItemData_ListView_lang(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListView_lang  !!!\n");
	pListItem->setText(g_language[g_pLangID[index]].Lang);

	if (index == g_nLangIdx)
	{
		pListItem->setSelected(true);
		mButton_langPtr->setText(g_language[g_pLangID[index]].Lang);
	}
	else
		pListItem->setSelected(false);
}

static void onListItemClick_ListView_lang(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ ListView_lang  !!!\n");
	if (g_nLangIdx != index)
	{
		g_nLangIdx = index;
		g_nSpeakerIdx = 0;

		g_nSpeakerCnt = TTS_GetAvailableSpeaker(CREADER_LIB_PATH, CREADER_DATA_PATH, g_pLangID[g_nLangIdx]);
		if (g_nSpeakerCnt <= 0)
		{
			printf("Get speaker fail\n");
			mButton_speakerPtr->setText("");
			pListView->setVisible(false);
			return;
		}

//		mButton_speakerPtr->setText("");
		mListView_speakerPtr->setSelection(0);
		mListView_speakerPtr->setVisible(true);
		//mListView_speakerPtr->setVisible(false);
		//mListView_speakerPtr->refreshListView();
		printf("refresh mListView_speakerPtr\n");
	}

	pListView->setVisible(false);
}

static int getListItemCount_ListView_speaker(const ZKListView *pListView) {
    //LOGD("getListItemCount_ListView_speaker !\n");
    return g_nSpeakerCnt;
}

static void obtainListItemData_ListView_speaker(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListView_speaker  !!!\n");
	pListItem->setText(g_speaker[index].speaker);
	printf("obtainListItemData_ListView_speaker, index=%d\n", index);

	if (index == g_nSpeakerIdx)
	{
		pListItem->setSelected(true);
		mButton_speakerPtr->setText(g_speaker[index].speaker);
	}
	else
		pListItem->setSelected(false);
}

static void onListItemClick_ListView_speaker(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ ListView_speaker  !!!\n");
	if (g_nSpeakerIdx != index)
		g_nSpeakerIdx = index;

	pListView->setVisible(false);
}

static void TTS_StopPlayCallback()
{
	if (!g_bStopped)
	{
		g_bStopped = true;
		g_bPaused = false;
	}

	printf("tts stop callback\n");

	mButton_initPtr->setInvalid(false);
	mButton_playPtr->setSelected(g_bPaused);
}

static bool onButtonClick_Button_init(ZKButton *pButton) {
    LOGD(" ButtonClick Button_init !!!\n");
    char *pText = mTextview_textPtr->getText().c_str();

    if (g_hTts)
    {
    	TTS_Release(g_hTts);
    	g_hTts = NULL;
    }

    if (g_pLangID && g_nLangCnt && g_nSpeakerCnt && strlen(pText))
    {
    	g_hTts = TTS_Initialize(CREADER_LIB_PATH, CREADER_DATA_PATH, g_pLangID[g_nLangIdx], g_speaker[g_nSpeakerIdx].speaker, TTS_StopPlayCallback);
		if (!g_hTts)
		{
			printf("fail to initialize tts\n");
			return false;
		}

		//Read TTS string from file
		if(TTS_AddTTSStringUTF8(g_hTts, pText) != __PLAYER_SUCCESS__)
		{
			printf("Fail to read TTS string!\r\n");
			return false;
		}

		g_bInit = true;
    }

    return false;
}

