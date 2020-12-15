/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __TTSACTIVITY_H__
#define __TTSACTIVITY_H__


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
#define ID_TTS_Button_init    20011
#define ID_TTS_ListView_speaker    80002
#define ID_TTS_ListView_lang    80001
#define ID_TTS_Button_speaker    20010
#define ID_TTS_TextView_speaker_title    50009
#define ID_TTS_Button_lang    20009
#define ID_TTS_TextView_lang_title    50008
#define ID_TTS_Textview_setPitch    50007
#define ID_TTS_Textview_getPitch    50006
#define ID_TTS_Textview_setSpeed    50005
#define ID_TTS_Button_setPitch    20008
#define ID_TTS_Button_getPitch    20007
#define ID_TTS_Button_setSpeed    20006
#define ID_TTS_Button_getSpeed    20005
#define ID_TTS_Textview_getSpeed    50004
#define ID_TTS_Textview_setVol    50003
#define ID_TTS_Button_setVol    20004
#define ID_TTS_Textview_getVol    50002
#define ID_TTS_Button_stop    20003
#define ID_TTS_Button_play    20002
#define ID_TTS_Button_getVol    20001
#define ID_TTS_sys_back   100
#define ID_TTS_Textview_text    50001
/*TAG:Macro宏ID END*/

class ttsActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    ttsActivity();
    virtual ~ttsActivity();

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