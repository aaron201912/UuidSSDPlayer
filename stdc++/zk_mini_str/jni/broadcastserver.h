#ifndef JNI_BROADCASTSERVER_H_
#define JNI_BROADCASTSERVER_H_

#include <sys/socket.h>
#include "system/Thread.h"

#include <vector>

struct Array{
    unsigned char* data;
    int len;
};

class BroadcastServer : public Thread {
public:
    BroadcastServer();
    virtual ~BroadcastServer();


    bool run(const char *name = 0);
    void Stop();
    void SetImageHandler(void (*func)(const char*));

    bool Receive(const char* ip, const char* port);

    void SetMessageCallback(void (*callback)(const char*));
protected:
    virtual bool threadLoop();

    int Start();
    void WriteImage(const char* file_name);

    void Add(const unsigned char* data, int len);
    void Clear();
    const char* GetFileSuffix();


protected:
    bool Connect(const char* ip, const char* port);
    bool Disconnect();
    int NonBlockConnect(int, const struct sockaddr *, socklen_t, int);
    size_t socket_receive(int socket);
    void HandleMessage(const char* msg);

private:
    int sock_;
    struct addrinfo *host_info_list_;
    unsigned char* pack_;

    void (*callback_)(const char*);

private:
    bool is_run_;
    void (*handler_)(const char*);
    std::vector<struct Array> cache_;

    int pack_index_;
    int file_type_flag_;
};

#endif /* JNI_BROADCASTSERVER_H_ */
