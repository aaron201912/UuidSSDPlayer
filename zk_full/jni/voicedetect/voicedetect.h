#ifndef __AUDIODETECT_H__
#define __AUDIODETECT_H__
#include "base_types.h"


#ifdef __cplusplus
extern "C" {
#endif


#define COMMAND_LEN					40

typedef struct
{
	char cmd[COMMAND_LEN];
} TrainedWord_t;

typedef void* (*VoiceAnalyzeCallback)(int);

int SSTAR_VoiceDetectGetWordList(TrainedWord_t *pWordList, int nWordCnt);
HANDLE SSTAR_VoiceDetectInit();
int SSTAR_VoiceDetectDeinit(HANDLE hDSpotter);
int SSTAR_VoiceDetectStart(HANDLE hDSpotter, VoiceAnalyzeCallback pfnCallback);
void SSTAR_VoiceDetectStop();

#ifdef __cplusplus
}
#endif

#endif
