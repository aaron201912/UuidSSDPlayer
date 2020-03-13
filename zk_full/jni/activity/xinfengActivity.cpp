/***********************************************
/gen auto by zuitools
***********************************************/
#include "xinfengActivity.h"

/*TAG:GlobalVariable全局变量*/
static ZKButton* mButtonK4Ptr;
static ZKButton* mButtonkey1Ptr;
static ZKButton* mButtonKClearPtr;
static ZKButton* mButtonK3Ptr;
static ZKButton* mButtonKCancelPtr;
static ZKButton* mButtonK2Ptr;
static ZKButton* mButtonKEnterPtr;
static ZKButton* mButtonK9Ptr;
static ZKButton* mButtonK0Ptr;
static ZKButton* mButtonK6Ptr;
static ZKButton* mButtonK7Ptr;
static ZKButton* mButtonK5Ptr;
static ZKButton* mButtonK8Ptr;
static ZKTextView* mTextPassPtr;
static ZKWindow* mWindowKeyPadPtr;
static ZKTextView* mText5Ptr;
static ZKTextView* mText3Ptr;
static ZKButton* mButton1YearPtr;
static ZKButton* mButton6MonthPtr;
static ZKButton* mButton3MonthPtr;
static ZKButton* mButton1MonthPtr;
static ZKTextView* mText1Ptr;
static ZKWindow* mWindowPayPtr;
static ZKTextView* mTextFanNamePtr;
static ZKTextView* mTextFanSpeedPtr;
static ZKSeekBar* mSeekBarFanSpeedPtr;
static ZKWindow* mwindow_fanspeedPtr;
static ZKTextView* mTextTemptureinPtr;
static ZKTextView* mText10Ptr;
static ZKTextView* mText8Ptr;
static ZKWindow* mWindow3Ptr;
static ZKTextView* mTextTemptureoutPtr;
static ZKTextView* mText9Ptr;
static ZKTextView* mText7Ptr;
static ZKWindow* mWindow2Ptr;
static ZKTextView* mTextCO2StatusPtr;
static ZKButton* mButtonCO2StatusPtr;
static ZKTextView* mText4Ptr;
static ZKTextView* mTextCO2ValuePtr;
static ZKWindow* mWindow1Ptr;
static ZKTextView* mTextPMstatusPtr;
static ZKTextView* mText2Ptr;
static ZKButton* mBtnPMstatusPtr;
static ZKTextView* mTextPMValuePtr;
static ZKWindow* mView_2Ptr;
static ZKButton* mbtn_setupPtr;
static ZKButton* mbtn_external_windPtr;
static ZKButton* mbtn_internal_windPtr;
static ZKButton* mbtn_powerPtr;
static ZKButton* mbtn_heatPtr;
static ZKButton* mbtn_degermingPtr;
static ZKButton* mbtn_autoPtr;
static ZKWindow* mView_1Ptr;
static ZKTextView* mTextWiFiPtr;
static ZKTextView* mTextWeekPtr;
static ZKTextView* mTextDatePtr;
static ZKTextView* mTextTimePtr;
static ZKButton* mButtonPayPtr;
static ZKTextView* mTextADPtr;
static ZKWindow* mWindow4Ptr;
static xinfengActivity* mActivityPtr;

/*register activity*/
REGISTER_ACTIVITY(xinfengActivity);

typedef struct {
	int id; // 定时器ID ， 不能重复
	int time; // 定时器  时间间隔  单位 毫秒
}S_ACTIVITY_TIMEER;

#include "logic/xinfengLogic.cc"

/***********/
typedef struct {
    int id;
    const char *pApp;
} SAppInfo;

/**
 *点击跳转window
 */
static SAppInfo sAppInfoTab[] = {
//  { ID_MAIN_TEXT, "TextViewActivity" },
};

/***************/
typedef bool (*ButtonCallback)(ZKButton *pButton);
/**
 * button onclick表
 */
typedef struct {
    int id;
    ButtonCallback callback;
}S_ButtonCallback;

