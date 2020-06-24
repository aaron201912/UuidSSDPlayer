#include "broadcastserver.h"

#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string.h>

#include "include/utils/Log.h"

#define BROADCAST_PORT 2333

#define PACKAGE_SIZE 1024

BroadcastServer::BroadcastServer() {
    is_run_ = false;
    handler_ = NULL;
    pack_index_ = -1;

    host_info_list_ = NULL;
    sock_ = -1;
    pack_ = NULL;
    callback_ = NULL;
    file_type_flag_ = -1;
}

BroadcastServer::~BroadcastServer() {
    Clear();
}

int BroadcastServer::Start() {
#if 1
    int sockListen;
    if ((sockListen = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        LOGD("socket fail\n");
        return -1;
    }
    int set = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(BROADCAST_PORT);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    // 必须绑定，否则无法监听
    if (bind(sockListen, (struct sockaddr *) &recvAddr, sizeof(struct sockaddr))
            == -1) {
        LOGD("bind fail\n");
        return -1;
    }

#if 1
    struct timeval tv_out;
    tv_out.tv_sec = 3;
    tv_out.tv_usec = 0;
    setsockopt(sockListen, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
#endif

    int recvbytes;
    unsigned char recvbuf[PACKAGE_SIZE * 2];
    int addrLen = sizeof(struct sockaddr_in);

    LOGD("receive from...");

    while (is_run_) {
        recvbytes = recvfrom(sockListen, recvbuf, sizeof(recvbuf), 0,
                (struct sockaddr *) &recvAddr, (socklen_t *)  &addrLen);
        if (-1 == recvbytes && (errno == EWOULDBLOCK)) {
            //超时
            LOGD("receive out time");
        } else if (recvbytes != -1) {
            char* ip = (char *)inet_ntoa(recvAddr.sin_addr);
            int port = htons(recvAddr.sin_port);
            LOGD("IP:%s", ip);
            LOGD("Port:%d", port);

#if 0
            SetMessageCallback([] (const char* msg) -> void {
                 LOGD("%s", msg);
            });
#endif
            char port_str[12] = {0};
            snprintf(port_str, sizeof(port_str), "%d", 2334);
            Receive(ip, port_str);
        } else {
            LOGD("receive fail\n");
        }
    }
    close(sockListen);
    return 0;
    LOGD("end");
#else
    int sockListen;
    if((sockListen = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        LOGD("socket fail\n");
        return -1;
    }
    int set = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(2333);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    // 必须绑定，否则无法监听
    if(bind(sockListen, (struct sockaddr *)&recvAddr, sizeof(struct sockaddr)) == -1) {
        LOGD("bind fail\n");
        return -1;
    }
    int recvbytes;
    char recvbuf[128];
    int addrLen = sizeof(struct sockaddr_in);
    LOGD("recvfrom...");

    struct timeval tv_out;
    tv_out.tv_sec=3;
    tv_out.tv_usec=0;
    setsockopt(sockListen,SOL_SOCKET,SO_RCVTIMEO,&tv_out,sizeof(tv_out));
    recvbytes = recvfrom(sockListen, recvbuf, 128, 0, (struct sockaddr *)&recvAddr, &addrLen);
    if (-1 == recvbytes && (errno == EWOULDBLOCK)) {
        //超时
        LOGD("recvfrom out time");
    } else if(recvbytes != -1) {
        recvbuf[recvbytes] = '\0';
        LOGD("receive a broadCast messgse:%s\n", recvbuf);
    } else {
        LOGD("recvfrom fail\n");
    }
    close(sockListen);
    LOGD("end");
#endif
}

bool BroadcastServer::threadLoop() {
    Start();
    return false;
}

void BroadcastServer::Stop() {
    is_run_ = false;
}

bool BroadcastServer::run(const char* name) {
    is_run_ = true;
    return Thread::run(name);
}

void BroadcastServer::SetImageHandler(void (*func)(const char*)) {
    handler_ = func;
}

void BroadcastServer::WriteImage(const char* file_name) {
    FILE* fp = NULL;
    fp = fopen(file_name, "wb");
    if (!fp) {
        LOGD("open file error, %s", file_name);
        return;
    }

    for (int i = 0; i < cache_.size(); ++i) {
        int r = fwrite(cache_.at(i).data, cache_.at(i).len, 1, fp);
        if (r != 1) {
            LOGD("write error");
        }
        delete[] cache_.at(i).data;
    }
    cache_.clear();
    fflush(fp);
    fclose(fp);
}

void BroadcastServer::Add(const unsigned char* data, int len) {
    unsigned char* p = new unsigned char[len];
    if (p) {
        memcpy(p, data, len);
    }
    Array array = {p, len};
    cache_.push_back(array);
}

void BroadcastServer::Clear() {
    for (int i = 0; i < cache_.size(); ++i) {
        delete[] cache_.at(i).data;
    }
    cache_.clear();
}

bool BroadcastServer::Connect(const char* ip, const char* port) {
    int status;
    int socketfd = -1;
    struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
    struct addrinfo * host_info_list = NULL;

    memset(&host_info, 0, sizeof host_info);

    host_info.ai_family = AF_UNSPEC;   // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

    HandleMessage("getaddrinfo");
    status = getaddrinfo(ip, port, &host_info, &host_info_list);
    if (status != 0) {
        LOGD("ERROR: Hostname could not be resolved");
        HandleMessage("ERROR: Hostname could not be resolved");
        return false;
    }
    HandleMessage("socket init");


    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
            host_info_list->ai_protocol);
    if (socketfd == -1) {
        LOGE("ERROR: Could not open socket");
        HandleMessage("ERROR: Could not open socket");
        freeaddrinfo(host_info_list);
        return false;
    }

#if 0
    status = ::Connect(socketfd, host_info_list_->ai_addr,
            host_info_list_->ai_addrlen);
#else
    status = NonBlockConnect(socketfd, host_info_list->ai_addr,
            host_info_list->ai_addrlen, 5);
#endif
    if (status == -1) {
        LOGE("ERROR: Could not connect");
        HandleMessage("ERROR: Could not connect");
        freeaddrinfo(host_info_list);
        close(socketfd);
        return false;
    }
    sock_ = socketfd;
    host_info_list_ = host_info_list;
    LOGD("connect %s success!\n", ip);
    HandleMessage("connect success!");
    return true;
}

bool BroadcastServer::Disconnect() {
    LOGD("SocketClient close socket...\n");
    if (sock_ >= 0) {
        freeaddrinfo(host_info_list_);
        host_info_list_ = NULL;
        close(sock_);
        sock_ = -1;
    }
    if (pack_ != NULL) {
        delete[] pack_;
        pack_ = NULL;
    }
    return true;
}

int BroadcastServer::NonBlockConnect(int fd, const struct sockaddr* ai_addr,
        socklen_t ai_addrlen, int out_time_seconds) {
    /*设置套接字为非阻塞*/
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        LOGD("Get flags error:%s\n", strerror(errno));
        return -1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        LOGD("Set flags error:%s\n", strerror(errno));
        return -1;
    }

    fd_set fdr, fdw;
    struct timeval timeout;
    int err = 0;
    int errlen = sizeof(err);
    /*阻塞情况下linux系统默认超时时间为75s*/
    int rc = ::connect(fd, ai_addr, ai_addrlen);
    if (rc != 0) {
        if (errno == EINPROGRESS) {
            LOGD("Doing connection.\n");
            HandleMessage("connecting ...\n");
            /*正在处理连接*/
            FD_ZERO(&fdr);
            FD_ZERO(&fdw);
            FD_SET(fd, &fdr);
            FD_SET(fd, &fdw);
            timeout.tv_sec = out_time_seconds;
            timeout.tv_usec = 0;
            rc = select(fd + 1, &fdr, &fdw, NULL, &timeout);
            LOGD("rc is: %d\n", rc);
            /*select调用失败*/
            if (rc < 0) {
                LOGD("connect error:%s\n", strerror(errno));
                return -1;
            }

            /*连接超时*/
            if (rc == 0) {
                LOGD("Connect timeout.\n");
                HandleMessage("Connect timeout.\n");
                return -1;
            }
            /*[1] 当连接成功建立时，描述符变成可写,rc=1*/
            if (rc == 1 && FD_ISSET(fd, &fdw)) {
                LOGD("Connect success\n");
                HandleMessage("Connect success\n");
                return 0;
            }
            /*[2] 当连接建立遇到错误时，描述符变为即可读，也可写，rc=2 遇到这种情况，可调用getsockopt函数*/
            if (rc == 2) {
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, (socklen_t*)&errlen) == -1) {
                    LOGD("getsockopt(SO_ERROR): %s", strerror(errno));
                    return -1;
                }

                if (err) {
                    errno = err;
                    LOGD("connect error:%s\n", strerror(errno));
                    return -1;
                }
            }
        }
        LOGD("connect failed, error:%s.\n", strerror(errno));
        return -1;
    }
    return rc;
}

