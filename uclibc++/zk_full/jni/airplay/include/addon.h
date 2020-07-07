#ifndef __ADDON_H__
#define __ADDON_H__

//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>

struct airplay_callbacks_s
{
    void *cls;
    /* Compulsory callback functions */
    void (*AirPlayPlayback_Open)(void *cls,char *url, float fPosition);
    void (*AirPlayPlayback_Play)(void *cls);
    void (*AirPlayPlayback_Pause)(void *cls);
    void (*AirPlayPlayback_Stop)(void *cls);
    void (*AirPlayPlayback_Seek)(void *cls,long fPosition);
    void (*AirPlayPlayback_SetVolume)(void *cls,int volume);
    void (*AirPlayPlayback_ShowPhoto)(void *cls, unsigned char *data, long long size);
    long (*AirPlayPlayback_GetDuration)(void *cls);
    long (*AirPlayPlayback_GetPostion)(void *cls);
    int  (*AirPlayPlayback_IsPlaying)(void *cls);
    int  (*AirPlayPlayback_IsPaused)(void *cls);
    int  (*AirPlayPlayback_IsStopped)(void *cls);
	
    void (*AirPlayAudio_Init)(void *cls, int bits, int channels, int samplerate, int isaudio);
    void (*AirPlayAudio_Process)(void *cls, const void *buffer, int buflen, double timestamp, uint32_t seqnum);
    void (*AirPlayAudio_destroy)(void *cls);
    void (*AirPlayAudio_SetVolume)(void *cls, int volume);//1-100
    void (*AirPlayAudio_SetMetadata) (void *cls, const void *buffer, int buflen);
    void (*AirPlayAudio_SetCoverart)(void *cls, const void *buffer, int buflen);
    void (*AirPlayAudio_Flush)(void *cls);

    //void (*AirPlayMirroring_Play)(void *cls, char *url, char *title);
    void (*AirPlayMirroring_Play)(void *cls, int width,int height,const void *buffer, int buflen, int payloadtype, double timestamp);
    void (*AirPlayMirroring_Process)(void *cls, const void *buffer, int buflen, int payloadtype, double timestamp);
    void (*AirPlayMirroring_Stop)(void *cls);
    void(*AirPlay_SetClientOsVersion) (void *cls,double osversion);
};
typedef struct airplay_callbacks_s airplay_callbacks_t;
int  XinDawn_StartMediaServer(char *friendname, char *dllpath, char *keypath, int width, int height, int framerate, char *activecode, airplay_callbacks_t *cb);
void XinDawn_StopMediaServer();
void XinDawn_Close();
void XinDawn_SetCallBacks(airplay_callbacks_t *cb);
#endif
