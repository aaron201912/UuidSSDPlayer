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

/**
 *
 */
#include "manager/ConfigManager.h"
typedef struct {
    const char* icon;
    const char* text;

    int windowId; //需要特定显示弹出框id
    const char* toastTitle;  //用于一般询问弹出框的标题
} s_SETUP_ITEM;

static s_SETUP_ITEM g_setupItems[] = {
    "main/setup/list/tongsuo_n.png", "Lock", -1, "Whether to Open Lock?",
    "main/setup/list/tongdeng_n.png", "Light", -1, "Whether to Open Light?",
    "main/setup/list/wifi_n.png", "WIFI", -1, "Whether to Open WIFI?",
    "main/setup/list/shuiguanjia_n.png", "Plumber", -1, "Whether to Open Plumber?",
    "main/setup/list/jingyin_n.png", "Voice", -1, NULL,
    "main/setup/list/light_n.png", "Brightness", -1, NULL,
    "main/setup/list/language_n.png", "Language", -1, NULL,
    "main/setup/list/video_n.png", "Video", -1, NULL,
    "main/setup/list/czzy_n.png", "Guide", -1, NULL,
    "main/setup/list/reset_n.png", "Recovery", -1, NULL,
    "main/setup/list/ywbs_n.png", "Logo", -1, NULL,
    "main/setup/list/xyjq_n.png", "Skills", -1, NULL,
};

#define TIMER_SETUP_ANIMATION 200
#define ANIMATION_SETUP_WINDOW_ENTER 1
#define ANIMATION_SETUP_WINDOW_LEAVE 2
typedef struct {
	int intervalTime;//timer时间间隔
	int event;
	int locationIndex;
	bool animating; //正在动画中
}s_SETUP_WINDOW_ANIMATION;
s_SETUP_WINDOW_ANIMATION g_setupWindowAnimation = {0};
short animationLocation[] = {800,760,722,685,651,619,588,558,545};

/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
//	mTextview1Ptr->setText(CONFIGMANAGER->getResFilePath("fzcircle.ttf"));
	EASYUICONTEXT->hideStatusBar();
}

static void onUI_quit() {
//	EASYUICONTEXT->showStatusBar();
}


static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
	switch (id) {
		case TIMER_SETUP_ANIMATION:
		{
            static LayoutPosition pos = mWindowSetupPtr->getPosition();
            if ((g_setupWindowAnimation.locationIndex < 0)
            		|| (g_setupWindowAnimation.locationIndex >= sizeof(animationLocation)/sizeof(short))) {
               if (g_setupWindowAnimation.event == ANIMATION_SETUP_WINDOW_LEAVE) {
            	   mWindowSetupPtr->setVisible(false);
               }

        	   g_setupWindowAnimation.animating = false;
            	return false;
            }
            switch (g_setupWindowAnimation.event) {
				case ANIMATION_SETUP_WINDOW_ENTER:
                    pos.mLeft = animationLocation[g_setupWindowAnimation.locationIndex++];
					break;
				case ANIMATION_SETUP_WINDOW_LEAVE:
                    pos.mLeft = animationLocation[g_setupWindowAnimation.locationIndex--];
					break;
			}
			mWindowSetupPtr->setPosition(pos);
		}
			break;
	}
    return true;
}


static void windowSetupQuitAnimation() {
	g_setupWindowAnimation.intervalTime = 10;
	g_setupWindowAnimation.locationIndex = sizeof(animationLocation)/sizeof(short) -1;
	g_setupWindowAnimation.event = ANIMATION_SETUP_WINDOW_LEAVE;
	g_setupWindowAnimation.animating = true;
	mActivityPtr->registerUserTimer(TIMER_SETUP_ANIMATION ,g_setupWindowAnimation.intervalTime);
}

static bool onwasherActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上
    if (g_setupWindowAnimation.animating) {
    	return true;
    }

    if (!mWindowSetupPtr->getPosition().isHit(ev.mX, ev.mY) && !mWindowDialogPtr->isVisible() && mWindowSetupPtr->isVisible()) {
    	windowSetupQuitAnimation();
    	return true;
    }


    return false;
}

static void onSlideItemClick_SlidewindowMain(ZKSlideWindow *pSlideWindow, int index) {
    //LOGD(" onSlideItemClick_ SlidewindowMain %d !!!\n", index);
	Intent* intent = new Intent();
	char s[8] = {0};
	sprintf(s, "%d", index);
	intent->putExtra("target", s);
    EASYUICONTEXT->openActivity("detailActivity", intent);
}

static bool onButtonClick_btnFaverite(ZKButton *pButton) {
    //LOGD(" ButtonClick btnFaverite !!!\n");
	EASYUICONTEXT->openActivity("WifiSettingActivity");
    return true;
}

static bool onButtonClick_btnSetup(ZKButton *pButton) {
	g_setupWindowAnimation.intervalTime = 10;
	g_setupWindowAnimation.event = ANIMATION_SETUP_WINDOW_ENTER;
	g_setupWindowAnimation.locationIndex = 0;
	g_setupWindowAnimation.animating = true;
    LayoutPosition pos = mWindowSetupPtr->getPosition();
    pos.mLeft = 1024;
	mWindowSetupPtr->setPosition(pos);
    mWindowSetupPtr->setVisible(true);
    mActivityPtr->registerUserTimer(TIMER_SETUP_ANIMATION, g_setupWindowAnimation.intervalTime);
    return true;
}

static int getListItemCount_ListviewSetup(const ZKListView *pListView) {
    //LOGD(" getListItemCount_ ListviewSetup  !!!\n");
    return sizeof(g_setupItems)/sizeof(s_SETUP_ITEM);
}

static void obtainListItemData_ListviewSetup(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    if ((index < 0)  || (index >= sizeof(g_setupItems)/sizeof(s_SETUP_ITEM))) {
    	return;
    }
    ZKListView::ZKListSubItem* subItemIcon =  pListItem->findSubItemByID(ID_WASHER_SubItemIcon);
    if (subItemIcon)
    {
        subItemIcon->setText((char)index);
    }
    ZKListView::ZKListSubItem* subItemText =  pListItem->findSubItemByID(ID_WASHER_SubItemText);
    if (subItemText) {
        subItemText->setText(g_setupItems[index].text);
    }
}

static void onListItemClick_ListviewSetup(ZKListView *pListView, int index, int id) {
    if ((index < 0)  || (index >= sizeof(g_setupItems)/sizeof(s_SETUP_ITEM))) {
    	return;
    }
    if (g_setupItems[index].windowId != -1 ) {

    }
    switch (index) {
		case 2:
			EASYUICONTEXT->openActivity("WifiSettingActivity");
            return;
			break;
		case 7:
			EASYUICONTEXT->openActivity("videotutorialActivity");
		default:
			break;
	}

    if (g_setupItems[index].toastTitle != NULL && strlen(g_setupItems[index].toastTitle) > 0) {
    	if (strstr(g_setupItems[index].toastTitle, "photo:") != NULL) {

    	} else {
    		mTextviewTtitlePtr->setText(g_setupItems[index].toastTitle);
    	}
		mWindowDialogPtr->setVisible(true);
    }
}

static bool onButtonClick_BtnNo(ZKButton *pButton) {
    //LOGD(" ButtonClick BtnNo !!!\n");
	mWindowDialogPtr->setVisible(false);
    return true;
}

static bool onButtonClick_BtnOk(ZKButton *pButton) {
    //LOGD(" ButtonClick BtnOk !!!\n");
	mWindowDialogPtr->setVisible(false);
    return true;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
