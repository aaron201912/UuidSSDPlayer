#include "voicedetect.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include "mi_sys.h"
#include "mi_ai.h"
#include "list.h"
#include "base_types.h"
#include "DSpotterApi.h"
#include "DSpotterApi_Const.h"
#include "appconfig.h"


#define AUDIO_PT_NUMBER_FRAME (1024)
#define AUDIO_SAMPLE_RATE (E_MI_AUDIO_SAMPLE_RATE_16000)
#define AUDIO_SOUND_MODE (E_MI_AUDIO_SOUND_MODE_MONO)

#define AUDIO_AO_DEV_ID_LINE_OUT 0
#define AUDIO_AO_DEV_ID_I2S_OUT  1

#define AUDIO_AI_DEV_ID_AMIC_IN   0
#define AUDIO_AI_DEV_ID_DMIC_IN   1
#define AUDIO_AI_DEV_ID_I2S_IN    2

#if USE_AMIC
#define AI_DEV_ID (AUDIO_AI_DEV_ID_AMIC_IN)
#else
#define AI_DEV_ID (AUDIO_AI_DEV_ID_DMIC_IN)
#endif
#define AO_DEV_ID (AUDIO_AO_DEV_ID_LINE_OUT)

#ifndef ExecFunc
#define ExecFunc(_func_, _ret_) \
    do{ \
        MI_S32 s32Ret = MI_SUCCESS; \
        s32Ret = _func_; \
        if (s32Ret != _ret_) \
        { \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret; \
        } \
        else \
        { \
            printf("[%s %d]exec function pass\n", __func__, __LINE__); \
        } \
    } while(0)
#endif

#define COLOR_NONE          "\033[0m"
#define COLOR_BLACK         "\033[0;30m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_WHITE         "\033[1;37m"

