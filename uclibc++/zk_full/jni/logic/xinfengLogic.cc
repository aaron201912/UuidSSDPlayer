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
#include "uart/ProtocolData.h"
#include "uart/ProtocolParser.h"
#include "utils/TimeHelper.h"
#include "uart/ProtocolSender.h"

static SProtocolData mProtocolData;
#define SHOWWND_INTWND	1
#define SHOWWND_OUTWND	2
static BYTE mbShowWndMode = 0;
std::string strpasswd;

static void ProtocolDataUpdateFun(const SProtocolData &data);
static void updateUI_time(){
	char timeStr[20];
	//static bool bflash = false;
	struct tm *t = TimeHelper::getDateTime();

	//if(bflash){
		sprintf(timeStr, "%02d:%02d", t->tm_hour,t->tm_min);
	//	bflash =false;
	//}else{
	//	sprintf(timeStr, "%02d %02d", t->tm_hour,t->tm_min);
	//	bflash = true;
	//}
	mTextTimePtr->setText(timeStr);

	sprintf(timeStr, "%d/%02d/%02d", 1900 + t->tm_year, t->tm_mon + 1, t->tm_mday);
	mTextDatePtr->setText(timeStr);

	static const char *day[] = { "Sun.", "Mon.", "Tues.", "Wed.", "Thur.", "Fri.", "Sat." };
//	sprintf(timeStr, "星期%s", day[t->tm_wday]);
	mTextWeekPtr->setText(day[t->tm_wday]);
}
static void UpdateUI_PM25(){
	char str[10];
	snprintf(str, 10, "%d", mProtocolData.pm2_5);
	mTextPMValuePtr->setText(str);
	int textColor = 0xFFFFFF;
	if (/*(mProtocolData.pm2_5 > 35) && */(mProtocolData.pm2_5 < 75)) {
		//textColor = 0x0000FF;	// 蓝色
		mTextPMstatusPtr->setText("1");
		mBtnPMstatusPtr->setText("Good");
	} else if ((mProtocolData.pm2_5 >= 75) && (mProtocolData.pm2_5 < 115)) {
		textColor = 0xFFFF00;	// 黄色
		mTextPMstatusPtr->setText("2");
		mBtnPMstatusPtr->setText("Mild");
	} else if (mProtocolData.pm2_5 >= 115) {
		textColor = 0xFF0000;	// 红色
		mTextPMstatusPtr->setText("3");
		mBtnPMstatusPtr->setText("Serious");
	}
	mTextPMValuePtr->setTextColor(textColor);

}
static void UpdateUI_Tempture(){
	mTextTemptureoutPtr->setText(mProtocolData.externalTemperature);
	mTextTemptureinPtr->setText(mProtocolData.internalTemperature);
}
static void UpdateUI_Bar(){
	char str[10];
	mbtn_autoPtr->setSelected(mProtocolData.eRunMode == E_RUN_MODE_MANUAL);

	mbtn_heatPtr->setSelected(mProtocolData.heat);

	mbtn_degermingPtr->setSelected(mProtocolData.uvSJ);

	snprintf(str, 10, "%d",mProtocolData.internalWindSpeedLevel);
	mbtn_internal_windPtr->setText(str);

	snprintf(str, 10, "%d",mProtocolData.externalWindSpeedLevel);
	mbtn_external_windPtr->setText(str);
}
static void UpdateUI_CO2(){
	mTextCO2ValuePtr->setText(mProtocolData.co2);
	if(mProtocolData.co2 > 3000){
		mButtonCO2StatusPtr->setText("Range");
		mTextCO2StatusPtr->setText("3");
		mTextCO2ValuePtr->setTextColor(0xFF0000);
	}else if(mProtocolData.co2 > 1000){
		mTextCO2ValuePtr->setTextColor(0xFFFF00);
		mButtonCO2StatusPtr->setText("Bad");
		mTextCO2StatusPtr->setText("2");
	}else{
		mTextCO2ValuePtr->setTextColor(0xFFFFFF);
		mButtonCO2StatusPtr->setText("Good");
		mTextCO2StatusPtr->setText("1");
	}
}
static void initControls() {
	mProtocolData = getProtocolData();
	registerProtocolDataUpdateListener(&ProtocolDataUpdateFun);
	BYTE reqCmd = 0xFF;
}

