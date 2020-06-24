#pragma once

#include "manager/LanguageManager.h"
typedef struct {
    string typeName;
    string timeUnit;
    int timeNumber;
}s_WASH_TIME_TYPE;
s_WASH_TIME_TYPE g_washProcessTime[] = {
    "wash5", "min", 20,
    "wash6", "min", 21,
    "wash7", "min", 10,
    "wash8", "", 12,
    "temperature", "℃", 2,
    "rotating_speed", "r/min", 1000,
};

typedef struct {
    int index;
    string title;
    string description;
    string background;
}s_PAGE_INFO;

s_PAGE_INFO const g_Pages[] = {
    0, 	"wash9", "wash21", "detail/background/1.png",
    1, 	"wash10", "wash22", "detail/background/2.png",
    2, 	"wash11", "wash23", "detail/background/3.png",
    3, 	"wash12", "wash24", "detail/background/4.png",
    4, 	"wash13", "wash25", "detail/background/5.png",
    5, 	"wash14", "wash26", "detail/background/6.png",

    6, 	"wash15", "wash27", "detail/background/7.png",
    7, 	"wash16", "wash28", "detail/background/8.png",
    8, 	"wash17", "wash29", "detail/background/9.png",
    9, 	"wash18", "wash30", "detail/background/a.png",
    10, "wash19", "wash31", "detail/background/b.png" ,
    11, "wash20", "wash32", "detail/background/c.png" ,

    12, "洗涤+烘干60‘", "洗涤及烘干程序60分钟", "detail/background/d.png" ,
    13, "智能洗衣专家", "全自动洗衣程序", "detail/background/e.png" ,
    14, "单脱水", "单脱水程序", "detail/background/f.png" ,
    15, "节能", "节能程序洗涤", "detail/background/g.png" ,
    16, "快洗15’", "快速洗涤程序15分钟", "detail/background/h.png" ,
    17, "羊毛", "羊毛类衣物", "detail/background/i.png" ,

};

int g_curPageIndex = -1;

const int TIMER_WASHER_PROGRESS = 100;
typedef struct {
    bool isRun;
    int progress;
    const int timerId;
}s_RUN_STRATE;

volatile s_RUN_STRATE g_washerState = {0, 0, TIMER_WASHER_PROGRESS};
static void startWash();
const int STOP_BY_USER = 1;
const int STOP_BY_WASH_OVER = 2;
static void stopWash(int stopBy);

typedef struct {
    int listItemOffset;
    int listItemIndex;
}s_TIME_POPUP;
volatile s_TIME_POPUP g_timePopup = {0};

typedef struct {
    int x;
    int y;
}s_TOUCH_POINT;
s_TOUCH_POINT g_touchPoint;

static void onActivityItent(const Intent *intentPtr) {
	int target = atoi(intentPtr->getExtra("target").c_str());
    for (int i = 0; i < sizeof(g_Pages) / sizeof(s_PAGE_INFO); ++i) {
        if (i == target) {
            g_curPageIndex = i;
            mTextTitlePtr->setText(LANGUAGEMANAGER->getValue(g_Pages[g_curPageIndex].title.c_str()));
            mTextDescriptionPtr->setText(LANGUAGEMANAGER->getValue(g_Pages[g_curPageIndex].description.c_str()));
            mTextviewBackgroundPtr->setBackgroundPic((/*"/mnt/extsd/ui/" +*/ g_Pages[g_curPageIndex].background).c_str());
            break;
        }
    }
}

/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
//	{g_washerState.timerId,  10},
};

static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
	if (!g_washerState.isRun) {
		mSeekbarProgressPtr->setVisible(false);
	}
}

static void onUI_quit() {

}


static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
    switch (id) {
		case TIMER_WASHER_PROGRESS:
		    if (g_washerState.isRun)
                mSeekbarProgressPtr->setProgress(g_washerState.progress += 1);

		    if (g_washerState.progress >= mSeekbarProgressPtr->getMax()) {
                stopWash(STOP_BY_WASH_OVER);
                return false;
		    }
			break;
		default:
			break;
	}

    return true;
}

/**
 * 根据触摸坐标判点击的列表位置
 */
static int getClickedItemOffsetFromEvent(int x, int y) {
	if ( x <= 173) {
		return 0;
	} else if (x <= 350) {
        return 1;
	} else {
		return 2;
	}
		//175  350 525
}


static bool ondetailActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上
	g_touchPoint.x = ev.mX;
	g_touchPoint.y = ev.mY;

	if (mWindowPopupPtr->isVisible()) {

/*		float x = ev.mX - (86  + g_timePopup.listItemOffset * 173);
		float y = ev.mY - 292;
        float jungle = atan2f(y, x);
        float j = jungle * -57.2 ;
        if (j < 0) {
        	j = (abs(j) + 90);
        } else
        {
            j = 180 - j + 270;
        }
        int percent = (int)j % 360 / 6;
        mCirclebarPtr->setProgress(percent);*/
	}

    return false;
}

