#pragma once
#include "manager/LanguageManager.h"
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
}

static void onUI_quit() {

}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要先打开 activity/ChargeActivity.cpp onCreate 函数里面的registerTmer
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
    return true;
}

static bool onChargeActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上
    return false;
}
static bool onButtonClick_BtnFull(ZKButton *pButton) {
    //LOGD(" ButtonClick BtnFull !!!\n");
	Intent* intent = new Intent();
	intent->putExtra("value","100");
	intent->putExtra("pay","￥10.00");
	intent->putExtra("time", string("38") + LANGUAGEMANAGER->getValue("minutes"));
	EASYUICONTEXT->openActivity("Charge2Activity",intent);
	return false;
}

static bool onButtonClick_Btn82p(ZKButton *pButton) {
    //LOGD(" ButtonClick Btn82p !!!\n");
	Intent* intent = new Intent();
	intent->putExtra("value","82");
	intent->putExtra("pay","￥7.00");
	intent->putExtra("time", string("27") + LANGUAGEMANAGER->getValue("minutes"));
	EASYUICONTEXT->openActivity("Charge2Activity",intent);
	return false;
}

static bool onButtonClick_Btn52p(ZKButton *pButton) {
    //LOGD(" ButtonClick Btn52p !!!\n");
	Intent* intent = new Intent();
	intent->putExtra("value","58");
	intent->putExtra("pay","￥5.00");
	intent->putExtra("time", string("12") + LANGUAGEMANAGER->getValue("minutes"));
	EASYUICONTEXT->openActivity("Charge2Activity",intent);
	return false;
}

static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}


/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
//    {0,  6000}, //定时器id=0, 时间间隔6秒
//    {1,  1000},
};
