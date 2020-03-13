/*******************/
/*  Include Files  */
/*******************/
#include "tts.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//For thread
#include <pthread.h>
//SDK
#include "CReader.h"
#include "CReader_const.h"
//For audio
#include "audioplayer.h"
#include "list.h"
//

/*******************/
/*      Macro      */
/*******************/
#define PLAY_STATUS_STOPPED 0
#define PLAY_STATUS_PLAYING 1
#define PLAY_STATUS_PAUSE 2

#define _MAX_LINE_LEN_ (1024)
#define _MAX_BUF_LEN_ (256)

//#define LANG_SUPPORT_NUM 	42
//#define SPEAKER_SUPPORT_NUM 20

/*******************/
/*    Structure    */
/*******************/
typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char chRIFF[4];
    unsigned int  dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    unsigned int  dwFMTLen;
    WaveFormat_t wave;
    char chDATA[4];
    unsigned int  dwDATALen;
} WaveFileHeader_t;

typedef struct _Player
{
	//CReader SDK
	HANDLE m_hCReader;

	//For Audio
	BOOL m_bAudioInit;
	BOOL m_bAudioStart;
	INT m_nPlayStatus;
	BOOL m_bTTSFinished;
	
	//For thread
	pthread_t m_hSynthesizeThread;
	pthread_t m_hPlayThread;
	pthread_mutex_t m_hMutex;
	BOOL m_bStop;

	//For link list
	list_t m_audioListHead;

	//Text to Synthesize
	char *m_pchText;
	BOOL m_bSavePlayData;
//	int m_hWaveFile;
	FILE *m_pFile;
	WaveFileHeader_t  m_waveFileHeader;
	TtsCallback pfnCallback;
} Player;

typedef struct
{
	list_t dataList;
	PlayAudioData_t *pAudioData;
} AudioListData_t;

typedef struct tm SYSTEMTIME;

//Playing parameters
static const int g_nBit = 16;
static const int g_nChannel = 1;
static const int g_nSampleRate = 16000;

Language g_language[LANG_SUPPORT_NUM] = {
	{"Undefined"},//0
	{"Chinese_TW"},//1
	{"Chinese_CN"},
	{"Cantonese"},
	{"English"},//4
	{"English_UK"},
	{"German"},//6
	{"Spanish_SA"},
	{"French"},
	{"Italian"},
	{"Korean"},//10
	{"Russian"},
	{"Portuguese_BZ"},
	{"Undefined"},
	{"Thai"},
	{"Arabic"},//15
	{"Dutch"},
	{"Japanese"},
	{"Hindi"},
	{"Polish"},
	{"Czech"},//20
	{"Turkish"},
	{"Danish"},
	{"Swedish"},
	{"Norwegian"},
	{"Finnish"},//25
	{"Greek"},
	{"Portuguese"},
	{"Slovak"},
	{"Hungarian"},
	{"Ukrainian"},//30
	{"English_AU"},
	{"Undefined"},
	{"Spanish_EU"},
	{"Minnan"},
	{"Vietnamese"},
	{"Undefined"},
	{"Undefined"},
	{"Bahasa_IDN"},
	{"Bahasa_MAL"},
	{"Philippines"},//40
	{"Hakka"}
};

Speaker g_speaker[SPEAKER_SUPPORT_NUM];

/*******************/
/*  Local Function */
/*******************/
static VOID GetLocalTime(SYSTEMTIME *st)
{
	struct tm *pst = NULL;
	time_t t = time(NULL);
	pst = localtime(&t);
	memcpy(st, pst, sizeof(SYSTEMTIME));
	st->tm_year += 1900;
}

