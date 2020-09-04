#ifndef __SSTAR_API_H__
#define __SSTAR_API_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define PLAYERSTATUS_NONE          0x00
#define PLAYERSTATUS_START         0x01
#define PLAYERSTATUS_BUFFER        0x02
#define PLAYERSTATUS_OPENED        0x04

#define BUFFER_TRIGGER              30
#define PLAYERCMD_OPENED            0
#define PLAYERCMD_START_LOADING     1
#define PLAYERCMD_STOP_LOADING      2
#define PLAYERCMD_SEEKED            3
#define PLAYERCMD_COMPLETE_VEDIO    4
#define PLAYERCMD_COMPLETE_AUDIO    5
#define PLAYERCMD_UNSUPPORTFORMAT   6
#define PLAYERCMD_READ_TIMEOUT      7

int my_player_open(const char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
int my_player_close(void);
int my_player_pause(void);
int my_player_resume(void);
int my_player_seek(double time);
int my_player_seek2time(double time);
int my_player_getposition(double *position);
int my_player_getduration(double *duration);
int my_player_set_chlayout(int soundmode);
int my_player_set_aodev(int dev);
int my_player_set_rotate(int rotate);
int my_player_set_volumn(int volumn);
int my_player_set_mute(bool mute);
int my_player_set_window(int x, int y, int width, int height);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif

