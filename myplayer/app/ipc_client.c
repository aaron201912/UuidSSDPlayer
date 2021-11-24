#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>

#include "ipc_common.h"
#include "interface.h"

static int client_fd;
static bool g_exit = false;
static bool g_player_status = false;
static ipc_msg_t g_recvmsg;

int ipc_client_send(ipc_msg_t *pmsg, uint8_t command)
{
    int ret;;

    ipc_log(IPC_DEBUG, "ipc_client_send cmd[%d]!\n", command);

    pmsg->cmd = (pmsg->cmd != command) ? command : pmsg->cmd;
    ret = send(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_NOSIGNAL);
    if (ret <= 0) {
        perror("CLIENT SEND");
        ipc_log(IPC_ERROR, "send cmd[%d] to server failed!\n", pmsg->cmd);
        return ret;
    }

    return ret;
}

int ipc_client_recv(ipc_msg_t *pmsg)
{
    int ret;;

    ret = recv(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_WAITALL | MSG_NOSIGNAL);
    if (ret <= 0) {
        perror("CLIENT RECV");
        ipc_log(IPC_ERROR, "recv data from server failed!\n");
        return ret;
    }

    ipc_log(IPC_DEBUG, "ipc_client_recv cmd[%d]!\n", pmsg->cmd);

    return ret;
}