//When the engine generates TTS data, it will call this function to transfer TTS data to outside
INT CallBack(LPVOID lpVoid, INT nFlag, CHAR *lpszWav, INT nSize)
{
	Player *poPlayer = (Player*)lpVoid;

	if(nFlag == CREADER_CB_WAV && lpszWav != NULL && nSize > 0)
	{
//		printf("TTSThread: nFlag is %d, lpszWav is %p, nSize is %d bytes\n", nFlag, lpszWav, nSize);

		AudioListData_t *pAudioListData = NULL;
		pAudioListData = (AudioListData_t*)malloc(sizeof(AudioListData_t));
		if (!pAudioListData)
		{
			printf("fail to allocate memory for audio list data\n");
			return 0;
		}

		pAudioListData->pAudioData = (PlayAudioData_t*)malloc(sizeof(PlayAudioData_t));
		if (!pAudioListData->pAudioData)
		{
			printf("fail to allocate memory for audio data struct\n");
			free(pAudioListData);
			return 0;
		}

		pAudioListData->pAudioData->lpsData = (SHORT*)malloc(nSize);
		if (!pAudioListData->pAudioData->lpsData)
		{
			printf("fail to allocate memory for audio data\n");
			free(pAudioListData->pAudioData);
			free(pAudioListData);
			return 0;
		}

		pAudioListData->pAudioData->dwBufferLength = nSize / sizeof(SHORT);
		memcpy(pAudioListData->pAudioData->lpsData, lpszWav, nSize);
//		printf("add audio data, dataLen=%d, pBuffer=%p\n", pAudioListData->pAudioData->dwBufferLength,
//				pAudioListData->pAudioData->lpsData);


		INIT_LIST_HEAD(&pAudioListData->dataList);
		pthread_mutex_lock(&(poPlayer->m_hMutex));
		list_add_tail(&pAudioListData->dataList, &poPlayer->m_audioListHead);
		pthread_mutex_unlock(&(poPlayer->m_hMutex));

		//printf("add audio data: dataLen=%d\n", nSize);;
	}
	else if(nFlag == CREADER_CB_TEXT)
	{
		//printf("CallBack():: Synthesize %s\n",lpszWav);
	}

	return 0;
}

//Use WavePlay play sound
int StartPlay(Player *poPlayer)
{
	int  i, nAddSize;
	short *sample, nALSAWriteBuff[640] = {0};
	long loops;
	int rc = 0;
	int size;
	unsigned int val;
	int dir;
	int nRet = 0;

	printf("StartPlay() start\n");
	poPlayer->m_bStop = FALSE;
	
	while(1)
	{
		if(poPlayer->m_nPlayStatus == PLAY_STATUS_PAUSE)
		{
			usleep(10000);
			continue;
		}
		
		if(poPlayer->m_nPlayStatus == PLAY_STATUS_STOPPED)
		{
			//printf("Stop!!\n");
			break;
		}
			
		//get TTS data
		if (!list_empty(&poPlayer->m_audioListHead))
		{
			AudioListData_t *pAudioListData = NULL;
			pthread_mutex_lock(&(poPlayer->m_hMutex));
			pAudioListData = list_entry(poPlayer->m_audioListHead.next, AudioListData_t, dataList);
			list_del(&pAudioListData->dataList);
			pthread_mutex_unlock(&(poPlayer->m_hMutex));

			if (!pAudioListData)
			{
				printf("audiolist is not empty, but get no data, continue\n");
				continue;
			}

			//printf("PlayThread: pBuffer=%p, dataLen=%d\n", (char*)pAudioListData->pAudioData->lpsData, pAudioListData->pAudioData->dwBufferLength*2);

			//if(poPlayer->m_bSavePlayData)
			if (0)
			{
				printf("save audio data, datalen=%d, pBuffer=%p\n", pAudioListData->pAudioData->dwBufferLength,
						pAudioListData->pAudioData->lpsData);
				fwrite((BYTE*)pAudioListData->pAudioData->lpsData, pAudioListData->pAudioData->dwBufferLength * 2, 1, poPlayer->m_pFile);
				poPlayer->m_waveFileHeader.dwDATALen += pAudioListData->pAudioData->dwBufferLength;
				printf("save audio data done\n");
			}

			nRet = AudioPlayerSendFrame(pAudioListData->pAudioData);
			free(pAudioListData->pAudioData->lpsData);
			free(pAudioListData->pAudioData);
			free(pAudioListData);

			if (nRet)
			{
				printf("play audio failed!\n");
				break;
			}

		}
		else
		{
			//TTS data generate finished and play finished
			//if(poPlayer->m_bTTSFinished && (!poPlayer->m_bAudioStart))		// never enter
			if (poPlayer->m_bTTSFinished)
			{
				printf("play thread exit\n");
				break;
			}
			//The speed of play TTS data is more than the speed of generating TTS data
			usleep(10000);
			printf("StartPlay():: No TTS data!, m_nPlayStatus: %s\n", (poPlayer->m_nPlayStatus == PLAY_STATUS_PAUSE)?"pause":((poPlayer->m_nPlayStatus == PLAY_STATUS_PLAYING)?"playing":"stop"));
		}
	}

	//close audio
	if(poPlayer->m_nPlayStatus == PLAY_STATUS_STOPPED)
		printf("StartPlay():: Stop by user\n");
	else
	{
		printf("StartPlay():: Stop by synthesize finish\n");
		poPlayer->pfnCallback();
	}

	poPlayer->m_bStop = TRUE;

	if(poPlayer->m_bSavePlayData)
	{
		if (poPlayer->m_bSavePlayData)
		{
			fseek(poPlayer->m_pFile, 0L, SEEK_SET);
			fwrite(&poPlayer->m_waveFileHeader, sizeof(WaveFileHeader_t), 1, poPlayer->m_pFile);
			fclose(poPlayer->m_pFile);
			poPlayer->m_pFile = NULL;
		}
	}

//	AudioPlayerDeinit();
//	poPlayer->m_bAudioInit = FALSE;

	printf("StartPlay() end\n");

	return 0;
}

