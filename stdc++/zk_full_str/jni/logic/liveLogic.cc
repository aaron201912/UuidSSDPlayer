#pragma once
#include "uart/ProtocolSender.h"
#include <string.h>
#include <time.h>

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
#ifdef SUPPORT_PLAYER_PROCESS
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "ipc_common.h"
#else

#endif

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */

#ifdef SUPPORT_CLOUD_PLAY_MODULE
#include "appconfig.h"

#if USE_PANEL_1024_600
#define MAINWND_W       1024
#define MAINWND_H       600
#else
#define MAINWND_W       800
#define MAINWND_H       480
#endif
//static char g_file[50] = "http://122.112.182.239/mp4/test.m3u8";
static char g_file[50] = "http://124.71.177.235/t/tcl/video.m3u8";

static std::string g_address;
#endif

static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    //{0,  6000}, //定时器id=0, 时间间隔6秒
    //{1,  1000},
};

#ifdef SUPPORT_CLOUD_PLAY_MODULE
static int PlayComplete(void)
{
    mWindow_errorPtr->setVisible(true);
    mTextview_errorPtr->setText("视频播放结束！");
    return 0;
}
static int PlayError(int error_id)
{
    char error_text[20];
    sprintf(error_text,"error :%d", error_id);
    printf("connenct fail!\n");
    mWindow_errorPtr->setVisible(true);
    if (error_id & AV_NO_NETWORK) {
        mTextview_errorPtr->setText("无法访问网络！");
    } else if (error_id & AV_READ_TIMEOUT) {
        mTextview_errorPtr->setText("网络链接超时,请重试！");
    } else {
        mTextview_errorPtr->setText("未知错误!");
    }
    return 0;
}
#endif

#ifdef SUPPORT_PLAYER_PROCESS
static ipc_msg_t g_recvmsg;
extern int ipc_server_recv(ipc_msg_t *pmsg);
extern int ipc_server_send(ipc_msg_t *pmsg, uint8_t command);
extern int ipc_server_wait(ipc_msg_t *pmsg, int time_ms);
extern int ipc_server_open(char *app_name, char *link_file);
extern int ipc_server_close(char *link_file);
extern int ipc_server_running(void);
#endif

#ifdef SUPPORT_CLOUD_PLAY_MODULE
static bool g_exit = false;
static pthread_t idle_tid;

static void * tp_idle_thread(void *arg)
{
    int ret;
    ipc_msg_t *ipcmsg = (ipc_msg_t *)arg;

    printf("tp_idle_thread start\n");

    while (!g_exit)
    {
        if (!ipc_server_running()) {
            usleep(100 * 1000);
            continue;
        }

        ret = ipc_server_recv(ipcmsg);
        if (ret <= 0) {
            ipc_server_close(NULL);
            printf("client player may be exit!\n");
            continue;
        }

        switch (ipcmsg->cmd)
        {
            case PLAYER_COMPLETE : {
                printf("receive message of playing completely!\n");
                PlayComplete();
            } break;

            case PLAYER_ERROR : {
                printf("receive message of error in playing!\n");
                PlayError(ipcmsg->flags);
                g_exit= true;
                continue;
            } break;

            default : break;
        }
    }

    printf("### tp_idle_thread exit ###\n");

    return NULL;
}
#endif

/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
#ifdef SUPPORT_CLOUD_PLAY_MODULE
	printf("hrer  %s!!!\n",g_file);
	system("echo 12 > /sys/class/gpio/export");
	system("echo out > /sys/class/gpio/gpio12/direction");
	system("echo 1 > /sys/class/gpio/gpio12/value");
#endif
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
#ifdef SUPPORT_CLOUD_PLAY_MODULE
        int ret;
        //mTextview_addressPtr->setText(g_file);
        //tp_player_open("http://122.112.182.239/mp4/test.m3u8", 0, 0, 1024, 600);
        g_address = intentPtr->getExtra("address");
        printf("test  %s\n",g_address.c_str());
        strcpy(g_file,g_address.c_str());
        //g_file = g_address.c_str()
        mTextview_addressPtr->setText(g_file);

        ret = ipc_server_open(NULL, NULL);
        if (ret < 0) {
            mWindow_errorPtr->setVisible(true);
            mTextview_errorPtr->setText("播放进程异常!");
            printf("ipc_server_open falied!\n");
            return;
        }

        strcpy(g_recvmsg.url, g_file);
        g_recvmsg.window.x = 0;
        g_recvmsg.window.y = 0;
        g_recvmsg.window.width  = MAINWND_W;
        g_recvmsg.window.height = MAINWND_H;
        g_recvmsg.volumn = 10;
        g_recvmsg.mute   = 0;
        g_recvmsg.opts.audio_dev     = 0;
        g_recvmsg.opts.audio_layout  = AV_CH_LAYOUT_MONO;
        g_recvmsg.opts.enable_scaler = 0;
        #if ENABLE_ROTATE
        g_recvmsg.opts.video_rotate  = AV_ROTATE_270;
        #else
        g_recvmsg.opts.video_rotate  = AV_ROTATE_NONE;
        #endif
        g_recvmsg.opts.video_only    = 0;
        g_recvmsg.opts.audio_only    = 0;
        g_recvmsg.opts.play_mode     = AV_ONCE;
        g_recvmsg.opts.video_ratio   = AV_SCREEN_MODE;
        strcpy(g_recvmsg.opts.resolution, "8294400");
        ipc_server_send(&g_recvmsg, PLAYER_CREATE);
        ret= ipc_server_wait(&g_recvmsg, 1000);
        if (ret > 0) {
            printf("tp_player try to open file: %s\n", g_file);
        }

        g_exit = false;
        ret = pthread_create(&idle_tid, NULL, tp_idle_thread, (void *)(&g_recvmsg));
        if (ret != 0) {
            printf("[%s %d]create tp_idle_thread failed!\n", __FILE__, __LINE__);
            return;
        }
#endif
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
static bool onliveActivityTouchEvent(const MotionEvent &ev) {
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
#ifdef SUPPORT_CLOUD_PLAY_MODULE
    int ret;

    if (ipc_server_running()) {
        ipc_server_send(&g_recvmsg, PLAYER_DESTORY);
        ipc_server_wait(&g_recvmsg, 1000);
        ipc_server_close(NULL);
    }

    g_exit = true;
    if (idle_tid) {
        pthread_join(idle_tid, NULL);
        idle_tid = 0;
    }

    printf("tp_player_close done!\n");
#endif
    return false;
}

