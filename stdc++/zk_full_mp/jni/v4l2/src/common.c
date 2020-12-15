#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>

#include "common.h"

int new_packet(Packet *pkt, int size)
{
    assert(pkt);

    BufferRef *buf = NULL;

    buf = (BufferRef*)calloc(1, sizeof(*buf));
    if(!buf)
        return -ENOMEM;

    buf->size = size;
    buf->data = (uint8_t*)calloc(size, sizeof(int));
    if(!buf->data) {
        free(buf);
        return -ENOMEM;
    }
    pkt->buf      = buf;
    pkt->data     = buf->data;
    pkt->size     = buf->size;
    return 0;
}

void free_packet(Packet *pkt)
{
    BufferRef *buf = pkt->buf;

    assert(buf->data);
    assert(buf);

    free(buf->data);
    free(buf);

    pkt->data = NULL;
    pkt->buf  = NULL;
    pkt->size = 0;
}

int64_t gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}
