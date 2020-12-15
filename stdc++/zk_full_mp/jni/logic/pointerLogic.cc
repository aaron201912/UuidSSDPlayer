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
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

static int sCarTemp[] = { 28, 32, 27, 30, 29, 26, 32, 31 };
static float totoalbase=1000;
static float curmil=0;
static float leaveMail = 500;

static int SpeedTemp[]={0,   3,    5,  10,  12, 18,  25,  36,49 ,60,   50,   65, 80  ,90,     110,120, 110,100,100,   90,95,95};
static int RPmTab[]={1500,1700,2000,2500,2600,2650,2400,2500,2500,2860,2560,2700,3200,3000,3000,3000,2800,2600,2000,2150,2000,2000};
static double oilTab[]={15,17,15,15,10,10,12,10,8,7.5,7.5,7.2,7.5,7.5,6.5,8,7.2,7.6,7.0,7.0,6.8,6.8};

static unsigned char s_resp_data[] = {
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7E, 0x7F, 0x81, 0x83, 0x83, 0x84, 0x85, 0x86,
	0x87, 0x87, 0x87, 0x87, 0x86, 0x85, 0x84, 0x83,

	0x82, 0x80, 0x7F, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7C, 0x7B, 0x7A, 0x78, 0x76, 0x79, 0x85,
	0x90, 0x9B, 0xA5, 0xB0, 0xB4, 0xAA, 0x9F, 0x94,
	0x89, 0x7E, 0x76, 0x77, 0x79, 0x7A, 0x7C, 0x7C,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D,
	0x7E, 0x80, 0x80, 0x82, 0x83, 0x84, 0x86, 0x87,
	0x88, 0x89, 0x8A, 0x8B, 0x8B, 0x8C, 0x8D, 0x8E,
	0x8F, 0x8F, 0x8F, 0x90, 0x90, 0x91, 0x90, 0x90,
	0x8F, 0x8E, 0x8D, 0x8C, 0x8B, 0x89, 0x89, 0x88,
	0x86, 0x85, 0x83, 0x82, 0x80, 0x7F, 0x7E, 0x7D,
	0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D
};

int speedindex = 0;
bool binc = 1;

int tmpindex = 0;
int dirindex = 0;
bool btest = 0;
#define DIAGRAM_SIZE 30
//static SZKPoint sPoints[DIAGRAM_SIZE];

#if 0
static void movePoints(SZKPoint* p, int size) {
	for (int i = 0; i < size - 1; i++) {
		p[i].y = p[i + 1].y;
	}
	p[size - 1].y = 0;
}
#endif

static void updateGUI() {
	if (binc) {
		if (++speedindex >= (int)(sizeof(SpeedTemp) / sizeof(int))) {
			binc = 0;
			speedindex -= 1;
		}
	} else {
		if (--speedindex < 0) {
			speedindex = 0;
			binc = 1;
		}
	}

	int spe = SpeedTemp[speedindex];

	char buff[50];

	totoalbase += ((float)spe/7200);
	sprintf(buff,"%.2f KM",totoalbase);
	mLabel_TotalMPtr->setText(buff);

	char buff1[50];
	curmil += ((float)spe/7200);
	sprintf(buff1,"%.2f KM",curmil);
	mLabel_CURMPtr->setText(buff1);

	char buff2[50];
	sprintf(buff2,"%d r/min",RPmTab[speedindex]);
	mLabel_RPMPtr->setText(buff2);

	char buff3[50];
	leaveMail = 500.0 - curmil;
	if (leaveMail < 0)
		leaveMail = 0;
	sprintf(buff3, "%.2f KM", leaveMail);
	mLabel_OIL1Ptr->setText(buff3);

	char buff4[50];
	sprintf(buff4,"%.1f/100KM",oilTab[speedindex]);
	mLabel_OILPtr->setText(buff4);
}

static void updateGUI_Tempture() {
	if (++tmpindex >= (int)(sizeof(sCarTemp) / sizeof(int))) {
		tmpindex = 0;
	}
	char buff5[50];
	sprintf(buff5, "%d ℃ ", sCarTemp[tmpindex]);
	mLabel_TEMPPtr->setText(buff5);
}

static int sta = 0;
static int value = 0;

static void updateAngleAndWave() {
	if (sta == 0) {
		value = value > 0 ? value - 2 : value;
		if (value <= 21)
			sta = 1;
	} else if (sta == 1) {
		value = (value >= 0 && value < 45) ? value + 3 : value;
		if (value >= 45)
			sta = 2;
	} else if (sta == 2) {
		value = (value >= 45 && value < 75) ? value + 2 : value;
		if (value >= 75)
			sta = 3;
	} else if (sta == 3) {
		value = value > 55 ? value - 1 : value;
		if (value <= 55)
			sta = 4;
	} else if (sta == 4) {
		value = value < 115 ? value + 1 : value;
		if (value >= 115)
			sta = 0;
	}

	mDashbroadView_1Ptr->setTargetAngle(value);

	if (mDiagram1Ptr != NULL) {
#if 0
		sPoints[DIAGRAM_SIZE - 1].y = (RPmTab[speedindex]) / 50;
		mDiagram1Ptr->setData(0, sPoints, DIAGRAM_SIZE);
#else
		static int index2 = 0;
		index2++;
		if (index2 > 50)
			index2 = 0;

		int value = index2 * 255 / 50;
		int v = (s_resp_data[value] - 0x70) * 90 / 0x90 + 10;
		mDiagram1Ptr->addData(0, v);
#endif
	}
}

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	{0,  300}, //定时器id=0, 时间间隔6秒
	{1,  1000},
	{2,  20},
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
//	for (int i =0;i < DIAGRAM_SIZE;i++) {
//		sPoints[i].x = (100*i)/DIAGRAM_SIZE;
//		sPoints[i].y = 50;
//	}
	updateGUI();
	updateGUI_Tempture();
	EASYUICONTEXT->hideStatusBar();
}


/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {

}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {

}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
//	EASYUICONTEXT->showStatusBar();
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id) {
	if (id == 0) {
		updateGUI();
	} else if (id == 1) {
		updateGUI_Tempture();
	} else if (id == 2) {
		updateAngleAndWave();
	}

	return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onpointerActivityTouchEvent(const MotionEvent &ev) {
	return false;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
