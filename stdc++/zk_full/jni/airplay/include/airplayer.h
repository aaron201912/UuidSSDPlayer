#ifndef AIRPLAYER_H_
#define AIRPLAYER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <arpa/inet.h>


#include "mi_vdec.h"
#include "mi_vdec_datatype.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
#include "mi_ao.h"
#include "mi_divp.h"
#include "mi_divp_datatype.h"

extern "C"
{
	#include "libavformat/avformat.h"
}


//bool IsH264ModeChange(unsigned char *buf, int spslen);
int Ss_mAirplayServiceClose(void);

#ifdef __cplusplus
extern "C" {
#endif

int Ss_DLNA_ServiceStart(void);
int Ss_DLNA_ServiceClose(void);
int Ss_AirplayStart(void);
int Ss_Player_Init(int input_w,int input_h,int flag);
int Ss_Player_DeInit(int flag);
int Ss_AO_Init(void);
int Ss_AO_Deinit(void);
int AirplayServiceStart(char * pickname);
int Ss_Player_SetlayerSize(int w, int h);

void Ss_UI_Close(void);
void Ss_UI_Open(void);

typedef struct {
    const char *fp;
    int exit;
    int pcm_exit;
} ss_player_t;


#if defined (__cplusplus)
}
#endif


#endif