static int getListItemCount_ListviewMenu(const ZKListView *pListView) {
    //LOGD(" getListItemCount_ ListviewMenu  !!!\n");
    return sizeof(g_washProcessTime) / sizeof(s_WASH_TIME_TYPE);
}

static void obtainListItemData_ListviewMenu(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListviewMenu  !!!\n");
	if (index < 0 || index >= sizeof(g_washProcessTime)/sizeof(s_WASH_TIME_TYPE)) {
        return;
   	}
    ZKListView::ZKListSubItem* subItem =  pListItem->findSubItemByID(ID_DETAIL_SubItemTimeType);
    if (subItem) {
    	subItem->setText(LANGUAGEMANAGER->getValue(g_washProcessTime[index].typeName.c_str()));
    	subItem->setInvalid(g_washerState.isRun ? true : false);
    }
    subItem =  pListItem->findSubItemByID(ID_DETAIL_SubItemTimeUnit);
    if (subItem) {
    	subItem->setText(g_washProcessTime[index].timeUnit);
    	subItem->setInvalid(g_washerState.isRun ? true : false);
    }
    subItem =  pListItem->findSubItemByID(ID_DETAIL_SubItemTimeNumber);
    if (subItem) {
    	subItem->setText(g_washProcessTime[index].timeNumber);
    	subItem->setInvalid(g_washerState.isRun ? true : false);
    }
}

static void onListItemClick_ListviewMenu(ZKListView *pListView, int index, int id) {
	g_timePopup.listItemIndex = index;
//	mCirclebarPtr->setProgress(g_washProcessTime[g_timePopup.listItemIndex].timeNumber);
	mListviewTimePickerPtr->setSelection(g_washProcessTime[g_timePopup.listItemIndex].timeNumber - 1);
	g_timePopup.listItemOffset = getClickedItemOffsetFromEvent(g_touchPoint.x, g_touchPoint.y);

	mWindowPopupPtr->setPosition(LayoutPosition(175 * g_timePopup.listItemOffset - 1, mWindowPopupPtr->getPosition().mTop,
			mWindowPopupPtr->getPosition().mWidth, mWindowPopupPtr->getPosition().mHeight));
	mWindowPopupPtr->showWnd();
	mWindowPopupPtr->setVisible(true);
}

static void startWash() {
	g_washerState.isRun = true;
	g_washerState.progress = 0;
	mSeekbarProgressPtr->setVisible(true);
	mbtnStartPtr->setSelected(true);
	mbtnStartPtr->setTextTr("stop");
	mListviewMenuPtr->setTouchable(false);
	mListviewMenuPtr->refreshListView();
	mActivityPtr->registerUserTimer(g_washerState.timerId, 10);
}

static void stopWash(int stopBy) {
	g_washerState.isRun = false;
	g_washerState.progress = 0;
	mSeekbarProgressPtr->setVisible(false);
	mbtnStartPtr->setSelected(false);
	mbtnStartPtr->setTextTr("start");
	if (STOP_BY_USER == stopBy) {
	    mActivityPtr->unregisterUserTimer(g_washerState.timerId);
	}
	mListviewMenuPtr->setTouchable(true);
	mListviewMenuPtr->refreshListView();
}

static bool onButtonClick_btnStart(ZKButton *pButton) {
    //LOGD(" ButtonClick btnStart !!!\n");
	if (g_washerState.isRun) {
		stopWash(STOP_BY_USER);
	} else {
		startWash();
	}
    return true;
}

static void onProgressChanged_SeekbarProgress(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged SeekbarProgress %d !!!\n", progress);
}

static bool onButtonClick_btnMenuMore(ZKButton *pButton) {
    //LOGD(" ButtonClick btnMenuMore !!!\n");
    return true;
}

static bool onButtonClick_btnNext(ZKButton *pButton) {
    //LOGD(" ButtonClick btnNext !!!\n");
	mListviewMenuPtr->setSelection(5);
    return true;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}

static int getListItemCount_ListviewTimePicker(const ZKListView *pListView) {
    //LOGD(" getListItemCount_ ListviewTimePicker  !!!\n");
    return 60;
}

static void obtainListItemData_ListviewTimePicker(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListviewTimePicker  !!!\n");
	pListItem->setText(index + 1);
}

static void onListItemClick_ListviewTimePicker(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ ListviewTimePicker  !!!\n");
    g_washProcessTime[g_timePopup.listItemIndex].timeNumber = index + 1;
    mListviewMenuPtr->refreshListView();
}

