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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct shared_use_st
{
    int  written;    //作为一个标志，非0：表示可读，0表示可写
    bool flag;
};

struct shared_use_st *tp_shm_addr = NULL;
int tp_shm_id = 0;

#define CLT_IPC     "/appconfigs/client_input"
#define SVC_IPC     "/appconfigs/server_input"

typedef enum
{
    IPC_COMMAND_OPEN,
    IPC_COMMAND_CLOSE,
    IPC_COMMAND_PAUSE,
    IPC_COMMAND_RESUME,
    IPC_COMMAND_SEEK,
    IPC_COMMAND_SEEK2TIME,
    IPC_COMMAND_GET_POSITION,
    IPC_COMMAND_GET_DURATION,
    IPC_COMMAND_MAX,
    IPC_COMMAND_ACK,
    IPC_COMMAND_SET_VOLUMN,
    IPC_COMMAND_SET_MUTE,
    IPC_COMMAND_ERROR,
    IPC_COMMAND_COMPLETE,
    IPC_COMMAND_CREATE,
    IPC_COMMAND_DESTORY,
    IPC_COMMAND_EXIT,
    IPC_COMMAND_PANT
} IPC_COMMAND_TYPE;

typedef struct{
    int x;
    int y;
    int width;
    int height;
    double misc;
    int aodev, volumn;
    int status;
    int rotate;
    bool mute;
    bool audio_only, video_only;
    int  play_mode;
    char filePath[512];
}stPlayerData;

typedef struct {
    unsigned int EventType;
    stPlayerData stPlData;
} IPCEvent;

class IPCOutput {
public:
    IPCOutput(const std::string& file):m_fd(-1), m_file(file) {
    }

    virtual ~IPCOutput() {
        Term();
    }

