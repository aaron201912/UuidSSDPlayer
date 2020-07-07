/*
 * audioplayer.h
 *
 *  Created on: 2020年1月2日
 *      Author: koda.xu
 */

#ifndef JNI_TTS_AUDIOPLAYER_H_
#define JNI_TTS_AUDIOPLAYER_H_

#include "base_types.h"

typedef struct
{
	SHORT  *lpsData;          /* pointer to locked data buffer */
	DWORD dwBufferLength;     /* length of data buffer */
} PlayAudioData_t;


int AudioPlayerInit(int chnCnt, int nBits, int nSampleRate, int s32AoVolume);
int AudioPlayerDeinit(void);
int AudioPlayerSendFrame(PlayAudioData_t* data);
int AudioPlayerPause();
int AudioPlayerResume();
void AudioPlayerSetVolume(int vol);

#endif /* JNI_TTS_AUDIOPLAYER_H_ */
