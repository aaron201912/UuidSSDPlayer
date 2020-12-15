#ifndef __VIDEO_STREAM_H__
#define __VIDEO_STREAM_H__

#ifdef __cplusplus
extern "C" {               // 告诉编译器下列代码要以C链接约定的模式进行链接
#endif

#ifdef SUPPORT_PLAYER_MODULE
#include "player.h"

//#include "mi_hdmi.h"
//#include "mi_hdmi_datatype.h"
#include "mi_disp.h"
#include "mi_disp_datatype.h"


#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_BACK(x, a)        (((x) / (a)) * (a))

#define SOFT_DECODING   0
#define HARD_DECODING   1

int open_video(player_stat_t *is);
int video_buffer_flush(player_stat_t *is);

#endif

#ifdef __cplusplus
}
#endif
#endif

