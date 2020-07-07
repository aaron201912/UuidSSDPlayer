/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __CHARGEACTIVITY_H__
#define __CHARGEACTIVITY_H__


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
#define ID_CHARGE_Btn52p    20004
#define ID_CHARGE_Btn82p    20003
#define ID_CHARGE_BtnFull    20000
#define ID_CHARGE_Label_19    50022
#define ID_CHARGE_Label_18    50021
#define ID_CHARGE_Label_17    50020
#define ID_CHARGE_Label_16    50019
#define ID_CHARGE_Label_15    50018
#define ID_CHARGE_Label_8    50007
#define ID_CHARGE_Label_7    50006
#define ID_CHARGE_Label_6    50005
#define ID_CHARGE_Label_14    50017
#define ID_CHARGE_Label_11    50014
#define ID_CHARGE_Label_5    50004
#define ID_CHARGE_View_4    100003
#define ID_CHARGE_Label_13    50016
#define ID_CHARGE_Label_10    50009
#define ID_CHARGE_Label_4    50003
#define ID_CHARGE_View_3    100002
#define ID_CHARGE_Label_12    50015
#define ID_CHARGE_Label_9    50008
#define ID_CHARGE_Label_3    50002
#define ID_CHARGE_View_2    100001
#define ID_CHARGE_Label_2    50001
#define ID_CHARGE_Label_1    50000
#define ID_CHARGE_View_1    100000
/*TAG:Macro宏ID END*/

class ChargeActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    ChargeActivity();
    virtual ~ChargeActivity();

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
    void videoLoopPlayback(ZKVideoView *pVideoView, int msg, int callbackTabIndex);
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