/*TAG:ButtonCallbackTab按键映射表*/
static S_ButtonCallback sButtonCallbackTab[] = {
    ID_XINFENG_ButtonK4, onButtonClick_ButtonK4,
    ID_XINFENG_Buttonkey1, onButtonClick_Buttonkey1,
    ID_XINFENG_ButtonKClear, onButtonClick_ButtonKClear,
    ID_XINFENG_ButtonK3, onButtonClick_ButtonK3,
    ID_XINFENG_ButtonKCancel, onButtonClick_ButtonKCancel,
    ID_XINFENG_ButtonK2, onButtonClick_ButtonK2,
    ID_XINFENG_ButtonKEnter, onButtonClick_ButtonKEnter,
    ID_XINFENG_ButtonK9, onButtonClick_ButtonK9,
    ID_XINFENG_ButtonK0, onButtonClick_ButtonK0,
    ID_XINFENG_ButtonK6, onButtonClick_ButtonK6,
    ID_XINFENG_ButtonK7, onButtonClick_ButtonK7,
    ID_XINFENG_ButtonK5, onButtonClick_ButtonK5,
    ID_XINFENG_ButtonK8, onButtonClick_ButtonK8,
    ID_XINFENG_Button1Year, onButtonClick_Button1Year,
    ID_XINFENG_Button6Month, onButtonClick_Button6Month,
    ID_XINFENG_Button3Month, onButtonClick_Button3Month,
    ID_XINFENG_Button1Month, onButtonClick_Button1Month,
    ID_XINFENG_ButtonCO2Status, onButtonClick_ButtonCO2Status,
    ID_XINFENG_BtnPMstatus, onButtonClick_BtnPMstatus,
    ID_XINFENG_btn_setup, onButtonClick_btn_setup,
    ID_XINFENG_btn_external_wind, onButtonClick_btn_external_wind,
    ID_XINFENG_btn_internal_wind, onButtonClick_btn_internal_wind,
    ID_XINFENG_btn_power, onButtonClick_btn_power,
    ID_XINFENG_btn_heat, onButtonClick_btn_heat,
    ID_XINFENG_btn_degerming, onButtonClick_btn_degerming,
    ID_XINFENG_btn_auto, onButtonClick_btn_auto,
    ID_XINFENG_ButtonPay, onButtonClick_ButtonPay,
};
/***************/


typedef void (*SeekBarCallback)(ZKSeekBar *pSeekBar, int progress);
typedef struct {
    int id;
    SeekBarCallback callback;
}S_ZKSeekBarCallback;
/*TAG:SeekBarCallbackTab*/
static S_ZKSeekBarCallback SZKSeekBarCallbackTab[] = {
    ID_XINFENG_SeekBarFanSpeed, onProgressChanged_SeekBarFanSpeed,
};


typedef int (*ListViewGetItemCountCallback)(const ZKListView *pListView);
typedef void (*ListViewobtainListItemDataCallback)(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index);
typedef void (*ListViewonItemClickCallback)(ZKListView *pListView, int index, int id);
typedef struct {
    int id;
    ListViewGetItemCountCallback getListItemCountCallback;
    ListViewobtainListItemDataCallback obtainListItemDataCallback;
    ListViewonItemClickCallback onItemClickCallback;
}S_ListViewFunctionsCallback;
/*TAG:ListViewFunctionsCallback*/
static S_ListViewFunctionsCallback SListViewFunctionsCallbackTab[] = {
};


typedef void (*SlideWindowItemClickCallback)(ZKSlideWindow *pSlideWindow, int index);
typedef struct {
    int id;
    SlideWindowItemClickCallback onSlideItemClickCallback;
}S_SlideWindowItemClickCallback;
/*TAG:SlideWindowFunctionsCallbackTab*/
static S_SlideWindowItemClickCallback SSlideWindowItemClickCallbackTab[] = {
};


typedef void (*EditTextInputCallback)(const std::string &text);
typedef struct {
    int id;
    EditTextInputCallback onEditTextChangedCallback;
}S_EditTextInputCallback;
/*TAG:EditTextInputCallback*/
static S_EditTextInputCallback SEditTextInputCallbackTab[] = {
};

typedef void (*VideoViewCallback)(ZKVideoView *pVideoView, int msg);
typedef struct {
    int id; //VideoView ID
    bool loop; // 是否是轮播类型
    int defaultvolume;//轮播类型时,默认视频音量
    VideoViewCallback onVideoViewCallback;
}S_VideoViewCallback;
/*TAG:VideoViewCallback*/
static S_VideoViewCallback SVideoViewCallbackTab[] = {
};


xinfengActivity::xinfengActivity() {
	//todo add init code here
	mVideoLoopIndex = 0;
	mVideoLoopErrorCount = 0;
}

xinfengActivity::~xinfengActivity() {
	//todo add init file here
    // 退出应用时需要反注册
    EASYUICONTEXT->unregisterGlobalTouchListener(this);
    onUI_quit();
    unregisterProtocolDataUpdateListener(onProtocolDataUpdate);
}

const char* xinfengActivity::getAppName() const{
	return "xinfeng.ftu";
}

