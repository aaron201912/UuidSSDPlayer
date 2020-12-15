#ifndef __MDLNA_H__
#define __MDLNA_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//*************************************************
#define  ZMFLY_IDLE   	    0x00
#define  ZMFLY_MIRROR  	    0x01
#define  ZMFLY_ARIPLAY  	0x02
#define  ZMFLY_ICAST  		0x03
#define  ZMFLY_DLNA 		0x04
#define  ZMFLY_ERROR		0x05

#define  DMR_NONE    		0
#define  DMR_PHOTO   		1
#define  DMR_MUSIC   		2
#define  DMR_MOVIE   		3

#define  MPTH_RET_CHK(_pf_) \
({ \
    int r = _pf_; \
    if ((r != 0) && (r != ETIMEDOUT)) \
        printf("[MPTHREAD] %s: %d: %s: %s\n", __FILE__, __LINE__, #_pf_, strerror(r)); \
    r; \
})
#define MASSERT(_x_)                                                                        \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
//-------------------------------------------------------------------------------------------------

struct dlna_callbacks
{
    /* Compulsory callback functions */
    void (*DLNAPlayback_Open)(char *url,char *MetaData, float fPosition);
    void (*DLNAPlayback_Play)();
    void (*DLNAPlayback_Pause)();
    void (*DLNAPlayback_Stop)();
    void (*DLNAPlayback_Seek)(long fPosition);
    void (*DLNAPlayback_SetVolume)(int volume);
    long (*DLNAPlayback_GetDuration)();
    long (*DLNAPlayback_GetPostion)();
    int  (*DLNAPlayback_IsPlaying)();
    int  (*DLNAPlayPlayback_IsPaused)();
    int  (*DLNAPlayPlayback_IsStopped)();

};
typedef struct dlna_callbacks dlna_callbacks_t;

#ifdef __cplusplus
extern "C" {
#endif
	int  ZmDLNAServiceStart(char * friendly_name, dlna_callbacks_t *cb);
	int  ZmDLNAServiceClose(void);
#ifdef __cplusplus
}
#endif

#endif
