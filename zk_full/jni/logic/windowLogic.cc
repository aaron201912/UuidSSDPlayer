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
#include "manager/LanguageManager.h"
#include "statusbarconfig.h"

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
	mTextview6Ptr->setText(LANGUAGEMANAGER->getValue("drag_and_drop_window_tip"));
}

static void onUI_quit() {
	ShowStatusBar(1, 0, 0);
}



static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
    return true;
}


static bool onwindowActivityTouchEvent(const MotionEvent &ev) {
    static MotionEvent last_ev;
    switch (ev.mActionStatus) {
        case MotionEvent::E_ACTION_DOWN://触摸按下
            last_ev = ev;
            break;
        case MotionEvent::E_ACTION_MOVE://触摸滑动
        {
            int x_offset = ev.mX - last_ev.mX;
            int y_offset = ev.mY - last_ev.mY;
            if (mWindowDragPtr->getPosition().isHit(ev.mX, ev.mY) && mWindowDragPtr->isWndShow()) {
                static LayoutPosition pos;
                pos = mWindowDragPtr->getPosition();
                pos.mLeft += x_offset;
                pos.mTop += y_offset;
                mWindowDragPtr->setPosition(pos);
            }
            last_ev = ev;
        }
            break;
        case MotionEvent::E_ACTION_UP:  //触摸抬起
            break;
        default:
            break;
    }
    return false;
}

static bool onButtonClick_Buttonmode(ZKButton *pButton) {
    //LOGD(" ButtonClick Buttonmode !!!\n");
	mWindowmodePtr->showWnd();
    return true;
}

static bool onButtonClick_Buttonnormal(ZKButton *pButton) {
    //LOGD(" ButtonClick Buttonnormal !!!\n");
	if(mWindownormalPtr->isWndShow()){
		mWindownormalPtr->hideWnd();
		pButton->setTextTr("show_window");
	}else{
		mWindownormalPtr->showWnd();
		pButton->setTextTr("hide_window");
	}
    return true;
}

static bool onButtonClick_Button4(ZKButton *pButton) {
    //LOGD(" ButtonClick Button4 !!!\n");
	mWindowmodePtr->hideWnd();
    return true;
}

static bool onButtonClick_Button3(ZKButton *pButton) {
    //LOGD(" ButtonClick Button3 !!!\n");
	mWindownormalPtr->hideWnd();
    return true;
}

static bool onButtonClick_Button1(ZKButton *pButton) {
    //LOGD(" ButtonClick Button1 !!!\n");
    return false;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
static bool onButtonClick_Button2(ZKButton *pButton) {
    mWindowmodePtr->hideWnd();
    mWindownormalPtr->hideWnd();
    mWindowDragPtr->showWnd();
    return false;
}