//Play TTS data Thread
void* ThreadPlay(void* param)
{
	Player *poPlayer = (Player*)param;
	int nTryCnt = 0;

	//Wait TTS data is generated
	printf("ThreadPlay():: wait TTS data Start!\n");
	while(nTryCnt++ < 500)
	{
		if (!list_empty(&poPlayer->m_audioListHead))
			break;

		usleep(2000);
	}
	printf("ThreadPlay():: wait TTS data End!\n");
	StartPlay(poPlayer);
	pthread_exit(0);
	return NULL;
}

//Generate TTS data Thread (Save TTS data to link list in callback function)
void* ThreadTTS(void* param)
{
	int nRet;
	Player *poPlayer = (Player*)param;
	CREADER_STATUS nRes;

	printf("ThreadTTS:: Start!\n");

	//Check input parameter
	if(poPlayer == NULL)
	{
		printf("ThreadTTS:: Error input parameter!\n");
		return NULL;
	}

	//Set variables
	if((nRet = CReader_StartWithUTF8(poPlayer->m_hCReader, (const char*)poPlayer->m_pchText, CREADER_TYPE_NORMAL, CallBack, (LPVOID)poPlayer)) != CREADER_RET_OK)
	{
		printf("ThreadTTS: Fail to call CReader_StartWithUTF8(...)!\n");
		return NULL;
	}

	printf("ThreadTTS: g_nPlayStatus: %s\n", (poPlayer->m_nPlayStatus == PLAY_STATUS_PAUSE)?"pause":((poPlayer->m_nPlayStatus == PLAY_STATUS_PLAYING)?"playing":"stop"));

	do 
	{
		if(poPlayer->m_nPlayStatus == PLAY_STATUS_PAUSE)
		{
			usleep(20000);
			continue;
		}

		nRes = CReader_Next(poPlayer->m_hCReader);

	} while(nRes == CREADER_RET_OK);
	
	poPlayer->m_bTTSFinished = TRUE;

	printf("ThreadTTS: Exit while loop, nRes=%d, g_nPlayStatus: %s\n", nRes, (poPlayer->m_nPlayStatus == PLAY_STATUS_PAUSE)?"pause":((poPlayer->m_nPlayStatus == PLAY_STATUS_PLAYING)?"playing":"stop"));

	//Wait playing end
	if(poPlayer->m_hPlayThread != (pthread_t)NULL)
	{
		pthread_join(poPlayer->m_hPlayThread, NULL);
		poPlayer->m_hPlayThread = (pthread_t)NULL;
	}
	
	//Remove remainder data
	pthread_mutex_lock(&(poPlayer->m_hMutex));
	while (!list_empty(&poPlayer->m_audioListHead))
	{
		AudioListData_t *pAudioListData = NULL;
		pAudioListData = list_entry(poPlayer->m_audioListHead.next, AudioListData_t, dataList);
		list_del(&pAudioListData->dataList);

		if (pAudioListData)
		{
			if (pAudioListData->pAudioData->lpsData)
			{
				free(pAudioListData->pAudioData->lpsData);
				pAudioListData->pAudioData->lpsData = NULL;
			}

			if (pAudioListData->pAudioData)
			{
				free(pAudioListData->pAudioData);
				pAudioListData->pAudioData = NULL;
			}

			free(pAudioListData);
			pAudioListData = NULL;
		}
	}

	pthread_mutex_unlock(&(poPlayer->m_hMutex));
	printf("ThreadTTS():: ThreadTTS end!\n");
	poPlayer->m_nPlayStatus = PLAY_STATUS_STOPPED;
	//pthread_exit(0);

	return NULL;
}