//TAG:onCreate
void xinfengActivity::onCreate() {
	Activity::onCreate();
    mButtonK4Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK4);
    mButtonkey1Ptr = (ZKButton*)findControlByID(ID_XINFENG_Buttonkey1);
    mButtonKClearPtr = (ZKButton*)findControlByID(ID_XINFENG_ButtonKClear);
    mButtonK3Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK3);
    mButtonKCancelPtr = (ZKButton*)findControlByID(ID_XINFENG_ButtonKCancel);
    mButtonK2Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK2);
    mButtonKEnterPtr = (ZKButton*)findControlByID(ID_XINFENG_ButtonKEnter);
    mButtonK9Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK9);
    mButtonK0Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK0);
    mButtonK6Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK6);
    mButtonK7Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK7);
    mButtonK5Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK5);
    mButtonK8Ptr = (ZKButton*)findControlByID(ID_XINFENG_ButtonK8);
    mTextPassPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextPass);
    mWindowKeyPadPtr = (ZKWindow*)findControlByID(ID_XINFENG_WindowKeyPad);
    mText5Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text5);
    mText3Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text3);
    mButton1YearPtr = (ZKButton*)findControlByID(ID_XINFENG_Button1Year);
    mButton6MonthPtr = (ZKButton*)findControlByID(ID_XINFENG_Button6Month);
    mButton3MonthPtr = (ZKButton*)findControlByID(ID_XINFENG_Button3Month);
    mButton1MonthPtr = (ZKButton*)findControlByID(ID_XINFENG_Button1Month);
    mText1Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text1);
    mWindowPayPtr = (ZKWindow*)findControlByID(ID_XINFENG_WindowPay);
    mTextFanNamePtr = (ZKTextView*)findControlByID(ID_XINFENG_TextFanName);
    mTextFanSpeedPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextFanSpeed);
    mSeekBarFanSpeedPtr = (ZKSeekBar*)findControlByID(ID_XINFENG_SeekBarFanSpeed);if(mSeekBarFanSpeedPtr!= NULL){mSeekBarFanSpeedPtr->setSeekBarChangeListener(this);}
    mwindow_fanspeedPtr = (ZKWindow*)findControlByID(ID_XINFENG_window_fanspeed);
    mTextTemptureinPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextTempturein);
    mText10Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text10);
    mText8Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text8);
    mWindow3Ptr = (ZKWindow*)findControlByID(ID_XINFENG_Window3);
    mTextTemptureoutPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextTemptureout);
    mText9Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text9);
    mText7Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text7);
    mWindow2Ptr = (ZKWindow*)findControlByID(ID_XINFENG_Window2);
    mTextCO2StatusPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextCO2Status);
    mButtonCO2StatusPtr = (ZKButton*)findControlByID(ID_XINFENG_ButtonCO2Status);
    mText4Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text4);
    mTextCO2ValuePtr = (ZKTextView*)findControlByID(ID_XINFENG_TextCO2Value);
    mWindow1Ptr = (ZKWindow*)findControlByID(ID_XINFENG_Window1);
    mTextPMstatusPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextPMstatus);
    mText2Ptr = (ZKTextView*)findControlByID(ID_XINFENG_Text2);
    mBtnPMstatusPtr = (ZKButton*)findControlByID(ID_XINFENG_BtnPMstatus);
    mTextPMValuePtr = (ZKTextView*)findControlByID(ID_XINFENG_TextPMValue);
    mView_2Ptr = (ZKWindow*)findControlByID(ID_XINFENG_View_2);
    mbtn_setupPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_setup);
    mbtn_external_windPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_external_wind);
    mbtn_internal_windPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_internal_wind);
    mbtn_powerPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_power);
    mbtn_heatPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_heat);
    mbtn_degermingPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_degerming);
    mbtn_autoPtr = (ZKButton*)findControlByID(ID_XINFENG_btn_auto);
    mView_1Ptr = (ZKWindow*)findControlByID(ID_XINFENG_View_1);
    mTextWiFiPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextWiFi);
    mTextWeekPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextWeek);
    mTextDatePtr = (ZKTextView*)findControlByID(ID_XINFENG_TextDate);
    mTextTimePtr = (ZKTextView*)findControlByID(ID_XINFENG_TextTime);
    mButtonPayPtr = (ZKButton*)findControlByID(ID_XINFENG_ButtonPay);
    mTextADPtr = (ZKTextView*)findControlByID(ID_XINFENG_TextAD);
    mWindow4Ptr = (ZKWindow*)findControlByID(ID_XINFENG_Window4);
	mActivityPtr = this;
	onUI_init();
    registerProtocolDataUpdateListener(onProtocolDataUpdate); 
    rigesterActivityTimer();
}

