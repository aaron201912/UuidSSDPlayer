/***********************************************
/gen auto by zuitools
***********************************************/
#include "playerActivity.h"

/*TAG:GlobalVariable全局变量*/
static ZKTextView* mTextView_picPtr;
static ZKButton* mButton_circlemodePtr;
static ZKButton* mButton_nextPtr;
static ZKButton* mButton_prevPtr;
static ZKWindow* mWindow_mediaInfoPtr;
static ZKWindow* mWindow_playBarPtr;
static ZKButton* mButton_confirmPtr;
static ZKTextView* mTextview_msgPtr;
static ZKWindow* mWindow_errMsgPtr;
static ZKTextView* mTextview_volTitlePtr;
static ZKTextView* mTextview_volumePtr;
static ZKTextView* mTextview_fileNamePtr;
static ZKVideoView* mVideoview_videoPtr;
static ZKSeekBar* mSeekbar_volumnPtr;
static ZKTextView* mTextview_slashPtr;
static ZKTextView* mTextview_durationPtr;
static ZKTextView* mTextview_curtimePtr;
static ZKTextView* mTextview_speedPtr;
static ZKTextView* mTextview_videoInfoPtr;
static ZKButton* mButton_voicePtr;
static ZKButton* mButton_fastPtr;
static ZKButton* mButton_slowPtr;
static ZKButton* mButton_stopPtr;
static ZKButton* mButton_playPtr;
static ZKSeekBar* mSeekbar_progressPtr;
static playerActivity* mActivityPtr;

/*register activity*/
REGISTER_ACTIVITY(playerActivity);

typedef struct {
	int id; // 定时器ID ， 不能重复
	int time; // 定时器  时间间隔  单位 毫秒
}S_ACTIVITY_TIMEER;

#include "logic/playerLogic.cc"

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
    ID_PLAYER_Button_circlemode, onButtonClick_Button_circlemode,
    ID_PLAYER_Button_next, onButtonClick_Button_next,
    ID_PLAYER_Button_prev, onButtonClick_Button_prev,
    ID_PLAYER_Button_confirm, onButtonClick_Button_confirm,
    ID_PLAYER_Button_voice, onButtonClick_Button_voice,
    ID_PLAYER_Button_fast, onButtonClick_Button_fast,
    ID_PLAYER_Button_slow, onButtonClick_Button_slow,
    ID_PLAYER_Button_stop, onButtonClick_Button_stop,
    ID_PLAYER_Button_play, onButtonClick_Button_play,
};
/***************/


typedef void (*SeekBarCallback)(ZKSeekBar *pSeekBar, int progress);
typedef void (*SeekBarTrackCallback)(ZKSeekBar *pSeekBar);
typedef struct {
    int id;
    SeekBarCallback callback;
}S_ZKSeekBarCallback;
typedef struct {
	int id;
	SeekBarTrackCallback callback;
}S_ZKSeekBarTrackCallback;

/*TAG:SeekBarCallbackTab*/
static S_ZKSeekBarCallback SZKSeekBarCallbackTab[] = {
    ID_PLAYER_Seekbar_volumn, onProgressChanged_Seekbar_volumn,
    ID_PLAYER_Seekbar_progress, onProgressChanged_Seekbar_progress,
};

// start track touch
static S_ZKSeekBarTrackCallback SZKSeekBarStartTrackCallbackTab[] = {
    ID_PLAYER_Seekbar_progress, onStartTrackingTouch_Seekbar_progress,
};

// stop track touch
static S_ZKSeekBarTrackCallback SZKSeekBarStopTrackCallbackTab[] = {
    ID_PLAYER_Seekbar_progress, onStopTrackingTouch_Seekbar_progress,
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
    ID_PLAYER_Videoview_video, true, 5, NULL,
};


playerActivity::playerActivity() {
	//todo add init code here
	mVideoLoopIndex = 0;
	mVideoLoopErrorCount = 0;
}

playerActivity::~playerActivity() {
  //todo add init file here
  // 退出应用时需要反注册
    EASYUICONTEXT->unregisterGlobalTouchListener(this);
    onUI_quit();
    unregisterProtocolDataUpdateListener(onProtocolDataUpdate);
}