static void sendMcuHearbeat(){

	BYTE reqCmd = 0x00;
	reqCmd |= mProtocolData.power&0x01;
	reqCmd |= (mProtocolData.power&0x01)<<3;
//	sendProtocol(CMDID_SYS_STATUS,0,&reqCmd,1);
}
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
	mWindowPayPtr->hideWnd();
	initControls();
	updateUI_time();
//	UpdateUI_Bar();
//	UpdateUI_CO2();
//	UpdateUI_PM25();
//	UpdateUI_Tempture();
	EASYUICONTEXT->hideStatusBar();
}

static void onUI_quit() {
	unregisterProtocolDataUpdateListener(&ProtocolDataUpdateFun);
//	EASYUICONTEXT->showStatusBar();
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要先打开 activity/xinfengActivity.cpp onCreate 函数里面的registerTmer
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
	static int heard = 0;
	updateUI_time();

	++heard;

	//发送心跳包
//	if(heard%2 == 0){
//		sendMcuHearbeat();
//	}
	return true;
}

static bool onxinfengActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上
    return false;
}
static bool onButtonClick_ButtonPay(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonPay !!!\n");
	 //LOGD(" ButtonClick ButtonPay !!!\n");
	LOGD(" ButtonClick ButtonPay !!!\n");
	mWindowPayPtr->showWnd();
	return true;
}

static bool onButtonClick_btn_auto(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_auto !!!\n");
	LOGD(" ButtonClick btn_auto !!!\n");
	if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	BYTE mode = mbtn_autoPtr->isSelected() ? E_RUN_MODE_AUTO : E_RUN_MODE_MANUAL;
//	sendProtocol(CMDID_RUN_MODE, 0, &mode, 1);
	mbtn_autoPtr->setSelected(!mbtn_autoPtr->isSelected());
	return true;
}

static bool onButtonClick_btn_degerming(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_degerming !!!\n");
if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();

		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	BYTE degerming = mbtn_degermingPtr->isSelected() ? 0 : 1;
//	sendProtocol(CMDID_UVLIGHT, 0, &degerming, 1);
	mbtn_degermingPtr->setSelected(!mbtn_degermingPtr->isSelected());
	return true;
}

static bool onButtonClick_btn_heat(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_heat !!!\n");
	LOGD(" ButtonClick btn_heat %d!!!\n",mbtn_heatPtr->isSelected());
	if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();

		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	BYTE hot = mbtn_heatPtr->isSelected() ? 0 : 1;

//	sendProtocol(CMDID_HEAT, 0, &hot, 1);
	mbtn_heatPtr->setSelected(!mbtn_heatPtr->isSelected());
	return true;
}

static bool onButtonClick_btn_power(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_power !!!\n");
	if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	//EASYUICONTEXT->openActivity("sliderwindowActivity");
	EASYUICONTEXT->goBack();
	return true;
}

static bool onButtonClick_btn_internal_wind(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_internal_wind !!!\n");
	if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	if(mwindow_fanspeedPtr->isWndShow()){
		mwindow_fanspeedPtr->hideWnd();
	}else{
		mbShowWndMode =  SHOWWND_INTWND;
		mSeekBarFanSpeedPtr->setProgress(mProtocolData.internalWindSpeedLevel);
		char str[10];
		snprintf(str, 10, "%d", mProtocolData.internalWindSpeedLevel);
		mTextFanSpeedPtr->setText(str);
		mTextFanNamePtr->setText("Indoor wind speed：");
		mwindow_fanspeedPtr->showWnd();
	}
	return true;
}

static bool onButtonClick_btn_external_wind(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_external_wind !!!\n");
if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	if(mwindow_fanspeedPtr->isWndShow()){
		mwindow_fanspeedPtr->hideWnd();
	}else{
		mbShowWndMode =  SHOWWND_OUTWND;
		mSeekBarFanSpeedPtr->setProgress(mProtocolData.externalWindSpeedLevel);
		char str[10];
		snprintf(str, 10, "%d", mProtocolData.externalWindSpeedLevel);
		mTextFanSpeedPtr->setText(str);
		mTextFanNamePtr->setText("Outdoor wind speed：");
		mwindow_fanspeedPtr->showWnd();
	}
	return true;
}

