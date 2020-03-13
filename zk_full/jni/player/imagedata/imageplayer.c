/*
 * showImage.c
 *
 *  Created on: 2020年2月26日
 *      Author: koda.xu
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "imageplayer.h"

static void *DispImageProc(void *pData)
{
	ImagePlayer_t *pstImagePlayer = (ImagePlayer_t*)pData;
	long long currentPos = 0;
	int paused = 0;
	pstImagePlayer->duration = IMAGE_DISPLAY_DURATION;
	pstImagePlayer->stImageCtrl.fpGetDuration(pstImagePlayer->duration);

	while(!pstImagePlayer->exitThread)
	{
		pthread_mutex_lock(&pstImagePlayer->playPosMutex);
		currentPos = pstImagePlayer->currentPos;
		paused = pstImagePlayer->paused;
		if (!paused)
			pstImagePlayer->currentPos += IMAGE_PROGRESS_UPDATE_INTERVAL;
		pthread_mutex_unlock(&pstImagePlayer->playPosMutex);

		if (paused)
		{
			usleep(500000);
			continue;
		}

		if (currentPos >= pstImagePlayer->duration)
		{
			currentPos = 0;
			pstImagePlayer->stImageCtrl.fpDisplayComplete();
			break;
		}

		pstImagePlayer->stImageCtrl.fpGetCurrentPlayPos(currentPos);

		usleep(IMAGE_PROGRESS_UPDATE_INTERVAL);
	}

	return NULL;
}


ImagePlayer_t *ImagePlayer_Init(ImagePlayerCtrl_t *pstPlayerOps)
{
	ImagePlayer_t *pstImagePlayer = (ImagePlayer_t*)malloc(sizeof(ImagePlayer_t));

	memset(pstImagePlayer, 0, sizeof(ImagePlayer_t));
	//pstImagePlayer->fileName = pImagePath;
	if (pstPlayerOps)
		memcpy(&pstImagePlayer->stImageCtrl, pstPlayerOps, sizeof(ImagePlayerCtrl_t));

	pthread_mutex_init(&pstImagePlayer->playPosMutex, NULL);
	pthread_create(&pstImagePlayer->display_tid, NULL, DispImageProc, pstImagePlayer);

	return pstImagePlayer;
}

int ImagePlayer_Deinit(ImagePlayer_t *pPlayer)
{
	pPlayer->exitThread = 1;

	if (pPlayer->display_tid)
	{
		pthread_join(pPlayer->display_tid, NULL);
		pPlayer->display_tid = NULL;
	}

	pthread_mutex_destroy(&pPlayer->playPosMutex);

	if (pPlayer)
	{
		free(pPlayer);
		pPlayer = NULL;
	}

	return 0;
}

void ImagePlayer_TogglePause(ImagePlayer_t *pPlayer)
{
	pthread_mutex_lock(&pPlayer->playPosMutex);
	pPlayer->paused = !pPlayer->paused;
	printf("imagePlayer pause status is %d\n", pPlayer->paused);
	pthread_mutex_unlock(&pPlayer->playPosMutex);
}

void ImagePlayer_Seek(ImagePlayer_t *pPlayer, long long seekPos)
{
	pthread_mutex_lock(&pPlayer->playPosMutex);
	pPlayer->currentPos = seekPos;
	pthread_mutex_unlock(&pPlayer->playPosMutex);
}