void xinfengActivity::onClick(ZKBase *pBase) {
	//TODO: add widget onClik code 
    int buttonTablen = sizeof(sButtonCallbackTab) / sizeof(S_ButtonCallback);
    for (int i = 0; i < buttonTablen; ++i) {
        if (sButtonCallbackTab[i].id == pBase->getID()) {
            if (sButtonCallbackTab[i].callback((ZKButton*)pBase)) {
            	return;
            }
            break;
        }
    }


    int len = sizeof(sAppInfoTab) / sizeof(sAppInfoTab[0]);
    for (int i = 0; i < len; ++i) {
        if (sAppInfoTab[i].id == pBase->getID()) {
            EASYUICONTEXT->openActivity(sAppInfoTab[i].pApp);
            return;
        }
    }

	Activity::onClick(pBase);
}

void xinfengActivity::onResume() {
	Activity::onResume();
	EASYUICONTEXT->registerGlobalTouchListener(this);
	startVideoLoopPlayback();
//	onUI_show();
}

void xinfengActivity::onPause() {
	Activity::onPause();
	EASYUICONTEXT->unregisterGlobalTouchListener(this);
	stopVideoLoopPlayback();
//	onUI_hide();
}

void xinfengActivity::onIntent(const Intent *intentPtr) {
	Activity::onIntent(intentPtr);
//	onUI_intent(intentPtr);
}

bool xinfengActivity::onTimer(int id) {
	return onUI_Timer(id);
}

void xinfengActivity::onProgressChanged(ZKSeekBar *pSeekBar, int progress){

    int seekBarTablen = sizeof(SZKSeekBarCallbackTab) / sizeof(S_ZKSeekBarCallback);
    for (int i = 0; i < seekBarTablen; ++i) {
        if (SZKSeekBarCallbackTab[i].id == pSeekBar->getID()) {
            SZKSeekBarCallbackTab[i].callback(pSeekBar, progress);
            break;
        }
    }
}

int xinfengActivity::getListItemCount(const ZKListView *pListView) const{
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            return SListViewFunctionsCallbackTab[i].getListItemCountCallback(pListView);
            break;
        }
    }
    return 0;
}

void xinfengActivity::obtainListItemData(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index){
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            SListViewFunctionsCallbackTab[i].obtainListItemDataCallback(pListView, pListItem, index);
            break;
        }
    }
}

void xinfengActivity::onItemClick(ZKListView *pListView, int index, int id){
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            SListViewFunctionsCallbackTab[i].onItemClickCallback(pListView, index, id);
            break;
        }
    }
}

void xinfengActivity::onSlideItemClick(ZKSlideWindow *pSlideWindow, int index) {
    int tablen = sizeof(SSlideWindowItemClickCallbackTab) / sizeof(S_SlideWindowItemClickCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SSlideWindowItemClickCallbackTab[i].id == pSlideWindow->getID()) {
            SSlideWindowItemClickCallbackTab[i].onSlideItemClickCallback(pSlideWindow, index);
            break;
        }
    }
}

bool xinfengActivity::onTouchEvent(const MotionEvent &ev) {
    return onxinfengActivityTouchEvent(ev);
}

void xinfengActivity::onTextChanged(ZKTextView *pTextView, const std::string &text) {
    int tablen = sizeof(SEditTextInputCallbackTab) / sizeof(S_EditTextInputCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SEditTextInputCallbackTab[i].id == pTextView->getID()) {
            SEditTextInputCallbackTab[i].onEditTextChangedCallback(text);
            break;
        }
    }
}

void xinfengActivity::rigesterActivityTimer() {
    int tablen = sizeof(REGISTER_ACTIVITY_TIMER_TAB) / sizeof(S_ACTIVITY_TIMEER);
    for (int i = 0; i < tablen; ++i) {
        S_ACTIVITY_TIMEER temp = REGISTER_ACTIVITY_TIMER_TAB[i];
        registerTimer(temp.id, temp.time);
    }
}


void xinfengActivity::onVideoPlayerMessage(ZKVideoView *pVideoView, int msg) {
    int tablen = sizeof(SVideoViewCallbackTab) / sizeof(S_VideoViewCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SVideoViewCallbackTab[i].id == pVideoView->getID()) {
        	if (SVideoViewCallbackTab[i].loop) {
                //循环播放
        		videoLoopPlayback(pVideoView, msg, i);
        	} else if (SVideoViewCallbackTab[i].onVideoViewCallback != NULL){
        	    SVideoViewCallbackTab[i].onVideoViewCallback(pVideoView, msg);
        	}
            break;
        }
    }
}