    bool Init() {
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_WRONLY | O_NONBLOCK, S_IWUSR | S_IWOTH);
            printf("[%s %d]IPCOutput m_fd = %d\n", __FILE__, __LINE__, m_fd);
        }
        return m_fd >= 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

    int Send(const IPCEvent& evt) {
        if (m_fd >= 0) {
            return write(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("write %s failed!\n", m_file.c_str());
        return -1;
    }

private:
    int m_fd, ret;
    std::string m_file;
};

class IPCNameFifo {
public:
    IPCNameFifo(const char* file): m_file(file) {
        unlink(m_file.c_str());
        m_valid = !mkfifo(m_file.c_str(), 0777);
    }

    ~IPCNameFifo() {
    unlink(m_file.c_str());
}

inline const std::string& Path() { return m_file; }
inline bool IsValid() { return m_valid; }

private:
    bool m_valid;
    std::string m_file;
};

class IPCInput {
public:
    IPCInput(const std::string& file):m_fd(-1),m_file(file),m_fifo(file.c_str()){}

    virtual ~IPCInput() {
        Term();
    }

    bool Init() {
        if (!m_fifo.IsValid()){
            printf("%s non-existent!!!! \n",m_fifo.Path().c_str());
            return false;
        }
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_RDWR | O_CREAT | O_NONBLOCK, S_IRWXU | S_IWOTH);
            printf("[%s %d]IPCInput m_fd = %d\n", __FILE__, __LINE__, m_fd);
        }
        return m_fd >= 0;
    }

    int Read(IPCEvent& evt) {
        if (m_fd >= 0) {
            return read(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("read %s failed!\n", m_file.c_str());
        return 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

private:
    int m_fd, ret;
    std::string m_file;
    IPCNameFifo m_fifo;
};

IPCEvent i_recvevt;
IPCEvent o_sendevt;
IPCInput  tp_server(SVC_IPC);
IPCOutput tp_client(CLT_IPC);

#endif

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */

#ifdef SUPPORT_CLOUD_PLAY_MODULE
#include "tp_player.h"

//static char g_file[50] = "http://122.112.182.239/mp4/test.m3u8";
static char g_file[50] = "http://124.71.177.235/t/tcl/video.m3u8";

static player_control_t g_pstPlayStatt;
static std::string g_address;
#endif

static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
    //{0,  6000}, //定时器id=0, 时间间隔6秒
    //{1,  1000},
};

#ifdef SUPPORT_CLOUD_PLAY_MODULE
static MI_S32 PlayCompletee()
{
    mWindow_errorPtr->setVisible(true);
    mTextview_errorPtr->setText("视频播放结束！");
    return 0;
}
static MI_S32 PlayErrorr(int error_id)
{
    char error_text[20];
    sprintf(error_text,"error :%d", error_id);
    printf("connenct fail!\n");
    mWindow_errorPtr->setVisible(true);
    if (-101 == error_id || -113 == error_id) {
        mTextview_errorPtr->setText("无法访问网络！");
    } else if (-4 == error_id) {
        mTextview_errorPtr->setText("网络链接超时,请重试！");
    } else {
        mTextview_errorPtr->setText("未知错误!");
    }
    return 0;
}
static void SetPlayerControlCallBackk(player_control_t *is)
{
    is->fpPlayComplete = PlayCompletee;
    is->fpPlayError = PlayErrorr;
}
#endif

#ifdef SUPPORT_PLAYER_PROCESS
#define USE_POPEN       1
#define PANT_TIME       5

#define MYPLAYER_PATH   "/customer/MyPlayer &"

extern int errno;

FILE *tp_fd = NULL;

static bool g_pantflag = false;
static int  g_status;
#endif

#ifdef SUPPORT_CLOUD_PLAY_MODULE
static bool enable_listen = false, g_exit = false;
static pthread_t idle_tid;

static void * tp_idle_thread(void *arg)
{
    int ret;
    player_control_t *func_t = (player_control_t *)arg;
    struct timeval pant_start, pant_wait;

    printf("tp_idle_thread start\n");

    while (!g_exit)
    {
        if (enable_listen) {
#ifdef SUPPORT_PLAYER_PROCESS
            if (tp_server.Read(i_recvevt) > 0) {
                if (i_recvevt.EventType == IPC_COMMAND_COMPLETE) {
                    printf("receive message of playing completely!\n");
                    func_t->fpPlayComplete();
                } else if (i_recvevt.EventType == IPC_COMMAND_ERROR) {
                    printf("receive message of error in playing!\n");
                    func_t->fpPlayError(i_recvevt.stPlData.status);
                    g_exit= true;
                } else if (i_recvevt.EventType == IPC_COMMAND_PANT) {
                    g_pantflag = true;
                    gettimeofday(&pant_start, NULL);
                    if(!tp_client.Init()) {
                        printf("[%s %d]my_player process not start!\n", __FILE__, __LINE__);
                        fprintf(stderr, "Error：%s\n", strerror(errno));
                    } else {
                        memset(&o_sendevt, 0, sizeof(IPCEvent));
                        o_sendevt.EventType = IPC_COMMAND_PANT;
                        tp_client.Send(o_sendevt);
                    }
                }
            }

            //心跳包判断
            gettimeofday(&pant_wait, NULL);
            if (g_pantflag && pant_wait.tv_sec - pant_start.tv_sec > 2 * PANT_TIME) {
                if (func_t && func_t->fpPlayError) {
                    func_t->fpPlayError(-1);
                }
                g_exit= true;
            }
#else
            ret = tp_player_status();
            if (ret < 0) {
                func_t->fpPlayError(ret);
                g_exit= true;
            } else if (ret > 0) {
                func_t->fpPlayComplete();
            }
#endif
        }
        usleep(10 * 1000);
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

	/*mTextview_addressPtr->setText(g_file);
	//SetPlayerControlCallBackk(&g_pstPlayStatt);
	g_pstPlayStatt.fpPlayError = PlayErrorr;
	g_pstPlayStatt.fpPlayComplete = PlayCompletee;
	//printf("in %p\n",&g_pstPlayStatt.fpPlayError);
	tp_player_open(g_file, 0, 0, 1024, 600, &g_pstPlayStatt);*/
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
        //SetPlayerControlCallBackk(&g_pstPlayStatt);
        g_pstPlayStatt.fpPlayError = PlayErrorr;
        g_pstPlayStatt.fpPlayComplete = PlayCompletee;
        //printf("in %p\n",&g_pstPlayStatt.fpPlayError);
#ifdef SUPPORT_PLAYER_PROCESS
        void *shm = NULL;
        struct timeval time_start, time_wait;

        printf("tp_player_open start!\n");

        memset(&i_recvevt, 0, sizeof(IPCEvent));

        if(!tp_server.Init()) {
            printf("[%s %d]create i_server fail!\n", __FILE__, __LINE__);
            fprintf(stderr, "Error：%s\n", strerror(errno));
            return -1;
        }

        #if USE_POPEN
        //创建共享内存
        tp_shm_id = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
        if(tp_shm_id < 0) {
            fprintf(stderr, "shmget failed\n");
            goto next;
        }

        //将共享内存连接到当前进程的地址空间
        shm = shmat(tp_shm_id, (void*)NULL, 0);
        if(shm < 0) {
            fprintf(stderr, "shmat failed\n");
            goto next;
        }

        tp_shm_addr = (struct shared_use_st *)shm;
        memset(tp_shm_addr, 0x0, sizeof(struct shared_use_st));
        printf("shared memory attached at %x\n", (int)shm);

        tp_fd = popen(MYPLAYER_PATH, "w");
        if (tp_fd == NULL) {
            printf("my_player is not exit!\n");
            goto next;
        }
        printf("popen myplayer progress done!\n");

        gettimeofday(&time_start, NULL);
        while (tp_server.Read(i_recvevt) <= 0
               || (i_recvevt.EventType != IPC_COMMAND_CREATE)) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 2) {
                printf("myplayer progress destory failed!\n");
                break;
            }
        }
next:
        if (i_recvevt.EventType == IPC_COMMAND_CREATE) {
            printf("myplayer progress create success!\n");
        } else {
            if (tp_shm_addr) {
                //把共享内存从当前进程中分离
                ret = shmdt((void *)tp_shm_addr);
                if (ret < 0) {
                    fprintf(stderr, "shmdt failed\n");
                }

                //删除共享内存
                ret = shmctl(tp_shm_id, IPC_RMID, NULL);
                if(ret < 0) {
                    fprintf(stderr, "shmctl(IPC_RMID) failed\n");
                }
            }
            tp_shm_addr = NULL;
            tp_shm_id = 0;

            g_pstPlayStatt.fpPlayError(-1);
            return;
        }
        #endif

        if(!tp_client.Init()) {
            printf("[%s %d]my_player is not start!\n", __FILE__, __LINE__);
            fprintf(stderr, "Error：%s\n", strerror(errno));
            return -1;
        }

        memset(&o_sendevt, 0, sizeof(IPCEvent));
        o_sendevt.EventType = IPC_COMMAND_OPEN;
        strcpy(o_sendevt.stPlData.filePath, g_file);
        // 旋转开关
        #if ENABLE_ROTATE
        o_sendevt.stPlData.rotate = E_MI_DISP_ROTATE_270;
        #else
        o_sendevt.stPlData.rotate = E_MI_DISP_ROTATE_NONE;
        #endif
        o_sendevt.stPlData.x = 0;
        o_sendevt.stPlData.y = 0;
        o_sendevt.stPlData.width  = MAINWND_W;
        o_sendevt.stPlData.height = MAINWND_H;
        o_sendevt.stPlData.aodev  = 0;
        o_sendevt.stPlData.audio_only = false;
        o_sendevt.stPlData.video_only = false;
        o_sendevt.stPlData.play_mode  = 0;    // 0: 单次播放,1: 循环播放(seek to start)
        tp_client.Send(o_sendevt);
        printf("tp_player try to open file: %s\n", g_file);

        memset(&i_recvevt, 0, sizeof(IPCEvent));
        while (tp_server.Read(i_recvevt) <= 0
               || ((i_recvevt.EventType != IPC_COMMAND_ACK)
               && (i_recvevt.EventType != IPC_COMMAND_ERROR))) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 10) {
                memset(&o_sendevt, 0, sizeof(IPCEvent));
                #if USE_POPEN
                o_sendevt.EventType = IPC_COMMAND_EXIT;
                #else
                o_sendevt.EventType = IPC_COMMAND_CLOSE;
                #endif
                tp_client.Send(o_sendevt);
                break;
            }
        }
        if (i_recvevt.EventType == IPC_COMMAND_ACK) {
            printf("receive ack from my_player!\n");
        } else if (i_recvevt.EventType == IPC_COMMAND_ERROR) {
            g_status = i_recvevt.stPlData.status;
            printf("my_player occur error [%d]!\n", g_status);
            g_pstPlayStatt.fpPlayError(g_status);
            return;
        }
        g_pantflag = false;