const char* playerActivity::getAppName() const{
	return "player.ftu";
}

//TAG:onCreate
void playerActivity::onCreate() {
	Activity::onCreate();
    mTextView_picPtr = (ZKTextView*)findControlByID(ID_PLAYER_TextView_pic);
    mButton_circlemodePtr = (ZKButton*)findControlByID(ID_PLAYER_Button_circlemode);
    mButton_nextPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_next);
    mButton_prevPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_prev);
    mWindow_mediaInfoPtr = (ZKWindow*)findControlByID(ID_PLAYER_Window_mediaInfo);
    mWindow_playBarPtr = (ZKWindow*)findControlByID(ID_PLAYER_Window_playBar);
    mButton_confirmPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_confirm);
    mTextview_msgPtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_msg);
    mWindow_errMsgPtr = (ZKWindow*)findControlByID(ID_PLAYER_Window_errMsg);
    mTextview_volTitlePtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_volTitle);
    mTextview_volumePtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_volume);
    mTextview_fileNamePtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_fileName);
    mVideoview_videoPtr = (ZKVideoView*)findControlByID(ID_PLAYER_Videoview_video);if(mVideoview_videoPtr!= NULL){mVideoview_videoPtr->setVideoPlayerMessageListener(this);}
    mSeekbar_volumnPtr = (ZKSeekBar*)findControlByID(ID_PLAYER_Seekbar_volumn);if(mSeekbar_volumnPtr!= NULL){mSeekbar_volumnPtr->setSeekBarChangeListener(this);}
    mTextview_slashPtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_slash);
    mTextview_durationPtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_duration);
    mTextview_curtimePtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_curtime);
    mTextview_speedPtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_speed);
    mTextview_videoInfoPtr = (ZKTextView*)findControlByID(ID_PLAYER_Textview_videoInfo);
    mButton_voicePtr = (ZKButton*)findControlByID(ID_PLAYER_Button_voice);
    mButton_fastPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_fast);
    mButton_slowPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_slow);
    mButton_stopPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_stop);
    mButton_playPtr = (ZKButton*)findControlByID(ID_PLAYER_Button_play);
    mSeekbar_progressPtr = (ZKSeekBar*)findControlByID(ID_PLAYER_Seekbar_progress);if(mSeekbar_progressPtr!= NULL){mSeekbar_progressPtr->setSeekBarChangeListener(this);}
	mActivityPtr = this;
	onUI_init();
    registerProtocolDataUpdateListener(onProtocolDataUpdate); 
    rigesterActivityTimer();
}

void playerActivity::onClick(ZKBase *pBase) {
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

void playerActivity::onResume() {
	Activity::onResume();
	EASYUICONTEXT->registerGlobalTouchListener(this);
	startVideoLoopPlayback();
	onUI_show();
}

void playerActivity::onPause() {
	Activity::onPause();
	EASYUICONTEXT->unregisterGlobalTouchListener(this);
	stopVideoLoopPlayback();
	onUI_hide();
}

void playerActivity::onIntent(const Intent *intentPtr) {
	Activity::onIntent(intentPtr);
	onUI_intent(intentPtr);
}

bool playerActivity::onTimer(int id) {
	return onUI_Timer(id);
}

void playerActivity::onProgressChanged(ZKSeekBar *pSeekBar, int progress){

    int seekBarTablen = sizeof(SZKSeekBarCallbackTab) / sizeof(S_ZKSeekBarCallback);
    for (int i = 0; i < seekBarTablen; ++i) {
        if (SZKSeekBarCallbackTab[i].id == pSeekBar->getID()) {
            SZKSeekBarCallbackTab[i].callback(pSeekBar, progress);
            break;
        }
    }
}
void playerActivity::onStartTrackingTouch(ZKSeekBar *pSeekBar){
	printf("start tracking touch\n");
	int seekBarTablen = sizeof(SZKSeekBarStartTrackCallbackTab) / sizeof(SZKSeekBarStartTrackCallbackTab);
	for (int i = 0; i < seekBarTablen; ++i) {
		if (SZKSeekBarStartTrackCallbackTab[i].id == pSeekBar->getID()) {
			SZKSeekBarStartTrackCallbackTab[i].callback(pSeekBar);
			break;
		}
	}
}

void playerActivity::onStopTrackingTouch(ZKSeekBar *pSeekBar){
	printf("stop tracking touch\n");
	int seekBarTablen = sizeof(SZKSeekBarStopTrackCallbackTab) / sizeof(SZKSeekBarStopTrackCallbackTab);
	for (int i = 0; i < seekBarTablen; ++i) {
		if (SZKSeekBarStopTrackCallbackTab[i].id == pSeekBar->getID()) {
			SZKSeekBarStopTrackCallbackTab[i].callback(pSeekBar);
			break;
		}
	}
}


int playerActivity::getListItemCount(const ZKListView *pListView) const{
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            return SListViewFunctionsCallbackTab[i].getListItemCountCallback(pListView);
            break;
        }
    }
    return 0;
}

