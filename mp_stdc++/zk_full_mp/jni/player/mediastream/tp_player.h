#ifndef _TP_PLAYER_H_
#define _TP_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SUPPORT_CLOUD_PLAY_MODULE) || defined(SUPPORT_PLAYER_MODULE)
#include "player.h"
#include "mi_divp.h"
#include "mi_divp_datatype.h"

#include "appconfig.h"

#if USE_PANEL_1024_600
#define MAINWND_W       1024
#define MAINWND_H       600
#else
#define MAINWND_W       800
#define MAINWND_H       480
#endif

int tp_player_open(char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *parg);
int tp_player_close(void);
int tp_player_status(void);

#endif

#ifdef __cplusplus
}
#endif

#endif