/*******************/
/*       API       */
/*******************/
int TTS_GetLanguageMaxNum()
{
	return sizeof(g_language)/sizeof(Language);
}

// return available language count
int TTS_GetAvailableLangID(char* pchDataPath, int *pLanguageID)
{
	CREADER_STATUS nRes;
	int nLangNum = 0;

	nLangNum = CReader_GetAvailableLangID(pchDataPath, NULL, 0, &nRes);
	if (nLangNum > 0)
	{
		CReader_GetAvailableLangID(pchDataPath, pLanguageID, nLangNum, &nRes);
		//support language
		for(int i = 0; i < nLangNum; i++)
		{
			printf("%d : %s\n", pLanguageID[i], g_language[pLanguageID[i]].Lang);
		}
	}

	return nLangNum;
}

// return available speaker count
int TTS_GetAvailableSpeaker(char* pchLibPath, char* pchDataPath, int nLangID)
{
	CREADER_STATUS nRes;
	int nSpeaker = 0;
	int nSpeakerNameLen = 0;

	memset(g_speaker, 0, sizeof(g_speaker));

	//Get Speaker number
	nSpeaker = CReader_GetAvailableSpeakerNum(pchDataPath, nLangID, &nRes);

	if (nSpeaker > 0)
	{
		for (int i = 0; i < nSpeaker; i++)
		{
			char *pSpeakerName = NULL;
			nSpeakerNameLen = CReader_GetAvailableSpeaker(pchLibPath, pchDataPath, nLangID, i, NULL, 0, &nRes);
			pSpeakerName = (char*)malloc(sizeof(char) * nSpeakerNameLen);
			if(!pSpeakerName)
			{
				printf("TTS_GetAvailableSpeaker:: allocate failed\n");
				for(; i >= 0; i--)
					memset(g_speaker[i].speaker, 0, sizeof(g_speaker[i].speaker));
				return -1;
			}

			CReader_GetAvailableSpeaker(pchLibPath, pchDataPath, nLangID, i, pSpeakerName, nSpeakerNameLen, &nRes);
			if(nRes == CREADER_RET_OK)
			{
				printf("speaker %d: %s, %d\n", i, pSpeakerName, nSpeakerNameLen);
				memcpy(g_speaker[i].speaker, pSpeakerName, nSpeakerNameLen);
				free(pSpeakerName);
			}
			else
			{
				printf("Get speaker failed: %d\n", nRes);
				free(pSpeakerName);
				for(; i >= 0; i--)
					memset(g_speaker[i].speaker, 0, sizeof(g_speaker[i].speaker));
				return -1;
			}
		}
	}

	return nSpeaker;
}