#else
        tp_player_open(g_file, 0, 0, MAINWND_W, MAINWND_H, &g_pstPlayStatt);
#endif
        g_exit = false;
        ret = pthread_create(&idle_tid, NULL, tp_idle_thread, (void *)(&g_pstPlayStatt));
        if (ret != 0) {
            printf("[%s %d]create tp_idle_thread failed!\n", __FILE__, __LINE__);
            return;
        }
        enable_listen = true;
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
    enable_listen = false;
#ifdef SUPPORT_PLAYER_PROCESS
    struct timeval time_start, time_wait;

    if(tp_client.Init()) {
        memset(&o_sendevt, 0, sizeof(IPCEvent));
        #if USE_POPEN
        o_sendevt.EventType = IPC_COMMAND_EXIT;
        tp_client.Send(o_sendevt);

        memset(&o_sendevt, 0, sizeof(IPCEvent));
        gettimeofday(&time_start, NULL);
        while ((tp_server.Read(o_sendevt) <= 0 || o_sendevt.EventType != IPC_COMMAND_DESTORY) &&
               (tp_shm_addr && (tp_shm_addr->written || !tp_shm_addr->flag))) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 2) {
                printf("myplayer progress destory failed!\n");
                break;
            }
        }
        #else
        o_sendevt.EventType = IPC_COMMAND_CLOSE;
        tp_client.Send(o_sendevt);

        memset(&i_recvevt, 0, sizeof(IPCEvent));
        gettimeofday(&time_start, NULL);
        while (tp_server.Read(i_recvevt) <= 0 || i_recvevt.EventType != IPC_COMMAND_ACK) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 2) {
                printf("myplayer progress close failed!\n");
                break;
            }
        }
        #endif
    } else {
        printf("my_player is not start!\n");
        fprintf(stderr, "Error：%s\n", strerror(errno));
    }

    #if USE_POPEN
    if (i_recvevt.EventType == IPC_COMMAND_DESTORY) {
        printf("myplayer progress destory done!\n");
    }

    if (tp_shm_addr) {
        //把共享内存从当前进程中分离
        ret = shmdt((void *)tp_shm_addr);
        if (ret < 0) {
            fprintf(stderr, "shmdt failed\n");
        }

        //删除共享内存
        ret = shmctl(tp_shm_id, IPC_RMID, NULL);
        if(ret < 0) {
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        }
    }
    tp_shm_addr = NULL;
    tp_shm_id = 0;

    if (tp_fd) {
        pclose(tp_fd);
        tp_fd = NULL;
    }

    tp_server.Term();
    tp_client.Term();
    system("rm -rf /appconfigs/server_input");
    printf("remove server_input file\n");
    #endif
#else
    tp_player_close();
#endif
    g_exit = true;
    if (idle_tid) {
        pthread_join(idle_tid, NULL);
        idle_tid = 0;
    }

    printf("tp_player_close done!\n");
#endif
    return false;
}

