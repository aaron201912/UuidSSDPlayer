#ifndef __AUDIO_H__
#define __AUDIO_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "player.h"


#ifdef SUPPORT_HDMI
#define AUDIO_DEV           3
#else
#define AUDIO_DEV           0
#endif

#define AUDIO_CHN           0

#define MIN_AO_VOLUME           -60
#define MAX_AO_VOLUME           30
#define MIN_ADJUST_AO_VOLUME    -10
#define MAX_ADJUST_AO_VOLUME    20

extern MI_AUDIO_DEV gplayer_AoDevId;
extern int64_t g_audio_chlayout;


int open_audio(player_stat_t *is);
int my_audio_deinit(void);
int my_audio_init(int naodev);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif
