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


#include "SocketClient.h"
#include "manager/LanguageManager.h"
#include "os/SystemProperties.h"
#include "base_types.h"
#include "statusbarconfig.h"

extern "C"{
#include "voicedetect.h"

typedef enum {
	START_PLAY_MUSIC = 5,
	STOP_PLAY_MUSIC = 8,
	ALL_APP_CMD
} AppCmd_e;

int SSTAR_StartPlayAudioFile(const char *WavAudioFile, int aoVolume);
int SSTAR_StopPlayAudioFile(void);
void SSTAR_setVolume(int vol);
}

#define WORD_COUNT			50

static bool sIsSeled[WORD_COUNT];
static HANDLE g_hDSpotter = NULL;
static TrainedWord_t g_stWordList[WORD_COUNT];
static int g_nWordCnt = 0;

static void* onVoiceAnalyzeCallback(int commandID) {
	//LOGD("Get cmd id: %d\n", commandID);

	if (commandID >= 0 && commandID < WORD_COUNT)
	{
		for (int i=0; i < WORD_COUNT; i++)
			memset(sIsSeled, 0, sizeof(sIsSeled));

		sIsSeled[commandID] = true;
		mWordListviewPtr->refreshListView();
	}

	return NULL;
}


/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
//	{0,  1000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

// 目前没有检测升级完成的注册接口，先用以下挫的方法处理
class CheckThread : public Thread {
protected:
	virtual bool readyToRun() {
		mIsUpgrading = false;
		return true;
	}

	virtual bool threadLoop() {
#if 0
		if (!mIsUpgrading) {
			mIsUpgrading = UPGRADEMONITOR->isUpgrading();
			if (mIsUpgrading) {
				LOGD("Upgrading ...\n");
			}
		} else {
			if (!UPGRADEMONITOR->isUpgrading()) {
				remove(UPGRADE_FILE_PATH);
				LOGD("Upgrade end ...\n");
				return false;
			}
		}
#endif
		sleep(10);
		return true;
	}

private:
	bool mIsUpgrading;
};

static CheckThread sCheckThread;

class iWiFiSocketListener : public Uploader::ISocketListener {
public:
	virtual void notify(int what, int status, const char *msg){

	}
};

static iWiFiSocketListener mWifiSocket;
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
    memset(sIsSeled, 0, sizeof(sIsSeled));
    g_nWordCnt = SSTAR_VoiceDetectGetWordList(g_stWordList, WORD_COUNT);
	g_hDSpotter = SSTAR_VoiceDetectInit();
	if (!g_hDSpotter)
	{
		printf("Voice detect init failed\n");
		return;
	}

	SSTAR_VoiceDetectStart(g_hDSpotter, onVoiceAnalyzeCallback);
}

static void onUI_quit() {
	printf("onUi quit\n");
	SSTAR_VoiceDetectStop();
	SSTAR_VoiceDetectDeinit(g_hDSpotter);
	ShowStatusBar(1, 0, 0);
}


static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool bSocketConnect = false;
static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
	return true;
}


static bool onadActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上
    return false;
}

static bool onButtonClick_Button1(ZKButton *pButton) {
    //LOGD(" ButtonClick Button1 !!!\n");
	EASYUICONTEXT->openActivity("NetSettingActivity");
    return true;
}

static bool onButtonClick_ButtonConnectSev(ZKButton *pButton) {
    return true;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}

static void onProgressChanged_DownloadSeekbar(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged DownloadSeekbar %d !!!\n", progress);
}

static int getListItemCount_WordListview(const ZKListView *pListView) {
    //LOGD("getListItemCount_WordListview !\n");
    return WORD_COUNT;
}

static void obtainListItemData_WordListview(ZKListView *pListView, ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ WordListview  !!!\n");
	if (index < g_nWordCnt)
	{
		pListItem->setText(g_stWordList[index].cmd);
		pListItem->setSelected(sIsSeled[index]);
	}
	else
	{
		pListItem->setText("");
		pListItem->setSelected(false);
	}

	pListItem->setSelected(sIsSeled[index]);
}

static void onListItemClick_WordListview(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ WordListview  !!!\n");
}
