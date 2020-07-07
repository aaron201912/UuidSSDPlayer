#ifndef __PACKET_H__
#define __PACKET_H__

#ifdef __cplusplus
extern "C" {               // 告诉编译器下列代码要以C链接约定的模式进行链接
#endif

#ifdef SUPPORT_PLAYER_MODULE
#include "player.h"

int packet_queue_init(packet_queue_t *q);
int packet_queue_put(packet_queue_t *q, AVPacket *pkt);
int packet_queue_get(packet_queue_t *q, AVPacket *pkt, int block);
int packet_queue_put_nullpacket(packet_queue_t *q, int stream_index);
void packet_queue_destroy(packet_queue_t *q);
void packet_queue_abort(packet_queue_t *q);
void packet_queue_flush(packet_queue_t *q);
#endif

#ifdef __cplusplus
}
#endif
#endif
