#ifndef _TTS_H_
#define _TTS_H_

#include "base_types.h"

//Error Code
#define __PLAYER_SUCCESS__				(0)
#define __PLAYER_ERR_NO_MEMORY__		(-1)
#define __PLAYER_ERR_INIT_SDK__			(-2)
#define __PLAYER_ERR_OPEN_FILE__		(-3)
#define __PLAYER_ERR_FILE_FROMAT__		(-4)
#define __PLAYER_ERR_ADD_TTSSTRING__	(-5)
#define __PLAYER_ERR_WRONG_PARAM__		(-6)
#define __PLAYER_ERR_CREATE_THREAD__	(-7)
#define __PLAYER_ERR_NULL_HANDLE__		(-8)

#define LANG_SUPPORT_NUM 	42
#define SPEAKER_SUPPORT_NUM 20

typedef struct _Languages{
	char Lang[20];
}Language;

typedef struct _Speakers{
	char speaker[64];
} Speaker;

typedef void (*TtsCallback)();


int TTS_GetLanguageMaxNum();
int TTS_GetAvailableLangID(char* pchDataPath, int *pLanguageID);
int TTS_GetAvailableSpeaker(char* pchLibPath, char* pchDataPath, int nLangID);
HANDLE TTS_Initialize(char* pchLibPath, char* pchDataPath, int nLangID, char *pSpeakerName, TtsCallback pfnCallback);
int TTS_Release(HANDLE hPlayer);
int TTS_AddTTSStringUTF8(HANDLE hPlayer, char* pchUTF8Str);
int TTS_Start(HANDLE hPlayer, BOOL bSavePlayData);
int TTS_Pause(HANDLE hPlayer);
int TTS_Resume(HANDLE hPlayer);
int TTS_Stop(HANDLE hPlayer);

#endif//_PLAYER_H_
