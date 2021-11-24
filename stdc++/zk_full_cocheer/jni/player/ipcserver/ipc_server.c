#include <stdio.h>
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

static bool g_exit = false;
static bool g_server_is_open = false;
static bool g_recv_wait = false;

pthread_mutex_t wait_mutex;
pthread_cond_t  wait_cond;

FILE *handler = NULL;
int server_fd = -1, client_fd = -1;
char client_name[128], link_file[128];

int ipc_server_recv(ipc_msg_t *pmsg)
{
    int ret;
    //struct timeval timeout;

    if (!g_server_is_open) {
        ipc_log(IPC_ERROR, "ipc_server_recv error!\n");
        return -1;
    }

    g_recv_wait = true;

    //timeout.tv_sec  = 1;
    //timeout.tv_usec = 0;
    //setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    //ret = recv(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_WAITALL | MSG_NOSIGNAL);
    ret = recv(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_NOSIGNAL);
    if (ret <= 0) {
        perror("SERVER RECV");
        //pmsg->cmd = PLAYER_LINKTEST;
        //ret = send(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_NOSIGNAL);
        //if (ret <= 0) {
        //    ipc_log(IPC_ERROR, "send linktest to client failed!\n");
        //}
        ipc_log(IPC_ERROR, "recv cmd[%d] from client failed!\n", pmsg->cmd);
    } else {
        ipc_log(IPC_DEBUG, "ipc_server_recv cmd[%d]!\n", pmsg->cmd);
    }

    if (pmsg->cmd == PLAYER_ACK) {
        pthread_cond_signal(&wait_cond);
    }

    g_recv_wait = false;

    return ret;
}

int ipc_server_send(ipc_msg_t *pmsg, uint8_t command)
{
    int ret;;

    if (!g_server_is_open) {
        ipc_log(IPC_ERROR, "ipc_server_send error!\n");
        return -1;
    }

    ipc_log(IPC_DEBUG, "ipc_server_send cmd[%d]!\n", command);

    pmsg->cmd = (pmsg->cmd != command) ? command : pmsg->cmd;
    ret = send(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_NOSIGNAL);
    if (ret <= 0) {
        ipc_log(IPC_ERROR, "send cmd[%d] to client failed!\n", pmsg->cmd);
        return ret;
    }

    return ret;
}

int ipc_server_wait(ipc_msg_t *pmsg, int time_ms)
{
    int ret = -1;
    uint64_t time;
    struct timeval timenow, timewait;
    struct timespec outtime;

    if (!g_server_is_open) {
        ipc_log(IPC_ERROR, "ipc_server_wait error!\n");
        return -1;
    }

    gettimeofday(&timenow, NULL);

    if (g_recv_wait) {
        if (pmsg->cmd != PLAYER_ACK) {
            gettimeofday(&timenow, NULL);
            outtime.tv_sec  = timenow.tv_sec + (time_ms / 1000);
            outtime.tv_nsec = timenow.tv_usec * 1000 + (time_ms % 1000) * 1000 * 1000;
            ret = !pthread_cond_timedwait(&wait_cond, &wait_mutex, &outtime);
            if (!ret) {
                perror("TIMEWAIT");
                ipc_log(IPC_ERROR, "pthread_cond_timedwait cmd[%d] timeout!\n", pmsg->cmd);
            }
        }
    } else {
        if (pmsg->cmd != PLAYER_ACK) {
            timewait.tv_sec  = time_ms / 1000;
            timewait.tv_usec = (time_ms % 1000) * 1000;
            setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timewait, sizeof(struct timeval));
            ret = recv(client_fd, (void *)pmsg, sizeof(ipc_msg_t), MSG_WAITALL | MSG_NOSIGNAL);
            if (ret <= 0) {
                perror("SERVER RECV");
                ipc_log(IPC_ERROR, "ipc_server_wait cmd[%d] timeout!\n", pmsg->cmd);
            } else {
                ipc_log(IPC_DEBUG, "ipc_server_wait cmd[%d]!\n", pmsg->cmd);
            }
        }
    }

    gettimeofday(&timewait, NULL);
    time = (timewait.tv_sec - timenow.tv_sec) * 1000000 + (timewait.tv_usec - timenow.tv_usec);
    ipc_log(IPC_DEBUG, "time of recv ack from client: %lld, ret: %d\n", time, ret);

    return ret;
}

int ipc_server_open(char *app_name, char *link_file)
{
    int ret;
    struct sockaddr_un srv_addr;
    struct sockaddr_un clt_addr;
    socklen_t len;
    struct timeval timeout;
    char ipc_link[128];

    if (g_server_is_open) {
        ipc_log(IPC_ERROR, "ipc server has been open!\n");
        return -1;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_fd < 0) {
        ipc_log(IPC_ERROR, "creat server communication socket failed!\n");
        return -1;
    }

    memset(ipc_link, '\0', 128);
    if (link_file) {
        strcpy(ipc_link, link_file);
    } else {
        strcpy(ipc_link, UNIX_DOMAIN);
    }

    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, ipc_link);
    unlink(ipc_link);

    ret = bind(server_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if(ret < 0) {
        perror("BIND");
        ipc_log(IPC_ERROR, "bind server_fd to socket failed!\n");
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    ret = listen(server_fd, 1);
    if(ret<0){
        perror("LISTEN");
        ipc_log(IPC_ERROR, "cannot listen server_fd!\n");
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    if (app_name) {
        printf("try to open app: %s, link to %s\n", app_name, ipc_link);
        handler = popen(app_name, "w");
    } else {
        printf("try to open app: %s, link to %s\n", CLIENT_NAME, ipc_link);
        handler = popen(CLIENT_NAME, "w");
    }

    if (!handler) {
        ipc_log(IPC_ERROR, "open server failed!\n");
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    timeout.tv_sec  = 3;
    timeout.tv_usec = 0;
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));

    len = sizeof(socklen_t);
    client_fd = accept(server_fd, (struct sockaddr*)&clt_addr, &len);
    if(client_fd < 0){
        perror("ACCEPT");
        ipc_log(IPC_ERROR, "cannot accept requst from client!\n");
        pclose(handler);
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    ret = pthread_cond_init(&wait_cond, NULL);
    if (ret != 0) {
        ipc_log(IPC_ERROR, "pthread_cond_init failed!\n");
        pclose(handler);
        close(client_fd);
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    ret = pthread_mutex_init(&wait_mutex, NULL);
    if (ret != 0) {
        ipc_log(IPC_ERROR, "pthread_mutex_init failed!\n");
        pthread_cond_destroy(&wait_cond);
        pclose(handler);
        close(client_fd);
        close(server_fd);
        unlink(ipc_link);
        return -1;
    }

    g_server_is_open = true;

    return client_fd;
}

int ipc_server_close(char *link_file)
{
    char ipc_link[128];

    if (!g_server_is_open) {
        ipc_log(IPC_ERROR, "ipc server has been closed!\n");
        return -1;
    }

    memset(ipc_link, '\0', 128);
    if (link_file) {
        strcpy(ipc_link, link_file);
    } else {
        strcpy(ipc_link, UNIX_DOMAIN);
    }

    pclose(handler);
    close(client_fd);
    close(server_fd);
    unlink(ipc_link);

    pthread_cond_destroy(&wait_cond);
    pthread_mutex_destroy(&wait_mutex);

    g_server_is_open = false;

    ipc_log(IPC_DEBUG, "ipc_server_close done!\n");

    return 0;
}

int ipc_server_running(void)
{
    return g_server_is_open;
}