#define ST_DBG(fmt, args...) \
    do { \
        printf(COLOR_GREEN "[DBG]:%s[%d]: " COLOR_NONE, __FUNCTION__,__LINE__); \
        printf(fmt, ##args); \
    }while(0)

#define ST_ERR(fmt, args...) \
	do { \
		printf(COLOR_RED "[ERR]:%s[%d]: " COLOR_NONE, __FUNCTION__,__LINE__); \
		printf(fmt, ##args); \
	}while(0)
#define AD_LOG(fmt, args...) {printf("\033[1;34m");printf("%s[%d]:", __FUNCTION__, __LINE__);printf(fmt, ##args);printf("\033[0m");}

#define MAX_BUF_LEN					256
#define MAX_FRAME_QUEUE_DEPTH       64

typedef struct
{
	struct list_head    frameList;
    unsigned char*      pFrameData;
    int                 frameLen;
} VoiceFrame_t;

typedef struct
{
    list_t wordList;
    int index;
    char szWord[62];
}TrainingWordData_t;

typedef struct
{
	bool bExit;
	pthread_t pt;
} ThreadData_t;

typedef struct
{
	HANDLE hDSpotter;
	VoiceAnalyzeCallback pfnCallback;
} UsrData_t;

static TrainedWord_t g_command[] = {
//static char g_command[][COMMAND_LEN] = {
	"嗨,小星星",
	"打开电灯",
	"关掉电灯",
	"打开窗帘",
	"关闭窗帘",
	"播放音乐",
	"后一首",
	"前一首",
	"停止音乐",
	"打开空调",
	"关闭空调",
	"空调温度调高",
	"空调温度调低",
	"清新模式",
	"睡眠模式",
	"开始煮饭",
	"风大一点",
	"风小一点",
	"音量调大",
	"音量调小",
	"接通电话",
	"开门",
	"挂断电话",
	"查看门口情况",
	"打开客厅的灯",
	"关闭客厅的灯",
	"离家场景",
	"降低速度",
	"降低坡度",
	"提高速度",
	"提高坡度",
	"拍照",
	"今天天气",
	"现在几点",
	"开始",
	"暂停",
	"洗涤模式",
	"脱水模式",
	"侵泡洗衣",
	"轻柔模式",
	"强力模式",
	"摩卡咖啡",
	"拿铁咖啡",
	"美式咖啡",
	"意式咖啡",
	"卡布奇诺"
};

static int g_voiceFrameCnt = 0;
static list_t g_wordListHead;
static list_t g_voiceFrameListHead;
static pthread_mutex_t g_frameMutex;
static ThreadData_t g_stRecogThreadData;
static ThreadData_t g_stAudioInThreadData;
static UsrData_t g_stUsrData;

// print comman list
void PrintCommanList()
{
	int i = 0;
	//int commandNum = sizeof(g_command)/COMMAND_LEN;
	int commandNum = sizeof(g_command)/sizeof(TrainedWord_t);

	printf("Recognized commands:\n");
	for (i = 0; i < commandNum; i++)
	{
		printf("%d: %s\n", i, g_command[i].cmd);
	}
	printf("\n");
}

// training word list opt
void InitTrainingWordList(list_t *listHead)
{
	INIT_LIST_HEAD(listHead);
}

void DeinitTrainingWordList(list_t *listHead)
{
	TrainingWordData_t *pos = NULL;
	TrainingWordData_t *posN = NULL;

    if (!list_empty(listHead))
    {
	    list_for_each_entry_safe(pos, posN, listHead, wordList)
	    {
		    list_del(&pos->wordList);
		    free(pos);
	    }
    }
}

void AddTrainingWord(int index, char *word, list_t *listHead)
{
	TrainingWordData_t *pTrainingWord = (TrainingWordData_t *)malloc(sizeof(TrainingWordData_t));
	memset(pTrainingWord, 0, sizeof(TrainingWordData_t));

	INIT_LIST_HEAD(&pTrainingWord->wordList);
	pTrainingWord->index = index;
	strcpy(pTrainingWord->szWord, word);
	list_add_tail(&pTrainingWord->wordList, listHead);
}

void DeleteTrainingWord(char *word, list_t *listHead)
{
	TrainingWordData_t *pos = NULL;
	TrainingWordData_t *posN = NULL;

	list_for_each_entry_safe(pos, posN, listHead, wordList)
	{
		if (!strcmp(pos->szWord, word))
		{
			list_del(&pos->wordList);
			free(pos);
			break;
		}
	}
}

int FindTrainingWord(char *word, list_t *listHead)
{
	TrainingWordData_t *pos = NULL;

	list_for_each_entry(pos, listHead, wordList)
	{
		if (!strcmp(pos->szWord, word))
		{
			return pos->index;
		}
	}

	return -1;
}

// voice frame list opt
void InitVoiceFrameQueue(void)
{
    INIT_LIST_HEAD(&g_voiceFrameListHead);
    pthread_mutex_init(&g_frameMutex, NULL);
    g_voiceFrameCnt = 0;
}

void DeinitVoiceFrameQueue(void)
{
    VoiceFrame_t *pstVoiceFrame = NULL;
	struct list_head *pListPos = NULL;
	struct list_head *pListPosN = NULL;

    pthread_mutex_lock(&g_frameMutex);
	list_for_each_safe(pListPos, pListPosN, &g_voiceFrameListHead)
	{
		pstVoiceFrame = list_entry(pListPos, VoiceFrame_t, frameList);
		list_del(pListPos);

        if (pstVoiceFrame->pFrameData)
        {
            free(pstVoiceFrame->pFrameData);
        }

		free(pstVoiceFrame);
	}
    pthread_mutex_unlock(&g_frameMutex);
}

VoiceFrame_t* PopVoiceFrameFromQueue(void)
{
    VoiceFrame_t *pstVoiceFrame = NULL;
    struct list_head *pListPos = NULL;

    pthread_mutex_lock(&g_frameMutex);
    if (list_empty(&g_voiceFrameListHead))
    {
        pthread_mutex_unlock(&g_frameMutex);
        return NULL;
    }

    pListPos = g_voiceFrameListHead.next;

    g_voiceFrameListHead.next = pListPos->next;
    pListPos->next->prev = pListPos->prev;
    pthread_mutex_unlock(&g_frameMutex);

    pstVoiceFrame = list_entry(pListPos, VoiceFrame_t, frameList);

    return pstVoiceFrame;
}

void PutVoiceFrameToQueue(MI_AUDIO_Frame_t *pstAudioFrame)
{
    struct list_head *pListPos = NULL;
	struct list_head *pListPosN = NULL;
    VoiceFrame_t *pstVoiceFrame = NULL;
    int queueDepth = 0;

    if (pstAudioFrame == NULL)
    {
        return;
    }

    // calc depth
    pthread_mutex_lock(&g_frameMutex);
    list_for_each_safe(pListPos, pListPosN, &g_voiceFrameListHead)
	{
		queueDepth ++;
	}
    pthread_mutex_unlock(&g_frameMutex);

    // max depth check
    if (queueDepth >= MAX_FRAME_QUEUE_DEPTH)
    {
        // pop frame
        pstVoiceFrame = PopVoiceFrameFromQueue();
        if (pstVoiceFrame != NULL)
        {
            if (pstVoiceFrame->pFrameData != NULL)
            {
                free(pstVoiceFrame->pFrameData);
                pstVoiceFrame->pFrameData = NULL;
            }

            free(pstVoiceFrame);
            pstVoiceFrame = NULL;
        }
    }

    pstVoiceFrame = (VoiceFrame_t *)malloc(sizeof(VoiceFrame_t));
    if (pstVoiceFrame == NULL)
    {
        ST_ERR("malloc error, not enough memory\n");
        goto END;
    }
    memset(pstVoiceFrame, 0, sizeof(VoiceFrame_t));

    pstVoiceFrame->pFrameData = (unsigned char *)malloc(pstAudioFrame->u32Len);
    if (pstVoiceFrame->pFrameData == NULL)
    {

        ST_ERR("malloc error, not enough memory\n");
        goto END;
    }
    memset(pstVoiceFrame->pFrameData, 0, pstAudioFrame->u32Len);
    memcpy(pstVoiceFrame->pFrameData, pstAudioFrame->apVirAddr[0], pstAudioFrame->u32Len);
    pstVoiceFrame->frameLen = pstAudioFrame->u32Len;

    // ST_DBG("pFrameData:%p, frameLen:%d\n", pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen);

    pthread_mutex_lock(&g_frameMutex);
    list_add_tail(&pstVoiceFrame->frameList, &g_voiceFrameListHead);
    g_voiceFrameCnt++;
    pthread_mutex_unlock(&g_frameMutex);

    return;
END:
    if (pstVoiceFrame->pFrameData != NULL)
    {
        free(pstVoiceFrame->pFrameData);
        pstVoiceFrame->pFrameData = NULL;
    }

    if (pstVoiceFrame)
    {
        free(pstVoiceFrame);
        pstVoiceFrame = NULL;
    }
}


// audio in thread
static void *_SSTAR_AudioInGetDataProc_(void *pdata)
{
    MI_AUDIO_DEV AiDevId = AI_DEV_ID;
    MI_AI_CHN AiChn = 0;
    MI_AUDIO_Frame_t stAudioFrame;
    MI_S32 s32ToTalSize = 0;
    MI_S32 s32Ret = 0;
    FILE *pFile = NULL;
    char szFileName[64] = {0,};

    InitVoiceFrameQueue();
    ST_DBG("pid=%d\n", syscall(SYS_gettid));

    MI_SYS_ChnPort_t stChnPort;
    MI_S32 s32Fd = -1;
    fd_set read_fds;
    struct timeval TimeoutVal;

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_AI;
    stChnPort.u32DevId = AiDevId;
    stChnPort.u32ChnId = AiChn;
    stChnPort.u32PortId = 0;
    s32Ret = MI_SYS_GetFd(&stChnPort, &s32Fd);

    if(MI_SUCCESS != s32Ret)
    {
        ST_ERR("MI_SYS_GetFd err:%x, chn:%d\n", s32Ret, AiChn);
        return NULL;
    }

    while(!g_stAudioInThreadData.bExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if(s32Ret < 0)
        {
            ST_ERR("select failed!\n");
            //  usleep(10 * 1000);
            continue;
        }
        else if(s32Ret == 0)
        {
            ST_ERR("get audio in frame time out\n");
            //usleep(10 * 1000);
            continue;
        }
        else
        {
            if(FD_ISSET(s32Fd, &read_fds))
            {
                MI_AI_GetFrame(AiDevId, AiChn, &stAudioFrame, NULL, 128);//1024 / 8000 = 128ms
                if (0 == stAudioFrame.u32Len)
                {
                    usleep(10 * 1000);
                    continue;
                }

                PutVoiceFrameToQueue(&stAudioFrame);		// save

                MI_AI_ReleaseFrame(AiDevId,  AiChn, &stAudioFrame, NULL);
            }
        }
    }

    DeinitVoiceFrameQueue();

    return NULL;
}


// aidev opt
static MI_S32 SSTAR_AudioInStart()
{
    MI_S32 s32Ret = MI_SUCCESS, i;

    //Ai
    MI_AUDIO_DEV AiDevId = AI_DEV_ID;
    MI_AI_CHN AiChn = 0;
    MI_AUDIO_Attr_t stAiSetAttr;
    MI_SYS_ChnPort_t stAiChn0OutputPort0;
    MI_AI_VqeConfig_t stAiVqeConfig;

    //Ao
    MI_AUDIO_DEV AoDevId = AO_DEV_ID;
    MI_AO_CHN AoChn = 0;
    MI_S16 s16CompressionRatioInput[5] = {-70, -60, -30, 0, 0};
    MI_S16 s16CompressionRatioOutput[5] = {-70, -45, -18, 0, 0};


    //set ai attr
    memset(&stAiSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stAiSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiSetAttr.eSamplerate = AUDIO_SAMPLE_RATE;
    stAiSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    //stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_SLAVE;
    stAiSetAttr.u32ChnCnt = 2;
    //stAiSetAttr.u32ChnCnt = 4;
    stAiSetAttr.u32FrmNum = 16;
    stAiSetAttr.u32PtNumPerFrm = AUDIO_PT_NUMBER_FRAME;

    //set ai output port depth
    memset(&stAiChn0OutputPort0, 0, sizeof(MI_SYS_ChnPort_t));
    stAiChn0OutputPort0.eModId = E_MI_MODULE_ID_AI;
    stAiChn0OutputPort0.u32DevId = AiDevId;
    stAiChn0OutputPort0.u32ChnId = AiChn;
    stAiChn0OutputPort0.u32PortId = 0;

    //ai vqe
    memset(&stAiVqeConfig, 0, sizeof(MI_AI_VqeConfig_t));
    stAiVqeConfig.bHpfOpen = FALSE;
    stAiVqeConfig.bAnrOpen = FALSE;
    stAiVqeConfig.bAgcOpen = TRUE;
    stAiVqeConfig.bEqOpen = FALSE;
    //stAiVqeConfig.bAecOpen = FALSE;
    stAiVqeConfig.bAecOpen = TRUE;

    stAiVqeConfig.s32FrameSample = 128;
    stAiVqeConfig.s32WorkSampleRate = AUDIO_SAMPLE_RATE;

    //Hpf
    stAiVqeConfig.stHpfCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
    stAiVqeConfig.stHpfCfg.eHpfFreq = E_MI_AUDIO_HPF_FREQ_120;

    //Anr
    stAiVqeConfig.stAnrCfg.eMode= E_MI_AUDIO_ALGORITHM_MODE_USER;
    stAiVqeConfig.stAnrCfg.eNrSpeed = E_MI_AUDIO_NR_SPEED_LOW;
    stAiVqeConfig.stAnrCfg.u32NrIntensity = 5;            //[0, 30]
    stAiVqeConfig.stAnrCfg.u32NrSmoothLevel = 10;          //[0, 10]

    //Agc
    stAiVqeConfig.stAgcCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
    stAiVqeConfig.stAgcCfg.s32NoiseGateDb = -30;           //[-80, 0], NoiseGateDb disable when value = -80
    stAiVqeConfig.stAgcCfg.s32TargetLevelDb =   0;       //[-80, 0]
    stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit = 1;  //[-20, 30]
    stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax =  15; //[0, 30]
    stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin = -5; //[-20, 30]
    stAiVqeConfig.stAgcCfg.u32AttackTime = 1;              //[1, 20]
    memcpy(stAiVqeConfig.stAgcCfg.s16Compression_ratio_input, s16CompressionRatioInput, sizeof(s16CompressionRatioInput));
    memcpy(stAiVqeConfig.stAgcCfg.s16Compression_ratio_output, s16CompressionRatioOutput, sizeof(s16CompressionRatioOutput));
    stAiVqeConfig.stAgcCfg.u32DropGainMax = 60;            //[0, 60]
    stAiVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb = 10;  //[0, 100]
    stAiVqeConfig.stAgcCfg.u32ReleaseTime = 10;             //[1, 20]
    stAiVqeConfig.u32ChnNum = 1;
    //Eq
    stAiVqeConfig.stEqCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
    for (i = 0; i < sizeof(stAiVqeConfig.stEqCfg.s16EqGainDb) / sizeof(stAiVqeConfig.stEqCfg.s16EqGainDb[0]); i++)
    {
       stAiVqeConfig.stEqCfg.s16EqGainDb[i] = 5;
    }

    // aec
    memset(&stAiVqeConfig.stAecCfg, 0, sizeof(MI_AI_AecConfig_t));
    stAiVqeConfig.stAecCfg.u32AecSupfreq[0] = 20;
    stAiVqeConfig.stAecCfg.u32AecSupfreq[1] = 40;
    stAiVqeConfig.stAecCfg.u32AecSupfreq[2] = 60;
    stAiVqeConfig.stAecCfg.u32AecSupfreq[3] = 80;
    stAiVqeConfig.stAecCfg.u32AecSupfreq[4] = 100;
    stAiVqeConfig.stAecCfg.u32AecSupfreq[5] = 120;
    for (i = 0; i < sizeof(stAiVqeConfig.stAecCfg.u32AecSupIntensity) / sizeof(stAiVqeConfig.stAecCfg.u32AecSupIntensity[0]); i++)
	{
    	stAiVqeConfig.stAecCfg.u32AecSupIntensity[i] = 4;
	}

    ExecFunc(MI_AI_SetPubAttr(AiDevId, &stAiSetAttr), MI_SUCCESS);
    ExecFunc(MI_AI_Enable(AiDevId), MI_SUCCESS);
    ExecFunc(MI_AI_EnableChn(AiDevId, AiChn), MI_SUCCESS);

#if 1
#if USE_AMIC
    ExecFunc(MI_AI_SetVqeVolume(AiDevId, 0, 9), MI_SUCCESS);
#else
    ExecFunc(MI_AI_SetVqeVolume(AiDevId, 0, 4), MI_SUCCESS);
#endif

    s32Ret = MI_AI_SetVqeAttr(AiDevId, AiChn, AoDevId, AoChn, &stAiVqeConfig);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("%#x\n", s32Ret);
    }
    ExecFunc(MI_AI_EnableVqe(AiDevId, AiChn), MI_SUCCESS);

#endif

    for (i = 0; i < stAiSetAttr.u32ChnCnt; i++)
    {
        stAiChn0OutputPort0.u32ChnId = i;
        ExecFunc(MI_SYS_SetChnOutputPortDepth(&stAiChn0OutputPort0, 4, 8), MI_SUCCESS);
    }

    g_stAudioInThreadData.bExit = false;
    s32Ret = pthread_create(&g_stAudioInThreadData.pt, NULL, _SSTAR_AudioInGetDataProc_, NULL);
    if(0 != s32Ret)
    {
         ST_ERR("create thread failed\n");
         return -1;
    }

    return MI_SUCCESS;
}

static MI_S32 SSTAR_AudioInStop()
{
    MI_AUDIO_DEV AiDevId = AI_DEV_ID;
    MI_AI_CHN AiChn = 0;

    g_stAudioInThreadData.bExit = true;
    pthread_join(g_stAudioInThreadData.pt, NULL);

    ExecFunc(MI_AI_DisableVqe(AiDevId, AiChn), MI_SUCCESS);
    ExecFunc(MI_AI_DisableChn(AiDevId, AiChn), MI_SUCCESS);
    ExecFunc(MI_AI_Disable(AiDevId), MI_SUCCESS);

    return MI_SUCCESS;
}

// get trained word list
int SSTAR_VoiceDetectGetWordList(TrainedWord_t *pWordList, int nWordCnt)
{
	int nCount = sizeof(g_command)/sizeof(TrainedWord_t);

	if (nCount > nWordCnt)
		nCount = nWordCnt;

	memset(pWordList, 0, sizeof(pWordList)*nWordCnt);

	for (int i = 0; i < nCount; i ++)
	{
		memcpy(&pWordList[i], &g_command[i], sizeof(TrainedWord_t));
	}

	return nCount;
}

// get command from commanBin, save command list
HANDLE SSTAR_VoiceDetectInit()
{
	INT nErr = 0;

	HANDLE hDSpotter = NULL;
	char szBaseBinFile[MAX_BUF_LEN];
	char szCommandBinFile[MAX_BUF_LEN];
	char szLicenseBinFile[MAX_BUF_LEN];
	char *pModelFile[1];

	//InitTrainingWordList(&g_wordListHead);
	PrintCommanList();

	sprintf(szBaseBinFile, "%s/CYBase.mod", DSPOTTER_DATA_PATH);
	sprintf(szCommandBinFile, "%s/Group_1.mod", DSPOTTER_DATA_PATH);
	sprintf(szLicenseBinFile, "%s/CybLicense.bin", DSPOTTER_DATA_PATH);

    printf("baseBin path: %s\n", szBaseBinFile);
    printf("commandBin path: %s\n", szCommandBinFile);
    printf("licenseBin path: %s\n", szLicenseBinFile);

	pModelFile[0] = szCommandBinFile;

	hDSpotter = DSpotterInitMultiWithMod(szBaseBinFile, pModelFile, 1, 1000, NULL, 0, &nErr, szLicenseBinFile);
	if (!hDSpotter)
	{
		printf("%s[%d]: failed to initialize DSpotter, nErr=%d\n", __FUNCTION__, __LINE__, nErr);
		return NULL;
	}

    return hDSpotter;
}

int SSTAR_VoiceDetectDeinit(HANDLE hDSpotter)
{
	DSpotterRelease(hDSpotter);

	return 0;
}

static void *_SSTAR_VoiceAnalyzeProc_(void *pData)
{
    VoiceFrame_t *pstVoiceFrame = NULL;
	//HANDLE hDSpotter = (HANDLE)pdata;
	UsrData_t *pUsrData = (UsrData_t*)pData;
    MI_S32 s32Ret = 0;
    INT nResultID;

    AD_LOG("Enter _SSTAR_VoiceAnalyzeProc_\n");

    if ((s32Ret = DSpotterReset(pUsrData->hDSpotter)) != DSPOTTER_SUCCESS)
    {
        printf("DSpotterReset:: Fail to start recognition (%d)\n", s32Ret);
        return NULL;
    }

    InitVoiceFrameQueue();

    while (!g_stRecogThreadData.bExit)
    {
        pstVoiceFrame = PopVoiceFrameFromQueue();

        if (!pstVoiceFrame)
        {
            usleep(1000*10);
            continue;
        }

       s32Ret = DSpotterAddSample(pUsrData->hDSpotter, (short*)pstVoiceFrame->pFrameData, pstVoiceFrame->frameLen/sizeof(short));
       if (s32Ret == DSPOTTER_SUCCESS)
       {
            //printf("Get result\n");

            if ((nResultID = DSpotterGetResult(pUsrData->hDSpotter)) >= 0)
            {
                printf("Get command: id=%d, %s\n", nResultID, g_command[nResultID].cmd);

                pUsrData->pfnCallback(nResultID);
            }
       }

        if (pstVoiceFrame != NULL)
        {
            if (pstVoiceFrame->pFrameData != NULL)
            {
                free(pstVoiceFrame->pFrameData);
                pstVoiceFrame->pFrameData = NULL;
            }

            free(pstVoiceFrame);
            pstVoiceFrame = NULL;
        }

    }

    DeinitVoiceFrameQueue();

    return NULL;
}

int SSTAR_VoiceDetectStart(HANDLE hDSpotter, VoiceAnalyzeCallback pfnCallback)
{
	if (MI_SUCCESS != SSTAR_AudioInStart())
	{
		printf("aidev init failed\n");
		return -1;
	}

	g_stRecogThreadData.bExit = false;
	g_stUsrData.hDSpotter = hDSpotter;
	g_stUsrData.pfnCallback = pfnCallback;
	//pthread_create(&g_stRecogThreadData.pt, NULL, _SSTAR_VoiceAnalyzeProc_, (void*)hDSpotter);
	pthread_create(&g_stRecogThreadData.pt, NULL, _SSTAR_VoiceAnalyzeProc_, (void*)&g_stUsrData);

	return 0;
}

void SSTAR_VoiceDetectStop()
{
	if (!g_stRecogThreadData.bExit)
	{
		g_stRecogThreadData.bExit = true;
		pthread_join(g_stRecogThreadData.pt, NULL);
	}

	SSTAR_AudioInStop();
}
