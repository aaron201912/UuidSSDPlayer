#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "interface.h"
#include "player.h"


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
  IPC_COMMAND_EXIT
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
            m_fd = open(m_file.c_str(), O_WRONLY | O_NONBLOCK);
            printf("IPCOutput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
    }

    int Send(const IPCEvent& evt) {
        if (m_fd >= 0) {
            ret = write(m_fd, &evt, sizeof(IPCEvent));
            //printf("write %d byte to %s\n", ret, m_file.c_str());
        } else {
            ret = -1;
            //printf("%s can't be writed!\n", m_file.c_str());
        }
        return ret;
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
            m_fd = open(m_file.c_str(), O_RDWR | O_NONBLOCK);
            printf("IPCInput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    int Read(IPCEvent& evt) {
        if (m_fd >= 0) {
            return read(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("read %s failed!\n", m_fifo.Path().c_str());
        return 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
    }

private:
    int m_fd;
    std::string m_file;
    IPCNameFifo m_fifo;
};


struct timeval time_now, time_last;
static bool g_playing = false;

int main(int argc, char *argv[]) 
{
    int ret;
    IPCEvent recvevt;
    IPCEvent sendevt;
    bool bExit = false;

    printf("##### Welcome to MyPlayer! #####\n");

    IPCInput i_client(CLT_IPC);
    if(!i_client.Init())
    {
        printf("create ipc input fail\n");
        return -1;
    }

    IPCOutput o_server(SVC_IPC);

    while(!bExit)
    {
        memset(&recvevt, 0, sizeof(IPCEvent));
        if(i_client.Read(recvevt) > 0)
        {
            printf("get event type = [%d]\n", recvevt.EventType);
            switch(recvevt.EventType)
            {
                case IPC_COMMAND_OPEN:
                {
                    if(!o_server.Init()) {
                        printf("Main Process Not start!!!\n");
                        //i_client.Term();
                        //return -1;
                        break;
                    }
                    printf("start to play %s, windows = [%d %d %d %d]\n", recvevt.stPlData.filePath, recvevt.stPlData.x,
                    recvevt.stPlData.y, recvevt.stPlData.width, recvevt.stPlData.height);
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
                    my_player_close();
                    g_playing = false;
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
                        //i_client.Term();
                        //return -1;
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
                        //i_client.Term();
                        //return -1;
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

                case IPC_COMMAND_EXIT:
                    my_player_close();
                    g_playing = false;
                    bExit = true;
                    av_log(NULL, AV_LOG_WARNING, "##### MyPlayer Exit #####\n");
                break;

                default: break;
            }
        }

        // 每秒更新一次播放时间发送给UI
        gettimeofday(&time_now, NULL);
        if (time_now.tv_sec - time_last.tv_sec >= 1 && g_playing) {
            double position;
            if(!o_server.Init()) {
                printf("Main Process Not start!!!\n");
                continue;
            }

            int ret = my_player_getposition(&position);
            if (ret >= 0) {
                memset(&sendevt,0,sizeof(IPCEvent));
                sendevt.EventType = IPC_COMMAND_GET_POSITION;
                sendevt.stPlData.misc = position;
                o_server.Send(sendevt);
                time_last.tv_sec = time_now.tv_sec;
                //av_log(NULL, AV_LOG_WARNING, "send current position time[%0.3lf]\n", sendevt.stPlData.misc);
            }
        }

        // 异常处理或者播放完成
        if (g_myplayer && g_playing) {
            if (g_myplayer->play_status > 0) {
                if(!o_server.Init()) {
                    printf("Main Process Not start!!!\n");
                    continue;
                }
                memset(&sendevt,0,sizeof(IPCEvent));
                sendevt.EventType = IPC_COMMAND_COMPLETE;
                sendevt.stPlData.status = (g_myplayer->audio_complete | g_myplayer->video_complete);
                o_server.Send(sendevt);
                av_log(NULL, AV_LOG_INFO, "my_player has played complete!\n");
            } else if (g_myplayer->play_status < 0) {
                if(!o_server.Init()) {
                    printf("Main Process Not start!!!\n");
                    continue;
                }
                memset(&sendevt,0,sizeof(IPCEvent));
                sendevt.EventType = IPC_COMMAND_ERROR;
                sendevt.stPlData.status = g_myplayer->play_status;
                o_server.Send(sendevt);
                av_log(NULL, AV_LOG_ERROR, "my_player occur error in playing!\n");
            }
            g_myplayer->play_status = 0;
        }

        usleep(100 * 1000);
    }

    printf("Player is exit: %d\n", bExit);

    //o_server.Term();
    i_client.Term();

    return 0;
}

