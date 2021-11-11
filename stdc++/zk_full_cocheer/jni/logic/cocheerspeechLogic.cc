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
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "coapi_api/coapi_common.h"
#include "coapi_api/coapi_cmd.h"
#include "coapi_api/coapi_audio.h"
#include "coapi_api/coapi_logger.h"
#include "coapi_api/coapi_logic.h"
#include "coapi_api/coapi_timer.h"
#include "coapi_api/coapi_register.h"
#include "coapi_api/coapi_bindcode.h"
#include "coapi_api/coapi_devbind.h"
#include "coapi_api/coapi_tts.h"
#include "coapi_api/coapi_devsta.h"

#include "coapi_app/coapi_app.h"
#include "coapi_app/coapi_comm.h"
//#include "coapi_app/coapi_button.h"
#include "coapi_app/coapi_player.h"
#include "coapi_app/coapi_audio_plat.h"
#include "coapi_app/coapi_app.h"
#include "coapi_app/coapi_record.h"
#include "coapi_app/coapi_speech.h"
//#include "coapi_app/coapi_network_cfg.h"

#include "appconfig.h"
#include "statusbarconfig.h"

static pthread_t g_threadCocheer = 0;
static bool g_bExit = false;
static bool g_bSignedIn = false;

static const char* g_pAppKey = "31540983348731";
static const char* g_pSecretKey = "91d51fb962d9c9a3399079ba85dfb01d";
static dev_comm g_dev = {
	"测试",
	"00.01",
	"ssd201d",
	"linux",
	"",
	"小酷",
	"酷旗",
};

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	{0,  100}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};


static int reg = -1;
static unsigned char boot_completed = 0;
//static bool maikefengBtIsRedy		= true;

#if USE_AMIC
static MicInputType_e g_eMicType = E_AMIC_MONO_MODE;
#else
static MicInputType_e g_eMicType = E_DMIC_STEREO_MODE;
#endif

void cb_stop_record(){
	mmaikefengBtPtr->setSelected(false);
//	maikefengBtIsRedy		= true;
}

void cb_start_record(void){
	mmaikefengBtPtr->setSelected(true);
}

void clean_speek_text(void){
	mTextView1Ptr->setText("   ");
}

void clean_anser_text(void){
	mTextView2Ptr->setText("   ");
}

void cb_flash_ansr_text(char* text){
	if(NULL == text) {
		mTextView2Ptr->setText("  ");
	}else {
		mTextView2Ptr->setText(text);
	}
}

void cb_flash_speek_text(char* text){
	if(NULL == text) {
		mTextView1Ptr->setText("  ");
	} else {
		mTextView1Ptr->setText(text);
	}
}


/**
 * @breif 与云端的连接回调
 * 连接上云端后所有coapi的功能才能使用
 * 如需判断是否已经连接上云端使用coapi_logic.h的coapi_connect_status()
 * 函数主动查询,比回调实时性更好
 */
static void coapp_connect_cb(unsigned char status)
{
	switch(status) {
		case 0:
			LOGD("未连接 ");
			break;
		case 1:
			LOGD("连接中 ");
			break;
		case 2:
			LOGD("已连接 ");
			break;
		case 3:
			LOGD("链接关闭 ");
			break;
		case 4:
			LOGD("网络差，信号弱 ");
			break;
		case 5:
			LOGD("重新连接");
			break;
		case 6:
		default:
			LOGD("未知状态 ");
			break;
	}
}

/*
 * @brief 获取网络状态，由开发者实现函数体逻辑
 * @return 0: 网络断开
 *         1: 网络连接
 * 注意事项: 函数体的运行逻辑尽量简单，不能有耗时的操作，运行耗时<10毫秒
 */
static int coapp_wifi_cb(void)
{
	return 1;
}

static int get_dev_net_mac(char *szMac)
{
    struct ifreq    ifr,*it     = NULL;
    struct ifconf   ifc;
    char            buf[2048];
    int             success     = 0;
    unsigned char   *ptr        = NULL;

    if(NULL == szMac) {
        printf("NULL == szMac\n");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        printf("socket error\n");
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        printf("ioctl error\n");
        return -1;
    }

    it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
                    snprintf(szMac,12,"%02X%02X%02X%02X%02X%02X",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
                    printf("Interface name : %s , Mac address : %s \n",ifr.ifr_name,szMac);
                    break;
                }
            } else {

            }
        } else {
            printf("get mac info error\n");
            return -1;
        }
    }
    return 0;
}

