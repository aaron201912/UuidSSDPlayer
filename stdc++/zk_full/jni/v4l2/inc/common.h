#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdint.h>
#include <stddef.h>

typedef struct BufferRef {
    uint8_t *data;
    int      size;
} BufferRef;

typedef struct Packet {
    BufferRef *buf;
    int size;
    uint8_t *data;
    int64_t pts;
} Packet;

int new_packet(Packet *pkt, int size);
void free_packet(Packet *pkt);
int64_t gettime(void);
#endif //_COMMON_H_