size_t BroadcastServer::socket_receive(int sockfd) {
    while (true) {
        unsigned char b;
        int size = recv(sockfd, &b, sizeof(b), 0);
        if ((size < 0) && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) {
            sleep(100);
            continue;
        } else if (size == 0) {
            LOGD("closed");
            return 0;
        } else if (size < 0) {
            LOGD("exception");
            return -1;
        } else if (size == 1) {
            //根据b 字节 打开文件 判断图片类型
            file_type_flag_ = b;
            LOGD(" file type flag = %02x", file_type_flag_);
            break;
        }
    }

    unsigned char buffer[5 + 4 + 4 + 4 + PACKAGE_SIZE] = {0};
    int read_count = 0;
    while (true) {
        read_count = recv(sockfd, buffer, sizeof(buffer), 0);
        if ((read_count < 0)
                && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) {
            LOGD("read nothing");
            sleep(50);
            continue;
        } else if (read_count == 0) {
            LOGD("kick off!");
            return 0;
        } else if (read_count < 0) {
            return -1;
        }
        LOGD("read len: %d", read_count);
        Add(buffer, read_count);
    }
    return 0;
}

ssize_t socket_send(int sockfd, const unsigned char* buffer, size_t buflen)
{
    ssize_t tmp;
    size_t total = buflen;
    const unsigned char *p = buffer;

    while (1) {
        tmp = send(sockfd, p, total, 0);
        if (tmp < 0) {
            // 当send收到信号时,可以继续写,但这里返回-1.
            if (errno == EINTR)
                return -1;

            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            if (errno == EAGAIN) {
                usleep(1000);
                continue;
            }

            return -1;
        }

        if ((size_t) tmp == total)
            return buflen;

        total -= tmp;
        p += tmp;
    }

    return tmp;
}

void BroadcastServer::SetMessageCallback(void (*callback)(const char*)) {
    callback_ = callback;
}

//tcp 连接ip， 接收图片
bool BroadcastServer::Receive(const char* ip, const char* port) {
    if (!Connect(ip, port)) {
        return false;
    }
    socket_receive(sock_);
    if (cache_.size() > 0) {
        char image_name[32] = { 0 };
        snprintf(image_name, sizeof(image_name), "/mnt/extsd/remote_image.%s",
                GetFileSuffix());
        WriteImage(image_name);
        if (handler_) {
            handler_(image_name);
        }
        Clear();
    }
    Disconnect();
    return true;
}

const char* BroadcastServer::GetFileSuffix() {
    switch (file_type_flag_) {
        case 0xA1:
            return "jpg";
        case 0xA2:
            return "png";
    }
    return "jpg";
}

void BroadcastServer::HandleMessage(const char* msg) {
    if (callback_) {
        callback_(msg);
    }
}
