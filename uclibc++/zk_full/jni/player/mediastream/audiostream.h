#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#ifdef __cplusplus
extern "C" {               // 告诉编译器下列代码要以C链接约定的模式进行链接
#endif

#ifdef SUPPORT_PLAYER_MODULE
#include "player.h"
#include "mi_ao.h"


int open_audio(player_stat_t *is);

#endif

#ifdef __cplusplus
}
#endif

#endif
