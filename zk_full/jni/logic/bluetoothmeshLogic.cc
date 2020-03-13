#pragma once
#include "uart/ProtocolSender.h"
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "statusbarconfig.h"

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

#define BL_DEV	"/dev/ttyS1"
#define CHECK_FD_VALID(fd)	do {	\
	if (fd < 0)	\
		return -1;	\
} while (0);

static int g_blFd = -1;

int BL_MESH_Init()
{
	struct termios opts;

	if (g_blFd >= 0)
	{
		printf("device has been initialized\n");
		return 0;
	}

	g_blFd = open(BL_DEV, O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (g_blFd < 0)
	{
		printf("open device failed\n");
		return -1;
	}

	fcntl(g_blFd, F_SETFL, 0); //重设为堵塞状态， 去掉O_NONBLOCK
	tcgetattr(g_blFd, &opts); //把原设置获取出来，存放在opts

	//设置波特率
	cfsetispeed(&opts, B115200);
	cfsetospeed(&opts, B115200);

	opts.c_cflag |= CLOCAL|CREAD; //忽略modem控制位 启动接收位
	opts.c_cflag &= ~PARENB;		// 8N1
	opts.c_cflag &= ~CSTOPB;
	opts.c_cflag |= CS8;
	opts.c_cflag &= ~CRTSCTS; //关闭硬件流控
	opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
	opts.c_oflag &= ~OPOST; // raw output
	tcsetattr(g_blFd, TCSANOW, &opts);

	return 0;
}


void BL_MESH_Deinit()
{
	if (g_blFd >= 0)
	{
		close(g_blFd);
		g_blFd = -1;
	}
}

int BL_MESH_Reset()
{
	int ret = -1;
	char data[1024];
	char send_data[1024];

	CHECK_FD_VALID(g_blFd);

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"delete\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++delete++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	return ret;
}

int BL_MESH_SetConfig()
{
	int ret = -1;
	char data[1024];
	char send_data[1024];

	CHECK_FD_VALID(g_blFd);

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"config addr 0x0010\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++config addr++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d, %s\n", ret, data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"config iv 0x1 0\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++config iv++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"key add net 63964771734fbd76e3b40519d1d94a48 0\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++key add net++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"key add app 63964771734fbd76e3b40519d1d94a48 0x123 0\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++key add app++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"config bind\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++config bind++++++++++\n");
	sleep(1);

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"prov run 2163453296847e44827618f6e02c404a 63964771734fbd76e3b40519d1d94a48 0 0x1 0x2 0\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++prov run++++++++++\n");

	memset(data,0,sizeof(data));
	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"config addr 0x0010\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");

	ret = read(g_blFd, data, sizeof(data));
	if(ret > 0)
	{
		//data[ret] = 0;
		printf("got : %d,%s\n", ret,data);
	}
	else
	{
		printf("not got data ret: %d\n", ret);
	}
	memset(data,0,sizeof(data));

	return ret;
}


int BL_MESH_SetStatus(bool bOn)
{
	int ret = -1;
	char data[1024];
	char send_data[1024];

	CHECK_FD_VALID(g_blFd);

	if (bOn)
	{
		memset(send_data,0,sizeof(send_data));
		strcpy(send_data,"msg onoff set 0x2 1 1\r");
		ret = write(g_blFd,send_data,strlen(send_data));
		if(ret != strlen(send_data))
			printf("send ack fail\n");
		printf("+++++++msg on++++++++++\n");
	}
	else
	{
		memset(send_data,0,sizeof(send_data));
		strcpy(send_data,"msg onoff set 0x2 0 1\r");
		ret = write(g_blFd,send_data,strlen(send_data));
		if(ret != strlen(send_data))
			printf("send ack fail\n");
		printf("+++++++msg off++++++++++\n");
	}

	mButtonLedswPtr->setSelected(bOn);

	return ret;
}


int BL_MESH_Scan()
{
	int ret = -1;
	char data[1024];
	char send_data[1024];

	CHECK_FD_VALID(g_blFd);

	memset(send_data,0,sizeof(send_data));
	strcpy(send_data,"prov scan 1\r");
	ret = write(g_blFd,send_data,strlen(send_data));
	if(ret != strlen(send_data))
		printf("send ack fail\n");
	printf("+++++++scan dev++++++++++\n");

	return ret;
}

/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	BL_MESH_Init();
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }

    BL_MESH_Reset();
    BL_MESH_SetConfig();
    BL_MESH_SetStatus(false);
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
	BL_MESH_Deinit();
	ShowStatusBar(1, 0, 0);
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
static bool onUI_Timer(int id){
	switch (id) {

		default:
			break;
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
static bool onbluetoothmeshActivityTouchEvent(const MotionEvent &ev) {
    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			break;
		default:
			break;
	}
	return false;
}
static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
static int getListItemCount_Listview1(const ZKListView *pListView) {
    //LOGD("getListItemCount_Listview1 !\n");
    return 5;
}

static void obtainListItemData_Listview1(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ Listview1  !!!\n");
}

static void onListItemClick_Listview1(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ Listview1  !!!\n");
}
static bool onButtonClick_ButtonLedsw(ZKButton *pButton) {
    //LOGD(" ButtonClick ButtonLedsw !!!\n");
	BL_MESH_SetStatus(!mButtonLedswPtr->isSelected());
    return false;
}
