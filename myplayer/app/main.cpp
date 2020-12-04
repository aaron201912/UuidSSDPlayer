#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h> 
#include <sys/ipc.h>
#include <sys/shm.h>

#include "interface.h"
#include "player.h"

struct shared_use_st
{
    int  written;    //作为一个标志，非0：表示可读，0表示可写
    bool flag;
};

struct shared_use_st *shm_addr;
int shm_id;

#define CLT_IPC "/appconfigs/client_input"
#define SVC_IPC "/appconfigs/server_input"


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
            printf("IPCOutput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("[%s %d]%s term!\n", __FILE__, __LINE__, m_file.c_str());
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
            printf("IPCInput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    int Read(IPCEvent& evt) {
        if (m_fd >= 0) {
            return read(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("read %s failed!\n", m_fifo.Path().c_str());
        return -1;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("[%s %d]%s term!\n", __FILE__, __LINE__, m_fifo.Path().c_str());
    }

private:
    int m_fd;
    std::string m_file;
    IPCNameFifo m_fifo;
};

#define USE_POPEN       1
#define PANT_TIME       5

struct timeval time_now, time_last;
struct timeval pant_start, pant_wait, pant_end;
static bool g_playing = false;
static bool g_pantflag = false;
static bool g_ipc_error = false;
extern int errno;

int main(int argc, char *argv[]) 
{
    int ret;
    IPCEvent recvevt;
    IPCEvent sendevt;
    bool bExit = false;
    void *shm = NULL;

    printf("##### Welcome to MyPlayer! #####\n");
    signal(SIGPIPE, SIG_IGN);

#if USE_POPEN
    //创建共享内存
    shm_id = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if(shm_id < 0)
    {
        fprintf(stderr, "shmget failed\n");
        return -1;
    }

    //将共享内存连接到当前进程的地址空间
    shm = shmat(shm_id, (void*)NULL, 0);
    if(shm < 0)
    {
        fprintf(stderr, "shmat failed\n");
        return -1;
    }

    shm_addr = (struct shared_use_st *)shm;
    printf("shared memory attached at %x\n", (int)shm);
#endif

    IPCInput i_client(CLT_IPC);
    if(!i_client.Init()) {
        printf("create myplayer ipc input fail\n");
        fprintf(stderr, "Error：%s\n", strerror(errno));
        return -1;
    }

    IPCOutput o_server(SVC_IPC);

#if USE_POPEN
    if(!o_server.Init()) {
        printf("Main Process Not start!!!\n");
        fprintf(stderr, "Error：%s\n", strerror(errno));
        return -1;
    }

    memset(&sendevt,0,sizeof(IPCEvent));
    sendevt.EventType = IPC_COMMAND_CREATE;
    o_server.Send(sendevt);
#endif

    gettimeofday(&pant_start, NULL);
    gettimeofday(&pant_wait, NULL);

    while(!bExit)
    {
        memset(&recvevt, 0, sizeof(IPCEvent));
        if(i_client.Read(recvevt) > 0)
        {
            av_log(NULL, AV_LOG_VERBOSE, "get event type = [%d]\n", recvevt.EventType);
            switch(recvevt.EventType)
            {
                case IPC_COMMAND_OPEN:
                {
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        g_ipc_error = true;
                        break;
                    }

                    printf("start to play %s, windows = [%d %d %d %d]\n", recvevt.stPlData.filePath, recvevt.stPlData.x,
                    recvevt.stPlData.y, recvevt.stPlData.width, recvevt.stPlData.height);

                    g_opts.audio_only = recvevt.stPlData.audio_only;
                    g_opts.video_only = recvevt.stPlData.video_only;
                    g_opts.play_mode  = recvevt.stPlData.play_mode;
                    printf("player options value = [%d %d %d]\n", g_opts.audio_only, g_opts.video_only, g_opts.play_mode);

                    my_player_set_aodev(recvevt.stPlData.aodev);
                    my_player_set_rotate(recvevt.stPlData.rotate);
                    ret = my_player_open(recvevt.stPlData.filePath, recvevt.stPlData.x, recvevt.stPlData.y, recvevt.stPlData.width, recvevt.stPlData.height);
                    if (ret < 0) {
                        memset(&sendevt,0,sizeof(IPCEvent));
                        sendevt.EventType = IPC_COMMAND_ERROR;
                        sendevt.stPlData.status = ret;
                        o_server.Send(sendevt);
                        g_playing = false;
                    } else {
                        memset(&sendevt,0,sizeof(IPCEvent));
                        sendevt.EventType = IPC_COMMAND_ACK;
                        o_server.Send(sendevt);
                        g_playing = true;
                    }
                }
                break;

                case IPC_COMMAND_CLOSE:
                {
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        g_ipc_error = true;
                        break;
                    }

                    my_player_close();

                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_ACK;
                    o_server.Send(sendevt);
                    g_playing  = false;
                    g_pantflag = false;
                    av_log(NULL, AV_LOG_INFO, "my_player_close done!\n");
                }
                break;

                case IPC_COMMAND_PAUSE:
                {
                    my_player_pause();
                }
                break;

                case IPC_COMMAND_RESUME:
                {
                    my_player_resume();
                }
                break;

                case IPC_COMMAND_SEEK:
                {
                    my_player_seek(recvevt.stPlData.misc);
                }
                break;

                case IPC_COMMAND_SEEK2TIME:
                {
                    my_player_seek2time(recvevt.stPlData.misc);
                }
                break;

                case IPC_COMMAND_GET_POSITION:
                {
                    double position;
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        g_ipc_error = true;
                        break;
                    }
                    
                    my_player_getposition(&position);

                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_GET_POSITION;
                    sendevt.stPlData.misc = position;
                    o_server.Send(sendevt);
                }
                break;

                case IPC_COMMAND_GET_DURATION:
                {
                    double duration;
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        g_ipc_error = true;
                        break;
                    }
                    
                    my_player_getduration(&duration);

                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_GET_DURATION;
                    sendevt.stPlData.misc = duration;
                    o_server.Send(sendevt);
                }
                break;

                case IPC_COMMAND_SET_VOLUMN:
                    my_player_set_volumn(recvevt.stPlData.volumn);
                break;

                case IPC_COMMAND_SET_MUTE:
                    my_player_set_mute(recvevt.stPlData.mute);
                break;

                case IPC_COMMAND_EXIT: {
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        g_ipc_error = true;
                        break;
                    }

                    my_player_close();

                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_ACK;
                    o_server.Send(sendevt);
                    g_playing  = false;
                    g_pantflag = false;
                    av_log(NULL, AV_LOG_INFO, "my_player_close done!\n");

                    bExit = true;
                    av_log(NULL, AV_LOG_WARNING, "##### MyPlayer Exit #####\n");
                }
                break;

                case IPC_COMMAND_PANT: {
                    g_pantflag = true;
                    gettimeofday(&pant_wait, NULL);
                    //int ack_time = (pant_wait.tv_sec - pant_end.tv_sec) * 1000 + (pant_wait.tv_usec - pant_end.tv_usec) / 1000;
                    //av_log(NULL, AV_LOG_VERBOSE, "receive pant ack signal [%dms]\n", ack_time);
                }
                break;

                default: break;
            }
        }

        // 每秒更新一次播放时间发送给UI
        gettimeofday(&time_now, NULL);
        if (time_now.tv_sec - time_last.tv_sec >= 1 && g_playing) {
            time_last.tv_sec = time_now.tv_sec;
            if(o_server.Init()) {
                double position;
                int ret = my_player_getposition(&position);
                if (ret >= 0) {
                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_GET_POSITION;
                    sendevt.stPlData.misc = position;
                    o_server.Send(sendevt);
                    //av_log(NULL, AV_LOG_WARNING, "send current position time[%0.3lf]\n", sendevt.stPlData.misc);
                }
            }
        }

        // 异常处理或者播放完成
        if (g_myplayer && g_playing) {
            if (g_myplayer->play_status > 0) {
                if(!o_server.Init()) {
                    printf("Main Process Not start!!!\n");
                    g_ipc_error = true;
                } else {
                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_COMPLETE;
                    sendevt.stPlData.status = (g_myplayer->audio_complete | g_myplayer->video_complete);
                    o_server.Send(sendevt);
                    g_myplayer->play_status = 0;
                    av_log(NULL, AV_LOG_INFO, "my_player has played complete!\n");
                }
            } else if (g_myplayer->play_status < 0) {
                if(!o_server.Init()) {
                    printf("Main Process Not start!!!\n");
                    g_ipc_error = true;
                } else {
                    memset(&sendevt,0,sizeof(IPCEvent));
                    sendevt.EventType = IPC_COMMAND_ERROR;
                    sendevt.stPlData.status = g_myplayer->play_status;
                    o_server.Send(sendevt);

                    g_myplayer->play_status = 0;

                    my_player_close();
                    g_playing  = false;
                    g_pantflag = false;

                    bExit = true;
                    av_log(NULL, AV_LOG_ERROR, "my_player occur error in playing!\n");
                    av_log(NULL, AV_LOG_ERROR, "please attemp to exit player!\n");
                }
            }
        }

        //增加心跳包机制
        gettimeofday(&pant_end, NULL);
        if (g_playing && pant_end.tv_sec - pant_start.tv_sec >= PANT_TIME) {
            if(o_server.Init()) {
                memset(&sendevt,0,sizeof(IPCEvent));
                sendevt.EventType = IPC_COMMAND_PANT;
                o_server.Send(sendevt);

                av_log(NULL, AV_LOG_VERBOSE, "my_player send pant signal!\n");
            }
            gettimeofday(&pant_start, NULL);
        }

        if (g_playing && g_pantflag && pant_end.tv_sec - pant_wait.tv_sec >= 2 * PANT_TIME) {
            if(o_server.Init()) {
                memset(&sendevt,0,sizeof(IPCEvent));
                sendevt.EventType = IPC_COMMAND_ERROR;
                sendevt.stPlData.status = -1;
                o_server.Send(sendevt);
            }
            av_log(NULL, AV_LOG_ERROR, "get pant ack signal form main time out!\n");
            g_ipc_error = true;
        }

        /*if (!g_playing && !bExit && pant_end.tv_sec - pant_start.tv_sec >= 3) {
            av_log(NULL, AV_LOG_ERROR, "wake up my_player time out!\n");
            bExit = true;
        }*/

        //进程间通讯异常处理
        if (g_ipc_error) {
#if USE_POPEN
            bExit = true;
#endif
            if (g_playing) {
                my_player_close();
                g_playing = false;
            }
            g_pantflag = false;
        }

        usleep(10 * 1000);
    }

    printf("MyPlayer is exit: %d\n", bExit);

#if USE_POPEN
    if(!o_server.Init()) {
        printf("Main Process Not start!!!\n");
    } else {
        memset(&sendevt,0,sizeof(IPCEvent));
        sendevt.EventType = IPC_COMMAND_DESTORY;
        o_server.Send(sendevt);
        av_log(NULL, AV_LOG_INFO, "send exit signal type = [%d]\n", sendevt.EventType);
    }

    shm_addr->written = 1;
    shm_addr->flag = true;
    shm_addr->written = 0;

    //把共享内存从当前进程中分离
    ret = shmdt((void *)shm_addr);
    if (ret < 0) 
    {
        fprintf(stderr, "shmdt failed\n");
    }
#endif

    return 0;
}