void xinfengActivity::videoLoopPlayback(ZKVideoView *pVideoView, int msg, int callbackTabIndex) {

	switch (msg) {
	case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_STARTED:
		LOGD("ZKVideoView::E_MSGTYPE_VIDEO_PLAY_STARTED\n");
		pVideoView->setVolume(SVideoViewCallbackTab[callbackTabIndex].defaultvolume / 10.0);
		mVideoLoopErrorCount = 0;
		break;
	case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_ERROR:
		/**错误处理 */
		++mVideoLoopErrorCount;
		if (mVideoLoopErrorCount > 100) {
			LOGD("video loop error counts > 100, quit loop playback !");
            break;
		} //不用break, 继续尝试播放下一个
	case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_COMPLETED:
		LOGD("ZKVideoView::E_MSGTYPE_VIDEO_PLAY_COMPLETED\n");
        std::vector<std::string> videolist;
        std::string fileName(getAppName());
        if (fileName.size() < 4) {
             LOGD("getAppName size < 4, ignore!");
             break;
        }
        fileName = fileName.substr(0, fileName.length() - 4) + "_video_list.txt";
        fileName = "/mnt/extsd/" + fileName;
        if (!parseVideoFileList(fileName.c_str(), videolist)) {
            LOGD("parseVideoFileList failed !");
		    break;
        }
		if (pVideoView && !videolist.empty()) {
			mVideoLoopIndex = (mVideoLoopIndex + 1) % videolist.size();
			pVideoView->play(videolist[mVideoLoopIndex].c_str());
		}
		break;
	}
}

void xinfengActivity::startVideoLoopPlayback() {
    int tablen = sizeof(SVideoViewCallbackTab) / sizeof(S_VideoViewCallback);
    for (int i = 0; i < tablen; ++i) {
    	if (SVideoViewCallbackTab[i].loop) {
    		ZKVideoView* videoView = (ZKVideoView*)findControlByID(SVideoViewCallbackTab[i].id);
    		if (!videoView) {
    			return;
    		}
    		//循环播放
    		videoLoopPlayback(videoView, ZKVideoView::E_MSGTYPE_VIDEO_PLAY_COMPLETED, i);
    		return;
    	}
    }
}

void xinfengActivity::stopVideoLoopPlayback() {
    int tablen = sizeof(SVideoViewCallbackTab) / sizeof(S_VideoViewCallback);
    for (int i = 0; i < tablen; ++i) {
    	if (SVideoViewCallbackTab[i].loop) {
    		ZKVideoView* videoView = (ZKVideoView*)findControlByID(SVideoViewCallbackTab[i].id);
    		if (!videoView) {
    			return;
    		}
    		if (videoView->isPlaying()) {
    		    videoView->stop();
    		}
    		return;
    	}
    }
}

bool xinfengActivity::parseVideoFileList(const char *pFileListPath, std::vector<string>& mediaFileList) {
	mediaFileList.clear();
	if (NULL == pFileListPath || 0 == strlen(pFileListPath)) {
        LOGD("video file list is null!");
		return false;
	}

	ifstream is(pFileListPath, ios_base::in);
	if (!is.is_open()) {
		LOGD("cann't open file %s \n", pFileListPath);
		return false;
	}
	char tmp[1024] = {0};
	while (is.getline(tmp, sizeof(tmp))) {
		string str = tmp;
		removeCharFromString(str, '\"');
		removeCharFromString(str, '\r');
		removeCharFromString(str, '\n');
		if (str.size() > 1) {
     		mediaFileList.push_back(str.c_str());
		}
	}
	LOGD("(f:%s, l:%d) parse fileList[%s], get [%d]files\n", __FUNCTION__,
			__LINE__, pFileListPath, mediaFileList.size());
	for (size_t i = 0; i < mediaFileList.size(); i++) {
		LOGD("file[%d]:[%s]\n", i, mediaFileList[i].c_str());
	}
	is.close();

	return true;
}

int xinfengActivity::removeCharFromString(string& nString, char c) {
    string::size_type   pos;
    while(1) {
        pos = nString.find(c);
        if(pos != string::npos) {
            nString.erase(pos, 1);
        } else {
            break;
        }
    }
    return (int)nString.size();
}

void xinfengActivity::registerUserTimer(int id, int time) {
	registerTimer(id, time);
}

void xinfengActivity::unregisterUserTimer(int id) {
	unregisterTimer(id);
}

void xinfengActivity::resetUserTimer(int id, int time) {
	resetTimer(id, time);
}