void playerActivity::obtainListItemData(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index){
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            SListViewFunctionsCallbackTab[i].obtainListItemDataCallback(pListView, pListItem, index);
            break;
        }
    }
}

void playerActivity::onItemClick(ZKListView *pListView, int index, int id){
    int tablen = sizeof(SListViewFunctionsCallbackTab) / sizeof(S_ListViewFunctionsCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SListViewFunctionsCallbackTab[i].id == pListView->getID()) {
            SListViewFunctionsCallbackTab[i].onItemClickCallback(pListView, index, id);
            break;
        }
    }
}

void playerActivity::onSlideItemClick(ZKSlideWindow *pSlideWindow, int index) {
    int tablen = sizeof(SSlideWindowItemClickCallbackTab) / sizeof(S_SlideWindowItemClickCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SSlideWindowItemClickCallbackTab[i].id == pSlideWindow->getID()) {
            SSlideWindowItemClickCallbackTab[i].onSlideItemClickCallback(pSlideWindow, index);
            break;
        }
    }
}

bool playerActivity::onTouchEvent(const MotionEvent &ev) {
    return onplayerActivityTouchEvent(ev);
}

void playerActivity::onTextChanged(ZKTextView *pTextView, const std::string &text) {
    int tablen = sizeof(SEditTextInputCallbackTab) / sizeof(S_EditTextInputCallback);
    for (int i = 0; i < tablen; ++i) {
        if (SEditTextInputCallbackTab[i].id == pTextView->getID()) {
            SEditTextInputCallbackTab[i].onEditTextChangedCallback(text);
            break;
        }
    }
}

void playerActivity::rigesterActivityTimer() {
    int tablen = sizeof(REGISTER_ACTIVITY_TIMER_TAB) / sizeof(S_ACTIVITY_TIMEER);
    for (int i = 0; i < tablen; ++i) {
        S_ACTIVITY_TIMEER temp = REGISTER_ACTIVITY_TIMER_TAB[i];
        registerTimer(temp.id, temp.time);
    }
}


void playerActivity::onVideoPlayerMessage(ZKVideoView *pVideoView, int msg) {
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

void playerActivity::videoLoopPlayback(ZKVideoView *pVideoView, int msg, size_t callbackTabIndex) {

	switch (msg) {
	case ZKVideoView::E_MSGTYPE_VIDEO_PLAY_STARTED:
		LOGD("ZKVideoView::E_MSGTYPE_VIDEO_PLAY_STARTED\n");
    if (callbackTabIndex >= (sizeof(SVideoViewCallbackTab)/sizeof(S_VideoViewCallback))) {
      break;
    }
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

void playerActivity::startVideoLoopPlayback() {
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

void playerActivity::stopVideoLoopPlayback() {
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

bool playerActivity::parseVideoFileList(const char *pFileListPath, std::vector<string>& mediaFileList) {
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

int playerActivity::removeCharFromString(string& nString, char c) {
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

void playerActivity::registerUserTimer(int id, int time) {
	registerTimer(id, time);
}

void playerActivity::unregisterUserTimer(int id) {
	unregisterTimer(id);
}

void playerActivity::resetUserTimer(int id, int time) {
	resetTimer(id, time);
}