static bool onButtonClick_btn_setup(ZKButton *pButton) {
    //LOGD(" ButtonClick btn_setup !!!\n");
	if(getProtocolData().childLock){
		mWindowKeyPadPtr->showWnd();
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
		return false;
	}
	EASYUICONTEXT->openActivity("SetupActivity");
	return true;
}

static bool onButtonClick_BtnPMstatus(ZKButton *pButton) {
    //LOGD(" ButtonClick BtnPMstatus !!!\n");
    return false;
}

static bool onButtonClick_ButtonCO2Status(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonCO2Status !!!\n");
    return false;
}

static void onProgressChanged_SeekBarFanSpeed(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged SeekBarFanSpeed %d !!!\n", progress);
	char str[10];
	int value;
	if(mbShowWndMode == SHOWWND_INTWND){
		value =progress;
//		sendProtocol(CMDID_WIND_SPEED_LEVEL, 0, (const BYTE *) &value, 1);
		mProtocolData.internalWindSpeedLevel = progress;
		getProtocolData().internalWindSpeedLevel = progress;
		mTextFanSpeedPtr->setText(progress);
		mbtn_internal_windPtr->setText(progress);

	}else if(mbShowWndMode ==  SHOWWND_OUTWND){
		value =progress;
//		sendProtocol(CMDID_WIND_SPEED_LEVEL, 1, (const BYTE *) &value, 1);
		mProtocolData.externalWindSpeedLevel = progress;
		getProtocolData().externalWindSpeedLevel = progress;
		mTextFanSpeedPtr->setText(progress);
		mbtn_external_windPtr->setText(progress);
	}
}
static void ProtocolDataUpdateFun(const SProtocolData &data){

	if(mProtocolData.power != data.power){
		mProtocolData.power = data.power;
		/*
		if(data.power){
			EASYUICONTEXT->openActivity("MainPageActivity");
		}else{
			EASYUICONTEXT->openActivity("poweroffActivity");
		}
		*/
	}

	if (mProtocolData.eRunMode != data.eRunMode) {
		mProtocolData.eRunMode = data.eRunMode;
		mbtn_autoPtr->setSelected(mProtocolData.eRunMode == E_RUN_MODE_MANUAL);
		if(mProtocolData.eRunMode != E_RUN_MODE_MANUAL){
			mbtn_external_windPtr->setText(mProtocolData.externalWindSpeedLevel);
			mbtn_internal_windPtr->setText(mProtocolData.internalWindSpeedLevel);
		}
	}
	if (mProtocolData.heat != data.heat) {
		mProtocolData.heat = data.heat;
		mbtn_heatPtr->setSelected(mProtocolData.heat);
	}


	if (mProtocolData.uvSJ != data.uvSJ) {
		mProtocolData.uvSJ = data.uvSJ;
		mbtn_degermingPtr->setSelected(mProtocolData.uvSJ);
	}

	if (mProtocolData.co2 != data.co2) {
		mProtocolData.co2 = data.co2;
		UpdateUI_CO2();
	}

	if (mProtocolData.pm2_5 != data.pm2_5) {
		mProtocolData.pm2_5 = data.pm2_5;
		UpdateUI_PM25();
	}

	if (mProtocolData.internalWindSpeedLevel != data.internalWindSpeedLevel) {
		mProtocolData.internalWindSpeedLevel = data.internalWindSpeedLevel;
		if(mProtocolData.eRunMode != E_RUN_MODE_MANUAL){
			mbtn_internal_windPtr->setText(mProtocolData.internalWindSpeedLevel);
		}
	}

	if (mProtocolData.externalWindSpeedLevel != data.externalWindSpeedLevel) {
		mProtocolData.externalWindSpeedLevel = data.externalWindSpeedLevel;
		if(mProtocolData.eRunMode != E_RUN_MODE_MANUAL){
			mbtn_external_windPtr->setText(mProtocolData.externalWindSpeedLevel);
		}
	}
	if(mProtocolData.internalTemperature != data.internalTemperature){
		mProtocolData.internalTemperature = data.internalTemperature;
		mTextTemptureinPtr->setText(mProtocolData.internalTemperature);
	}
	if(mProtocolData.externalTemperature != data.externalTemperature){
		mProtocolData.externalTemperature = data.externalTemperature;
		mTextTemptureoutPtr->setText(mProtocolData.externalTemperature);
	}

	if (mProtocolData.openTime != data.openTime) {
		mProtocolData.openTime = data.openTime;
	}

	if (mProtocolData.closeTime != data.closeTime) {
		mProtocolData.closeTime = data.closeTime;
	}

	if (mProtocolData.sError != data.sError) {
		mProtocolData.sError = data.sError;
	}

	if (mProtocolData.iError != data.iError) {
		mProtocolData.iError = data.iError;
	}

	if (mProtocolData.gError != data.gError) {
		mProtocolData.gError = data.gError;
	}

	if (mProtocolData.fError != data.fError) {
		mProtocolData.fError = data.fError;
	}
	return;
}