static void *CocheerSpeechProc(void *pData)
{
	char    p_devid[13] ={0};
	

	audioInit(g_eMicType);
	get_dev_net_mac(p_devid);
	coapi_init(p_devid, NULL, g_pAppKey, g_pSecretKey, &g_dev);
	coapi_speech_init();
	coapi_record_create(0);

	while (!g_bExit)
	{
		if (!g_bSignedIn)
		{
			int retRegister = coapi_register_status();
			//printf("retRegister = %d\n", retRegister);

			if (retRegister != AUTH_STATE_AUTH_OK && retRegister != AUTH_STATE_HAS_AUTH)
			{
				usleep(100000);
			}
			else
			{
				g_bSignedIn = true;
				mTextView3Ptr->setText("已登录");
				
				/* 开始播放欢迎词 */
				printf("welcome\n");
				dev_player_local_start(WELCOME_WAV);		// 此处封装指定路径
				break;
			}
		}
	}

    return NULL;
}


/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");

}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }

    // sign in once
	if (g_bSignedIn)
	{
		mTextView3Ptr->setText("已登录");
		return;
	}

	printf("first sign in\n");

	mTextView3Ptr->setText("");

	if (access(COCHEER_AUDIO_DIR, R_OK))
	{
		if (!mkdir(COCHEER_AUDIO_DIR, 0777))
		{
			printf("create %s success\n", COCHEER_AUDIO_DIR);

			if (access(COCHEER_WAV_DIR, R_OK))
			{
				if (!mkdir(COCHEER_WAV_DIR, 0777))
					printf("create %s success\n", COCHEER_WAV_DIR);
				else
					return;
			}
		}
		else
		{
			return;
		}
	}

	g_bExit = false;
	mTextView3Ptr->setText("正在登录···");
	pthread_create(&g_threadCocheer, NULL, CocheerSpeechProc, NULL);
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
	// 登录成功，退出时停止播放；登录未成功，退出时deinit，停止底层的连接动作

	if (g_bSignedIn)
		dev_player_stop();
//	else
//		coapi_deinit();

	g_bExit = true;
	if (g_threadCocheer)
	{
		pthread_join(g_threadCocheer, NULL);
		g_threadCocheer = 0;
	}

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
	static int bootNum = 0;
	switch (id) {
	case 0:
		bootNum +=1;
		if(bootNum >= 20){
			bootNum = 0;
			mbootWinPtr->hideWnd();
			return false;
		}
		break;
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
static bool oncocheerspeechActivityTouchEvent(const MotionEvent &ev) {
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
static bool onButtonClick_helpBt(ZKButton *pButton) {
    LOGD(" ButtonClick helpBt !!!\n");
    if (!g_bSignedIn)
    	return false;

    return false;
}

static bool onButtonClick_maikefengBt(ZKButton *pButton) {
	LOGD(" ButtonClick maikefengBt !!!\n");
	if (!g_bSignedIn)
		return false;
		
#if 0		// wake up by voice
	pButton->setSelected(true);
	if( (pButton->isSelected()) && (maikefengBtIsRedy == true)) {
		maikefengBtIsRedy = false;			/* 防止重复点击 */
		clean_speek_text();
		clean_anser_text();
		LOGD("=================== mp3_play_stop \n\n");
		dev_player_stop();
		LOGD("=================== coapi_speech_start \n\n");
		coapi_speech_start();
	} else {
		//LOGD("=================== coapi_speech_stop \n\n");
		//coapi_speech_stop();
	}
#endif
    return false;
}

static bool onButtonClick_settingBt(ZKButton *pButton) {
    LOGD(" ButtonClick settingBt !!!\n");
    if (!g_bSignedIn)
    	return false;

    return false;
}

static bool onButtonClick_sys_back(ZKButton *pButton) {
    LOGD(" ButtonClick sys_back !!!\n");
    return false;
}