HANDLE TTS_Initialize(char* pchLibPath, char* pchDataPath, int nLangID, char *pSpeakerName, TtsCallback pfnCallback)
{
	Player *poPlayer;
	CREADER_STATUS nRes;

	poPlayer = (Player*)malloc(sizeof(Player));
	if(!poPlayer)
	{
		printf("Allocate memory failed\n");
		return NULL;
	}

	memset(poPlayer, 0, sizeof(Player));
	poPlayer->m_hCReader = CReader_Init(nLangID, pchLibPath, pchDataPath, pSpeakerName, &nRes);

	if(nRes != CREADER_RET_OK)
	{
		printf("TTS_Init:: Fail to create TTS = %d\n", nRes);
		return NULL;
	}

	pthread_mutex_init(&(poPlayer->m_hMutex), NULL);
	INIT_LIST_HEAD(&poPlayer->m_audioListHead);

	// init audio player
	if (AudioPlayerInit(g_nChannel, g_nBit, g_nSampleRate, 50 - 60))
	{
		printf("init audio failed\n");
		return NULL;
	}

	poPlayer->m_bAudioInit = TRUE;
	poPlayer->pfnCallback = pfnCallback;

	// init dump file header, file header should be updated before saving.
	memcpy(poPlayer->m_waveFileHeader.chRIFF, "RIFF", sizeof(poPlayer->m_waveFileHeader.chRIFF));
	poPlayer->m_waveFileHeader.dwRIFFLen = sizeof(WaveFileHeader_t);
	memcpy(poPlayer->m_waveFileHeader.chWAVE, "WAVE", sizeof(poPlayer->m_waveFileHeader.chWAVE));
	memcpy(poPlayer->m_waveFileHeader.chFMT, "fmt ", sizeof(poPlayer->m_waveFileHeader.chWAVE));
	poPlayer->m_waveFileHeader.dwFMTLen = 16;
	poPlayer->m_waveFileHeader.wave.wFormatTag = 1;
	poPlayer->m_waveFileHeader.wave.wChannels = 1;
	poPlayer->m_waveFileHeader.wave.dwSamplesPerSec = 16000;
	poPlayer->m_waveFileHeader.wave.wBitsPerSample = 16;
	poPlayer->m_waveFileHeader.wave.dwAvgBytesPerSec = 32000;
	poPlayer->m_waveFileHeader.wave.wBlockAlign = 2;
	memcpy(poPlayer->m_waveFileHeader.chDATA, "data", sizeof(poPlayer->m_waveFileHeader.chDATA));
	poPlayer->m_waveFileHeader.dwDATALen = 0;
	poPlayer->m_pFile = NULL;

	return (HANDLE)poPlayer;
}