static bool onButtonClick_Button1Month(ZKButton *pButton) {
    //LOGD(" ButtonClick Button1Month !!!\n");
	mButton1MonthPtr->setSelected(true);
	mButton3MonthPtr->setSelected(false);
	mButton6MonthPtr->setSelected(false);
	mButton1YearPtr->setSelected(false);
	return true;
}

static bool onButtonClick_Button3Month(ZKButton *pButton) {
    //LOGD(" ButtonClick Button3Month !!!\n");
	mButton1MonthPtr->setSelected(false);
	mButton3MonthPtr->setSelected(true);
	mButton6MonthPtr->setSelected(false);
	mButton1YearPtr->setSelected(false);
	return false;
}

static bool onButtonClick_Button6Month(ZKButton *pButton) {
    //LOGD(" ButtonClick Button6Month !!!\n");
	mButton1MonthPtr->setSelected(false);
	mButton3MonthPtr->setSelected(false);
	mButton6MonthPtr->setSelected(true);
	mButton1YearPtr->setSelected(false);
	return false;
}

static bool onButtonClick_Button1Year(ZKButton *pButton) {
    //LOGD(" ButtonClick Button1Year !!!\n");
	mButton1MonthPtr->setSelected(false);
	mButton3MonthPtr->setSelected(false);
	mButton6MonthPtr->setSelected(false);
	mButton1YearPtr->setSelected(true);
	return false;
}

static bool onButtonClick_ButtonK8(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK8 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="8";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK5(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK5 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="5";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK7(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK7 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="7";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK6(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK6 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="6";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK0(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK0 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="0";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK9(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK9 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="9";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonKEnter(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonKEnter !!!\n");
	if(strpasswd.length() != 6){
		strpasswd.clear();
		mTextPassPtr->setText("Locked, please enter the password");
	}
	if(strpasswd == getProtocolData().password){
		getProtocolData().childLock = false;
//		sendProtocol(CMDID_CHILD_LOCK,0,&getProtocolData().childLock,1);
		mWindowKeyPadPtr->hideWnd();
	}
	return false;
}
static bool onButtonClick_ButtonK2(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK2 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="2";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonKCancel(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonKCancel !!!\n");
	strpasswd.clear();
	mWindowKeyPadPtr->hideWnd();
	return false;
}

static bool onButtonClick_ButtonK3(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK3 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="3";
	mTextPassPtr->setText(strpasswd.c_str());
	return true;
}

static bool onButtonClick_ButtonKClear(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonKClear !!!\n");
	strpasswd.clear();
	mTextPassPtr->setText("Locked, please enter the password");
	return false;
}

static bool onButtonClick_Buttonkey1(ZKButton *pButton) {
    //LOGD(" ButtonClick Buttonkey1 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="1";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_ButtonK4(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonK4 !!!\n");
	if(strpasswd.length() >= 6) return false;
	strpasswd +="4";
	mTextPassPtr->setText(strpasswd.c_str());
	return false;
}

static bool onButtonClick_Button234(ZKButton *pButton) {
    //LOGD(" ButtonClick Button234 !!!\n");
    return true;
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
