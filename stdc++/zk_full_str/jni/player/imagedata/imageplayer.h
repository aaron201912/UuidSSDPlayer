/*
 * showImage.h
 *
 *  Created on: 2020年2月26日
 *      Author: koda.xu
 */

#ifndef JNI_PLAYER_IMAGEDATA_IMAGEPLAYER_H_
#define JNI_PLAYER_IMAGEDATA_IMAGEPLAYER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define IMAGE_DISPLAY_DURATION				5000000
#define IMAGE_PROGRESS_UPDATE_INTERVAL		1000000

typedef struct
{
	int (*fpGetDuration)(long long duration);
	int (*fpGetCurrentPlayPos)(long long currentPos);
//	int (*fpDisplayImage)(char *pImagePath);
//	int (*fpPauseDisplayImage)();
//	int (*fpResumeDisplayImage)();
	int (*fpDisplayComplete)();
} ImagePlayerCtrl_t;

typedef struct
{
	//char *fileName;
	int paused;
	int exitThread;
	long long currentPos;
	long long duration;
	ImagePlayerCtrl_t	stImageCtrl;
	pthread_t display_tid;
	pthread_mutex_t playPosMutex;
} ImagePlayer_t;


ImagePlayer_t *ImagePlayer_Init(ImagePlayerCtrl_t *pstPlayerOps);
int ImagePlayer_Deinit(ImagePlayer_t *pPlayer);
void ImagePlayer_TogglePause(ImagePlayer_t *pPlayer);
void ImagePlayer_Seek(ImagePlayer_t *pPlayer, long long seekPos);

#endif /* JNI_PLAYER_IMAGEDATA_IMAGEPLAYER_H_ */