static void * ipc_client_thread(void *arg)
{
    int ret;
    uint8_t server_cmd;
    ipc_msg_t *recvmsg = (ipc_msg_t *)arg;

    while (!g_exit)
    {
        ret = ipc_client_recv(recvmsg);
        if (ret <= 0) {
            g_exit = true;
            break;
        }

        server_cmd = recvmsg->cmd;
        switch (recvmsg->cmd)
        {
            case PLAYER_OPEN :
            case PLAYER_CREATE : {
                printf("try to open : %s, windows x/y/w/h=[%d %d %d %d]\n", recvmsg->url, recvmsg->window.x, recvmsg->window.y, recvmsg->window.width, recvmsg->window.height);
                mm_player_set_opts("audio_device", "", recvmsg->opts.audio_dev);
                mm_player_set_opts("audio_layout", "", recvmsg->opts.audio_layout);
                mm_player_set_opts("video_rotate", "", recvmsg->opts.video_rotate);
                mm_player_set_opts("video_only", "", recvmsg->opts.video_only);
                mm_player_set_opts("video_ratio", "", recvmsg->opts.video_ratio);
                mm_player_set_opts("enable_scaler", "", recvmsg->opts.enable_scaler);
                mm_player_set_opts("resolution", recvmsg->opts.resolution, 0);
                mm_player_set_opts("play_mode", "", recvmsg->opts.play_mode);
                ret = mm_player_open(recvmsg->url, recvmsg->window.x, recvmsg->window.y, recvmsg->window.width, recvmsg->window.height);
                if (ret >= 0) {
                    ipc_client_send(recvmsg, PLAYER_ACK);
                    g_player_status = true;
                } else {
                    ipc_client_send(recvmsg, PLAYER_ACK);
                    ipc_client_send(recvmsg, PLAYER_ERROR);
                    g_exit = true;
                }
            } break;

            case PLAYER_CLOSE :
            case PLAYER_DESTORY : {
                mm_player_close();
                ipc_client_send(recvmsg, PLAYER_ACK);
                g_player_status = false;
                g_exit = (server_cmd == PLAYER_DESTORY) ? true : false;
                continue;
            } break;

            case PLAYER_POSITION : {
                mm_player_getposition(&(recvmsg->position));
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_DURATION : {
                mm_player_getduration(&(recvmsg->duration));
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_SEEK2TIME : {
                mm_player_seek2time(recvmsg->seektime);
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_PAUSE : {
                mm_player_pause();
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_RESUME : {
                mm_player_resume();
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_VOLUMN : {
                mm_player_set_volumn(recvmsg->volumn);
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_MUTE : {
                mm_player_set_mute(recvmsg->mute);
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_WINDOW : {
                mm_player_set_window(recvmsg->window.x, recvmsg->window.y, recvmsg->window.width, recvmsg->window.height);
                ipc_client_send(recvmsg, PLAYER_ACK);
            } break;

            case PLAYER_LINKTEST : {
                ipc_client_send(recvmsg, PLAYER_ACK);
            }

            default : break;
        }
    }

    ipc_log(IPC_DEBUG, "ipc_client_thread exit!\n");

    return NULL;
}

int main(int argc, char *argv[])
{
    int ret;
    struct sockaddr_un srv_addr;
    pthread_t clt_tid;
    char ipc_link[128];

    struct timeval timenow, timeout;
    struct timespec outtime;
    pthread_mutex_t wait_mutex;
    pthread_cond_t  wait_cond;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(client_fd < 0) {
        ipc_log(IPC_ERROR, "create client socket failed!\n");
        return -1;
    }

    memset(ipc_link, '\0', 128);
    if (!argv[1]) {
        strcpy(ipc_link, UNIX_DOMAIN);
    } else {
        strcpy(ipc_link, argv[1]);
    }
    ipc_log(IPC_DEBUG, "open client link filel: %s\n", ipc_link);

    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, ipc_link);

    timeout.tv_sec  = 3;
    timeout.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

    ret = connect(client_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        perror("CONNECT");
        close(client_fd);
        ipc_log(IPC_ERROR, "client connect to server timeout!\n");
        return -1;
    }

    ipc_log(IPC_DEBUG, "client connect to server success!\n");

    ret = pthread_cond_init(&wait_cond,NULL);
    if (ret != 0) {
        ipc_log(IPC_ERROR, "pthread_cond_init failed!\n");
        close(client_fd);
        return -1;
    }
    ret = pthread_mutex_init(&wait_mutex, NULL);
    if (ret != 0) {
        ipc_log(IPC_ERROR, "pthread_mutex_init failed!\n");
        pthread_cond_destroy(&wait_cond);
        close(client_fd);
        return -1;
    }

    memset(&g_recvmsg, 0, sizeof(ipc_msg_t));
    ret = pthread_create(&clt_tid, NULL, ipc_client_thread, (void *)&g_recvmsg);
    if (ret != 0) {
        ipc_log(IPC_ERROR, "ipc_client_thread create failed!\n");
        close(client_fd);
        return -1;
    }

    while (!g_exit)
    {
        pthread_mutex_lock(&wait_mutex);
        gettimeofday(&timenow, NULL);
        outtime.tv_sec  = timenow.tv_sec + 1;
        outtime.tv_nsec = timenow.tv_usec * 1000;
        ret = pthread_cond_timedwait(&wait_cond, &wait_mutex, &outtime);
        pthread_mutex_unlock(&wait_mutex);

        if (g_player_status)
        {
            ret = mm_player_getposition(&(g_recvmsg.position));
            if (ret >= 0) {
                ipc_client_send(&g_recvmsg, PLAYER_POSITION);
            }

            ret = mm_player_get_status();
            if (ret > 0) {
                if (ret & AV_PLAY_ERROR){
                    g_recvmsg.flags = ret;
                    ipc_client_send(&g_recvmsg, PLAYER_ERROR);
                    ipc_log(IPC_DEBUG, "player error status: 0x%x\n", ret);
                } else if ((ret & AV_PLAY_COMPLETE) == AV_PLAY_COMPLETE) {
                    g_recvmsg.flags = AV_PLAY_COMPLETE;
                    ipc_client_send(&g_recvmsg, PLAYER_COMPLETE);
                }
            }
        }
    }

    pthread_join(clt_tid, NULL);

    pthread_cond_destroy(&wait_cond);
    pthread_mutex_destroy(&wait_mutex);

    close(client_fd);

    ipc_log(IPC_DEBUG, "##### exit client #####\n");

    return 0;
}


