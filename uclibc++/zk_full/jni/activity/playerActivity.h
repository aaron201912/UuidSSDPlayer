/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __PLAYERACTIVITY_H__
#define __PLAYERACTIVITY_H__


#include "app/Activity.h"
#include "entry/EasyUIContext.h"

#include "uart/ProtocolData.h"
#include "uart/ProtocolParser.h"

#include "utils/Log.h"
#include "control/ZKDigitalClock.h"
#include "control/ZKButton.h"
#include "control/ZKCircleBar.h"
#include "control/ZKDiagram.h"
#include "control/ZKListView.h"
#include "control/ZKPointer.h"
#include "control/ZKQRCode.h"
#include "control/ZKTextView.h"
#include "control/ZKSeekBar.h"
#include "control/ZKEditText.h"
#include "control/ZKVideoView.h"
#include "window/ZKSlideWindow.h"

/*TAG:Macro宏ID*/
#define ID_PLAYER_TextView_pic    50001
#define ID_PLAYER_Button_circlemode    20009
#define ID_PLAYER_Button_next    20008
#define ID_PLAYER_Button_prev    20007
#define ID_PLAYER_Window_mediaInfo    110003
#define ID_PLAYER_Window_playBar    110002
#define ID_PLAYER_Button_confirm    20006
#define ID_PLAYER_Textview_msg    50010
#define ID_PLAYER_Window_errMsg    110001
#define ID_PLAYER_Textview_volTitle    50009
#define ID_PLAYER_Textview_volume    50008
#define ID_PLAYER_Textview_fileName    50007
#define ID_PLAYER_Videoview_video    95001
#define ID_PLAYER_Seekbar_volumn    91002
#define ID_PLAYER_Textview_slash    50005
#define ID_PLAYER_Textview_duration    50004
#define ID_PLAYER_Textview_curtime    50003
#define ID_PLAYER_Textview_speed    50002
#define ID_PLAYER_Textview_videoInfo    50006
#define ID_PLAYER_Button_voice    20005
#define ID_PLAYER_Button_fast    20004
#define ID_PLAYER_Button_slow    20003
#define ID_PLAYER_Button_stop    20002
#define ID_PLAYER_Button_play    20001
#define ID_PLAYER_Seekbar_progress    91001
/*TAG:Macro宏ID END*/

class playerActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener,
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:

    playerActivity();
    virtual ~playerActivity();
	void onStartTrackingTouch(ZKSeekBar *pSeekBar);
	void onStopTrackingTouch(ZKSeekBar *pSeekBar);
    /**
     * 注册定时器
     */
	void registerUserTimer(int id, int time);
	/**
	 * 取消定时器
	 */
	void unregisterUserTimer(int id);
	/**
	 * 重置定时器
	 */
	void resetUserTimer(int id, int time);

protected:
    /*TAG:PROTECTED_FUNCTION*/
    virtual const char* getAppName() const;
    virtual void onCreate();
    virtual void onClick(ZKBase *pBase);
    virtual void onResume();
    virtual void onPause();
    virtual void onIntent(const Intent *intentPtr);
    virtual bool onTimer(int id);

    virtual void onProgressChanged(ZKSeekBar *pSeekBar, int progress);

    virtual int getListItemCount(const ZKListView *pListView) const;
    virtual void obtainListItemData(ZKListView *pListView, ZKListView::ZKListItem *pListItem, int index);
    virtual void onItemClick(ZKListView *pListView, int index, int subItemIndex);

    virtual void onSlideItemClick(ZKSlideWindow *pSlideWindow, int index);

    virtual bool onTouchEvent(const MotionEvent &ev);

    virtual void onTextChanged(ZKTextView *pTextView, const string &text);

    void rigesterActivityTimer();

    virtual void onVideoPlayerMessage(ZKVideoView *pVideoView, int msg);
    void videoLoopPlayback(ZKVideoView *pVideoView, int msg, size_t callbackTabIndex);
    void startVideoLoopPlayback();
    void stopVideoLoopPlayback();
    bool parseVideoFileList(const char *pFileListPath, std::vector<string>& mediaFileList);
    int removeCharFromString(string& nString, char c);


private:
    /*TAG:PRIVATE_VARIABLE*/
    int mVideoLoopIndex;
    int mVideoLoopErrorCount;

};

#endif