int TTS_Release(HANDLE hPlayer)
{
	Player *poPlayer = (Player*)hPlayer;

	if(poPlayer == NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	if (poPlayer->m_bAudioInit)
	{
		poPlayer->m_bAudioInit = FALSE;
		AudioPlayerDeinit();
	}

	//Release resource
	if(poPlayer->m_pchText)
		poPlayer->m_pchText = NULL;

	if (poPlayer->m_pFile)
	{
		fseek(poPlayer->m_pFile, 0L, SEEK_SET);
		fwrite(&poPlayer->m_waveFileHeader, sizeof(WaveFileHeader_t), 1, poPlayer->m_pFile);
		fclose(poPlayer->m_pFile);
		poPlayer->m_pFile = NULL;
	}

	if (!list_empty(&poPlayer->m_audioListHead))
	{
		AudioListData_t *pos = NULL;
		AudioListData_t *posN = NULL;
		list_for_each_entry_safe(pos, posN, &poPlayer->m_audioListHead, dataList)
		{
			list_del(&pos->dataList);
			free(pos);
		}
	}

	pthread_mutex_destroy(&(poPlayer->m_hMutex));

	if(poPlayer->m_hCReader)
		CReader_Release(poPlayer->m_hCReader);

	SAFE_FREE(poPlayer);

	return 0;
}

int TTS_AddTTSStringUTF8(HANDLE hPlayer, char* pchUTF8Str)
{
	Player *poPlayer = (Player*)hPlayer;

	if(poPlayer==NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	poPlayer->m_pchText = pchUTF8Str;

	if(!poPlayer->m_pchText)
		return __PLAYER_ERR_ADD_TTSSTRING__;

	return __PLAYER_SUCCESS__;
}

int TTS_Start(HANDLE hPlayer, BOOL bSavePlayData)
{
	Player *poPlayer = (Player*)hPlayer;
	char pchzRecordFile[_MAX_BUF_LEN_];
	SYSTEMTIME oSystemTime;
	INT nErr;

	if(poPlayer==NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	//initial
	poPlayer->m_bSavePlayData = bSavePlayData;
	poPlayer->m_nPlayStatus = PLAY_STATUS_PLAYING;
	poPlayer->m_bTTSFinished = FALSE;

	//Create WaveFile
	if(poPlayer->m_bSavePlayData)
	{
		if (poPlayer->m_pFile)
		{
			fseek(poPlayer->m_pFile, 0L, SEEK_SET);
			fwrite(&poPlayer->m_waveFileHeader, sizeof(WaveFileHeader_t), 1, poPlayer->m_pFile);
			fclose(poPlayer->m_pFile);
			poPlayer->m_pFile = NULL;
		}
		GetLocalTime(&oSystemTime);
		sprintf(pchzRecordFile, "./CReader_Record_%04d_%02d_%02d_%02d_%02d_%02d.wav", oSystemTime.tm_year,
				oSystemTime.tm_mon + 1, oSystemTime.tm_mday, oSystemTime.tm_hour, oSystemTime.tm_min, oSystemTime.tm_sec);

		poPlayer->m_pFile = fopen(pchzRecordFile, "w+");
		if (!poPlayer->m_pFile)
		{
			printf("open dump file failed\n");
			poPlayer->m_bSavePlayData = FALSE;
		}
		else
		{
			poPlayer->m_waveFileHeader.dwDATALen = 0;
			fwrite(&poPlayer->m_waveFileHeader, sizeof(WaveFileHeader_t), 1, poPlayer->m_pFile);
		}
	}

	//Create Thread
	poPlayer->m_bAudioStart = TRUE;
	pthread_create(&poPlayer->m_hPlayThread, NULL, &ThreadPlay, (LPVOID)hPlayer);
	pthread_create(&poPlayer->m_hSynthesizeThread, NULL, &ThreadTTS, (LPVOID)hPlayer);

	if(poPlayer->m_hPlayThread == (pthread_t)NULL || poPlayer->m_hSynthesizeThread == (pthread_t)NULL)
	{
		printf("main():: Fail to create TTS or Play thread!\r\n");
		return __PLAYER_ERR_CREATE_THREAD__;
	}

	return __PLAYER_SUCCESS__;
}

int TTS_Pause(HANDLE hPlayer)
{
	Player *poPlayer = (Player*)hPlayer;

	if(poPlayer==NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	printf("TTS_Pause\n");
	AudioPlayerPause();
	poPlayer->m_nPlayStatus = PLAY_STATUS_PAUSE;
	printf("TTS_Pause done\n");

	return __PLAYER_SUCCESS__;
}

int TTS_Stop(HANDLE hPlayer)
{
	Player *poPlayer = (Player*)hPlayer;

	if(poPlayer==NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	poPlayer->m_nPlayStatus = PLAY_STATUS_STOPPED;
	CReader_Stop(poPlayer->m_hCReader);
	poPlayer->m_bAudioStart = FALSE;

	if(poPlayer->m_hSynthesizeThread != (pthread_t)NULL)
	{
		pthread_join(poPlayer->m_hSynthesizeThread, NULL);
		poPlayer->m_hSynthesizeThread = (pthread_t)NULL;
	}

	return __PLAYER_SUCCESS__;
}

int TTS_Resume(HANDLE hPlayer)
{
	Player *poPlayer = (Player*)hPlayer;

	if(poPlayer==NULL)
		return __PLAYER_ERR_NULL_HANDLE__;

	printf("TTS_Resume\n");
	AudioPlayerResume();
	poPlayer->m_nPlayStatus = PLAY_STATUS_PLAYING;
	printf("TTS_Resume done\n");

	return __PLAYER_SUCCESS__;
}

