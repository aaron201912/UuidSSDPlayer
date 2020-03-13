/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __XINFENGACTIVITY_H__
#define __XINFENGACTIVITY_H__


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
#define ID_XINFENG_ButtonK4    20051
#define ID_XINFENG_Buttonkey1    20050
#define ID_XINFENG_ButtonKClear    20049
#define ID_XINFENG_ButtonK3    20048
#define ID_XINFENG_ButtonKCancel    20047
#define ID_XINFENG_ButtonK2    20046
#define ID_XINFENG_ButtonKEnter    20045
#define ID_XINFENG_ButtonK9    20044
#define ID_XINFENG_ButtonK0    20043
#define ID_XINFENG_ButtonK6    20042
#define ID_XINFENG_ButtonK7    20041
#define ID_XINFENG_ButtonK5    20040
#define ID_XINFENG_ButtonK8    20039
#define ID_XINFENG_TextPass    50031
#define ID_XINFENG_WindowKeyPad    100013
#define ID_XINFENG_Text5    50030
#define ID_XINFENG_Text3    50029
#define ID_XINFENG_Button1Year    20038
#define ID_XINFENG_Button6Month    20037
#define ID_XINFENG_Button3Month    20036
#define ID_XINFENG_Button1Month    20035
#define ID_XINFENG_Text1    50028
#define ID_XINFENG_WindowPay    100012
#define ID_XINFENG_TextFanName    50027
#define ID_XINFENG_TextFanSpeed    50026
#define ID_XINFENG_SeekBarFanSpeed    90004
#define ID_XINFENG_window_fanspeed    100010
#define ID_XINFENG_TextTempturein    50019
#define ID_XINFENG_Text10    50017
#define ID_XINFENG_Text8    50015
#define ID_XINFENG_Window3    100008
#define ID_XINFENG_TextTemptureout    50018
#define ID_XINFENG_Text9    50016
#define ID_XINFENG_Text7    50014
#define ID_XINFENG_Window2    100007
#define ID_XINFENG_TextCO2Status    50013
#define ID_XINFENG_ButtonCO2Status    20031
#define ID_XINFENG_Text4    50008
#define ID_XINFENG_TextCO2Value    50007
#define ID_XINFENG_Window1    100006
#define ID_XINFENG_TextPMstatus    50012
#define ID_XINFENG_Text2    50006
#define ID_XINFENG_BtnPMstatus    20030
#define ID_XINFENG_TextPMValue    50005
#define ID_XINFENG_View_2    100005
#define ID_XINFENG_btn_setup    20029
#define ID_XINFENG_btn_external_wind    20025
#define ID_XINFENG_btn_internal_wind    20022
#define ID_XINFENG_btn_power    20018
#define ID_XINFENG_btn_heat    20015
#define ID_XINFENG_btn_degerming    20011
#define ID_XINFENG_btn_auto    20007
#define ID_XINFENG_View_1    100002
#define ID_XINFENG_TextWiFi    50033
#define ID_XINFENG_TextWeek    50025
#define ID_XINFENG_TextDate    50024
#define ID_XINFENG_TextTime    50020
#define ID_XINFENG_ButtonPay    20034
#define ID_XINFENG_TextAD    50032
#define ID_XINFENG_Window4    100014
/*TAG:Macro宏ID END*/

class xinfengActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    xinfengActivity();
    virtual ~xinfengActivity();

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