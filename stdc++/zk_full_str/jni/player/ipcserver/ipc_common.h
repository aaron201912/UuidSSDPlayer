#ifndef __IPC_COMMON_H__
#define __IPC_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "stdio.h"
#include "stdbool.h"
#include "string.h"

#define  UNIX_DOMAIN    "/customer/socket.domain"
#define  CLIENT_NAME    "/customer/MyPlayer &"

#define AV_NOTHING          (0x0000)
#define AV_AUDIO_COMPLETE   (0x0001)
#define AV_VIDEO_COMPLETE   (0x0002)
#define AV_PLAY_PAUSE       (0x0004)
#define AV_ACODEC_ERROR     (0x0008)
#define AV_VCODEC_ERROR     (0x0010)
#define AV_NOSYNC           (0x0020)
#define AV_READ_TIMEOUT     (0x0040)
#define AV_NO_NETWORK       (0x0080)
#define AV_INVALID_FILE     (0x0100)
#define AV_AUDIO_MUTE       (0x0200)
#define AV_AUDIO_PAUSE      (0x0400)
#define AV_PLAY_LOOP        (0x0800)

#define AV_PLAY_COMPLETE    (AV_AUDIO_COMPLETE | AV_VIDEO_COMPLETE)
#define AV_PLAY_ERROR       (AV_ACODEC_ERROR | AV_VCODEC_ERROR | AV_NOSYNC | AV_READ_TIMEOUT | AV_NO_NETWORK | AV_INVALID_FILE)

enum {
    AV_ONCE,
    AV_LOOP,
};

enum {
    AV_ROTATE_NONE,
    AV_ROTATE_90,
    AV_ROTATE_180,
    AV_ROTATE_270
};

enum {
    AV_ORIGIN_MODE   = 0,
    AV_SCREEN_MODE   = 1,
    AV_SAR_4_3_MODE  = 2,
    AV_SAR_16_9_MODE = 3
};

enum {
    PLAYER_ACK,
    PLAYER_CREATE,
    PLAYER_DESTORY,
    PLAYER_OPEN,
    PLAYER_CLOSE,
    PLAYER_PAUSE,
    PLAYER_RESUME,
    PLAYER_SEEK,
    PLAYER_SEEK2TIME,
    PLAYER_POSITION,
    PLAYER_DURATION = 10,
    PLAYER_VOLUMN,
    PLAYER_MUTE,
    PLAYER_WINDOW,
    PLAYER_COMPLETE,
    PLAYER_ERROR,
    PLAYER_LINKTEST
};

typedef struct {
    int x;
    int y;
    int width;
    int height;
} window_t;

typedef struct {
    int audio_only;
    int video_only;
    int video_ratio;
    int video_rotate;
    int audio_dev;
    int audio_layout;
    int enable_scaler;
    char resolution[32];
    int play_mode;
} player_opts_t;

typedef struct {
    uint8_t cmd;
    char url[128];
    bool mute;
    int  volumn;
    double position;
    double duration;
    double seektime;
    window_t window;
    player_opts_t opts;
    int  flags;
} ipc_msg_t;

#define IPC_DEBUG   1
#define IPC_WARN    2
#define IPC_ERROR   3
#define IPC_LEVEL   IPC_ERROR

#define ipc_log(level, _fmt, _args...) \
do {\
    if (level >= IPC_LEVEL) {\
        printf(_fmt, ## _args);\
    }\
} while(0)

#ifdef __cplusplus
}
#endif // __cplusplu

#endif

