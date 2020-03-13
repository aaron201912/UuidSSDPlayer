#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/ioctl.h>

//#include <wifiInfo.h>
#include <addon.h>
#include <mdlna.h>
#include <airplayer.h>
#include <UtilS_SPS_PPS.h>
#include <MSrv_Airplay_Player.h>
#include "mi_wlan.h"

#define VDEC_INPUT_WIDTH1        1920
#define VDEC_INPUT_HEIGHT1       1080
#define AIRPLAY			0
#define DLNA			1
//-------------------------------------------
#define SAVE_264_FILE            0
#define SAVE_PCM_FILE            0
#define MAX_AU_DELAY 			 100

#if SAVE_264_FILE
FILE *FP_H264 = NULL;
#endif
#if SAVE_PCM_FILE
FILE *FP_Pcm = NULL;
#endif
//-------------------------------------------
#define MPLAYER_IDLE      		0
#define MPLAYER_AIRPLAY   		1
#define MPLAYER_DLNA            2
int nPlayerStatuc = MPLAYER_IDLE;
//-------------------------------------------
int nStartMediaStatuc = -1;
int nPayLoadAuProcess = MAX_AU_DELAY;

//-------------------------------------------
typedef int  (*fpStartMediaServer)(char *friendname,char *dllpath, char *keypath ,int width, int height, int framerate,char *activecode,airplay_callbacks_t *cb);
typedef void (*fpStopMediaServer)(void);
typedef void (*fpSetAirplayExtraParam)(void * parma);
typedef int (*fpZmDLNAServiceStart)(char * friendly_name, dlna_callbacks_t *cb);
typedef int (*fpZmDLNAServiceClose)(void);

void *m_pLibHandle = NULL;
void *m_pLibdlnaHandle = NULL;
fpStartMediaServer 		  fStartMediaServerFuc = NULL;
fpSetAirplayExtraParam    fSetAirplayExtraParam = NULL;
fpZmDLNAServiceStart fsZmDLNAServiceStart = NULL;

//-------------------------------------------
int gVedioWidth = 0;
int gVedioHeight = 0;

static ss_player_t margs1,margs2;
static pthread_t g_thid_pcm = 0;


//-----------------------------------------------DLNA-------------------------------------------------------------------------
/*--------------------------------------------
 write dlna data
 return:succeed->ret,Failure->-1
**-------------------------------------------*/
int sstar_send_stream(const char *data, int size, int pts)
{
	int ret = 0;
    MI_VDEC_VideoStream_t stVdecStream;
    const char start_code[4] = {0, 0, 0, 1};
    //printf("size, %d, data: %x,%x,%x,%x,%x,%x,%x,%x\n",size,
    		//data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
    stVdecStream.pu8Addr      = data;
    stVdecStream.u32Len       = size;
    stVdecStream.u64PTS       = pts;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = 0;
    if(MI_SUCCESS != (ret = MI_VDEC_SendStream(0, &stVdecStream, 0)))
    {
        printf("[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return ret;
}

/*-----------------------------------------------

-----------------------------------------------*/
static void *sstar_play_pthread(void *arg)
{
	ss_player_t *args = (ss_player_t *)arg;
    AVFormatContext *format_ctx = NULL;
    AVDictionary *format_opts = NULL;
    AVPacket *packet = av_malloc(sizeof(AVPacket));
    int ret = 0, video_idx;

    printf("get in sstar_play_pthread!\n");
    printf("try to play %s ...\n", args->fp);

replay:
    format_ctx = avformat_alloc_context();
    if (format_ctx == NULL)
    {
        printf("avformat_alloc_context failed!\n");
        goto error;
    }

    av_dict_set(&format_opts, "stimeout", "5000000", 0);
    av_dict_set(&format_opts, "http_transport",  "http", 0);
    //av_dict_set(&format_opts, "rtsp_transport",  "tcp", 0);

    //start player
    if ((ret = avformat_open_input(&format_ctx, args->fp, NULL, &format_opts)) != 0) {
        printf("avformat_open_input failed!\n");
        goto error;
    }

    if ((ret = avformat_find_stream_info(format_ctx, NULL)) < 0)
    {
        printf("avformat_find_stream_info() failed!\n");
        goto error;
    }

    for (int i = 0; i < (int)format_ctx->nb_streams; i ++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_idx = i;
            printf("Find a video stream, index %d\n", video_idx);
        }
    }
	Ss_UI_Close();
    while (args->exit!=1)
    {
        if ((ret = av_read_frame(format_ctx, packet)) >= 0)
        {
            if (packet->stream_index == video_idx || format_ctx->nb_streams == 0)
            {
                sstar_send_stream(packet->data, packet->size, packet->pts);
            }
            av_packet_unref(packet);
        }
        else
        {
            avformat_close_input(&format_ctx);
            av_dict_free(&format_opts);
            printf("play again\n");
            goto replay;
        }

        usleep(10 * 1000);
    }

error:
    av_free(packet);
    if (format_ctx != NULL)
    {
        avformat_close_input(&format_ctx);
    }
    if (format_opts != NULL)
    {
        av_dict_free(&format_opts);
    }
    if (ret < 0)
    {
        printf("[code %d]occur error!\n", ret);
    }

    printf("exit sstar_play_pthread normal!\n");

    return NULL;
}

pthread_t play_tid = 0;
char g_ur_site[1024] = {0};

void Ss_DLNA_pthread_close(void)
{
	if(play_tid != 0)
	{
		margs1.exit = 1;
		pthread_join(play_tid, NULL);
		play_tid = 0;
	}
	nPlayerStatuc = MPLAYER_IDLE;
	//deinit sdk
	Ss_Player_DeInit(0);
}

void mDLNAPlayback_Open(char *url,char *MetaData, float fPosition)
{
	if(nPlayerStatuc == MPLAYER_AIRPLAY)
	{
		return;
	}
	if(play_tid !=0 && nPlayerStatuc == MPLAYER_DLNA)
	{
		Ss_DLNA_pthread_close();
	}
	memset(&g_ur_site, 0, sizeof(g_ur_site));
	memcpy(g_ur_site, url, strlen(url));
	printf("g_ur_site: %s\n",g_ur_site);
	printf("###mDLNAPlayback_Open[%s]\n\n[%s]###\n",url,MetaData);
    margs1.fp = g_ur_site;
    margs1.exit = 0;
	//int sdk
	//Sstar_DLNA_init();
	Ss_Player_Init(VDEC_INPUT_WIDTH1,VDEC_INPUT_HEIGHT1,1);
    pthread_create(&play_tid, NULL, sstar_play_pthread, &margs1);
    nPlayerStatuc = MPLAYER_DLNA;
}

void mDLNAPlayback_Play()
{
	printf("###mDLNAPlayback_Play ###\n");

}

void mDLNAPlayback_Pause()
{
	printf("###mDLNAPlayback_Pause ###\n");
}

void mDLNAPlayback_Stop()
{
	printf("###mDLNAPlayback_Stop ###\n");
	Ss_DLNA_pthread_close();
	Ss_UI_Open();
	nPlayerStatuc = MPLAYER_IDLE;
}
void mDLNAPlayback_Seek(long fPosition)
{
	printf("###mDLNAPlayback_Seek ###\n");

}
void mDLNAPlayback_SetVolume(int volume)
{
	printf("###mDLNAPlayback_SetVolume[%d] ###\n",volume);

}

long mDLNAPlayback_GetDuration()
{
	return 0;
}

long mDLNAPlayback_GetPostion()
{
	return 0;

}
int  mDLNAPlayback_IsPlaying()
{
	return 1;
}
int  mDLNAPlayPlayback_IsPaused()
{
	return 0;
}
int  mDLNAPlayPlayback_IsStopped()
{
	return 0;
}

int Ss_DLNA_ServiceClose(void)
{
	fpZmDLNAServiceClose fsZmDLNAServiceClose=  (fpZmDLNAServiceClose)dlsym(m_pLibHandle, "ZmDLNAServiceClose");
	if(fsZmDLNAServiceClose)
	{
		fsZmDLNAServiceClose();
		Ss_DLNA_pthread_close();
	}
	//Ss_Player_DeInit(0);
	nPlayerStatuc = MPLAYER_IDLE;
	return 0;
}

int Ss_DLNA_ServiceStart(void)
{
    char *dlaname = (char*)"/customer/lib/libmdlna.so";
    margs2.pcm_exit = 0;
    printf(" reception\n");
	dlna_callbacks_t uo;
    if(1)
    {
        signal(SIGPIPE, SIG_IGN);

		memset(&uo, 0, sizeof(dlna_callbacks_t));
		uo.DLNAPlayback_Open = mDLNAPlayback_Open;
		uo.DLNAPlayback_Play = mDLNAPlayback_Play;
		uo.DLNAPlayback_Pause = mDLNAPlayback_Pause;
		uo.DLNAPlayback_Stop = mDLNAPlayback_Stop;
		uo.DLNAPlayback_SetVolume = mDLNAPlayback_SetVolume;
		uo.DLNAPlayback_GetDuration = mDLNAPlayback_GetDuration;
		uo.DLNAPlayback_GetPostion = mDLNAPlayback_GetPostion;
		uo.DLNAPlayback_IsPlaying = mDLNAPlayback_IsPlaying;
		uo.DLNAPlayPlayback_IsPaused = mDLNAPlayPlayback_IsPaused;
		uo.DLNAPlayPlayback_IsStopped = mDLNAPlayPlayback_IsStopped;
    }
    else
    {
        return -1;
    }

    if(m_pLibdlnaHandle == NULL)
    {
		m_pLibdlnaHandle = dlopen(dlaname, RTLD_LAZY);
	}
    if(m_pLibdlnaHandle != NULL)
    {
        fsZmDLNAServiceStart  = (fpZmDLNAServiceStart)dlsym(m_pLibdlnaHandle, "ZmDLNAServiceStart");
        if (dlerror() != NULL)
        {
            printf("!!!warning dlsym(ZmDLNAServiceStart) failed.\n");
        }
        else
        {
		  	printf("succeed\n");
		 	nStartMediaStatuc = fsZmDLNAServiceStart((char*)"NewLink-ff88(dlna)",&uo);
		 	printf("nStartMediaStatuc = %d\n",nStartMediaStatuc);
        }
    }
    else
    {
        char *error = dlerror();
        if(error != NULL)
        {
            printf("!!!warning(%s) open fail| error.\n");
        }
        else
        {
            printf("!!!warning open fail.\n");
        }
        return -1;
    }
	return nStartMediaStatuc;
}

//--------------------------------------------------------------------------Airplay-------------------------------------------------------

//Gets the width and height of h264
int GetH264VedioWidth()
{
	return gVedioWidth;
}

int GetH264VedioHeight()
{
	return gVedioHeight;
}

bool IsH264ModeChange(unsigned char *buf, int spslen)
{
	bool result = false;
	SPS sps_buf;
	get_bit_context bitcontext;
	memset(&bitcontext,0x00,sizeof(get_bit_context));
	bitcontext.buf = buf + 5;
	bitcontext.buf_size = spslen-1;
	h264dec_seq_parameter_set(&bitcontext, &sps_buf);
	int width = h264_get_width(&sps_buf);
	int height = h264_get_height(&sps_buf);
	printf("########IsH264ModeChange:(%d,%d),(%d,%d)########\r\n",width,height,gVedioWidth,gVedioHeight);
	if((gVedioWidth != width) ||(gVedioHeight != height) )
	{
		gVedioWidth = width;
		gVedioHeight = height;
		result = true;
	}
	return result;
}

bool IsResolutionSupport()
{
	if((gVedioWidth*gVedioHeight)<= 1920*1088)
	{
		return true;
	}
	else
	{
		return false;
	}
}


// The thread that listens to the audio cache
static void *Pcm_bufloop(void *arg)
{
	return NULL;
	MI_AO_ChnState_t AO_ChnState_t;
	ss_player_t *args = (ss_player_t *)arg;
	printf("###Pcm_bufloop1 = %d\n",args->pcm_exit);
	while (args->pcm_exit == 0)
	{
		MI_AO_QueryChnStat(0,0,&AO_ChnState_t);
		//printf("u32ChnTotalNum = %d ,u32ChnFreeNum = %d ,u32ChnBusyNum  = %d \n",AO_ChnState_t.u32ChnTotalNum,
				//AO_ChnState_t.u32ChnFreeNum,AO_ChnState_t.u32ChnBusyNum);
		usleep(1000*1000);
		printf("margs2.pcm_exit[%d]\n",args->pcm_exit);
	}
	printf("###Pcm_bufloop2 = %d\n",args->pcm_exit);
	//return NULL;
}

// Create Pcm_buf Pthread
static void Ss_Pthread_Pcm(void)
{
	margs2.pcm_exit = 0;
	if((pthread_create(&g_thid_pcm, NULL, Pcm_bufloop, &margs2))!= 0)
	{
		printf("pthread_create error.\n");
		return -1;
	}
}

// finish  Pcm_buf Pthread
static void Ss_pthread_finish(void)
{
	printf("###Enter Ss_pthread_finish###\n");
	if(g_thid_pcm != 0)
	{
		margs2.pcm_exit = 1;
		pthread_join(g_thid_pcm, NULL);
		g_thid_pcm = 0;
	}
	printf("###Leave Ss_pthread_finish###\n");
}

void VideoMirroringOpen(void *cls, int width, int height, const void *buffer, int buflen, int payloadtype, double timestamp)
{
	int ret = 0;
	int spscnt =0,spsnalsize=0,ppscnt=0,ppsnalsize=0;
	MI_VDEC_VideoStream_t stVdecStream;
	MI_U32 s32Ret;

#if SAVE_264_FILE
	printf("start open test.h264");
	FP_H264 = fopen("/customer/test.h264", "wb");
    if(FP_H264 == NULL)
    {
        fprintf(stderr,"=====FP_H264 null=======");
        return;
    }
#endif

	// get SPS
    unsigned    char *head = (unsigned  char *)buffer;
    spscnt = head[5] & 0x1f;
    spsnalsize = ((uint32_t)head[6] << 8) | ((uint32_t)head[7]);
    ppscnt = head[8 + spsnalsize];
    ppsnalsize = ((uint32_t)head[9 + spsnalsize] << 8) | ((uint32_t)head[10 + spsnalsize]);
    unsigned char *data = (unsigned char *)malloc(4 + spsnalsize + 4 + ppsnalsize);
	printf("sps size[%d,%d,%d,%d]\r\n",spsnalsize,ppsnalsize,spscnt,ppscnt);
	if(data == NULL)
    {
        fprintf(stderr,"=====error mallocfail=======\n");
        return;
    }
	data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 1;
	memcpy(data + 4, head + 8, spsnalsize);
    data[4 + spsnalsize] = 0;
    data[5 + spsnalsize] = 0;
    data[6 + spsnalsize] = 0;
    data[7 + spsnalsize] = 1;
    memcpy(data + 8 + spsnalsize, head + 11 + spsnalsize, ppsnalsize);
	printf("VideoMirroringOpen");
	
#if SAVE_264_FILE
	ret = fwrite(data,1,4 + spsnalsize + 4 + ppsnalsize,FP_H264);
	printf("ret = %d\n",ret);
#endif

	IsH264ModeChange(data,spsnalsize);

	Ss_Player_Init(gVedioWidth,gVedioHeight,1);
	printf("width = %d , hight = %d \n",gVedioWidth,gVedioHeight);
	printf("start playing\n");
	stVdecStream.pu8Addr      = (unsigned char*)data;
    stVdecStream.u32Len       = 4 + spsnalsize + 4 + ppsnalsize;
    stVdecStream.u64PTS       = 0;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = 0;
	usleep(10);
	//printf("data: %x,%x,%x,%x,%x\n",stVdecStream.pu8Addr[0],stVdecStream.pu8Addr[1],stVdecStream.pu8Addr[2],stVdecStream.pu8Addr[3],stVdecStream.pu8Addr[4]);
	if(MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(0, &stVdecStream, 0)))
	{
        printf("[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
        return;
	}
	Ss_UI_Close();
	Ss_Pthread_Pcm();
	free(data);
}


// Airplay play h264 video and audio streams
void VideoMirroringProcess(void *cls, const void *buffer, int buflen, int payloadtype, double timestamp)
{
  if(nPlayerStatuc == MPLAYER_DLNA)
  {
	return ;
  }
  if (payloadtype == 0)
  {
	   int rLen = 0;
	   unsigned char *head;
	   unsigned char *data = (unsigned char *)buffer;//(unsigned char *)malloc(buflen);
	   head = (unsigned char *)data + rLen;
	   while (rLen < buflen)
	   {
	       rLen += 4;
	       rLen += (((uint32_t)head[0] << 24) | ((uint32_t)head[1] << 16) | ((uint32_t)head[2] << 8) | (uint32_t)head[3]);
	       head[0] = 0;
	       head[1] = 0;
	       head[2] = 0;
	       head[3] = 1;
	       head = (unsigned char *)data + rLen;
	   }

#if SAVE_264_FILE
	fwrite(data, 1, buflen, FP_H264);
#endif

	MI_VDEC_VideoStream_t stVdecStream;
	MI_U32 s32Ret;
	
	stVdecStream.pu8Addr      = (unsigned char*)data;
    stVdecStream.u32Len       = buflen;
    stVdecStream.u64PTS       = 0;
    stVdecStream.bEndOfFrame  = 1;
    stVdecStream.bEndOfStream = 0;
	usleep(10);
	//printf("data: %x,%x,%x,%x,%x\n",stVdecStream.pu8Addr[0],stVdecStream.pu8Addr[1],stVdecStream.pu8Addr[2],stVdecStream.pu8Addr[3],stVdecStream.pu8Addr[4]);
	if(MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(0, &stVdecStream, 0)))
    {
        printf("[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
        return;
    }
   }
 	else if (payloadtype == 1)
    {
		// int spscnt;
        int spsnalsize = 0;
        //int ppscnt;
        int ppsnalsize = 0;
        printf("=====vmp_pload====%f,%d,%d====\n",timestamp,buflen,2);
        unsigned    char *head = (unsigned  char *)buffer;
       // spscnt = head[5] & 0x1f;
        spsnalsize = ((uint32_t)head[6] << 8) | ((uint32_t)head[7]);
        //ppscnt = head[8 + spsnalsize];
        ppsnalsize = ((uint32_t)head[9 + spsnalsize] << 8) | ((uint32_t)head[10 + spsnalsize]);
        unsigned char *data = (unsigned char *)malloc(4 + spsnalsize + 4 + ppsnalsize);
        if(data == NULL)
        {
            printf("######payloadtype data == null######\r\n");
            return;
        }
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 1;
        memcpy(data + 4, head + 8, spsnalsize);
        data[4 + spsnalsize] = 0;
        data[5 + spsnalsize] = 0;
        data[6 + spsnalsize] = 0;
        data[7 + spsnalsize] = 1;
        memcpy(data + 8 + spsnalsize, head + 11 + spsnalsize, ppsnalsize);

#if SAVE_264_FILE
		fwrite(data,1,4 + spsnalsize + 4 + ppsnalsize,FP_H264);
#endif

		if(IsH264ModeChange(data,spsnalsize))
		{
			Ss_Player_DeInit(0);
			printf("width = %d , hight = %d \n",gVedioWidth,gVedioHeight);
			Ss_Player_Init(gVedioWidth,gVedioHeight,0);

			MI_VDEC_VideoStream_t stVdecStream;
			MI_U32 s32Ret;

			stVdecStream.pu8Addr      = data;
		    stVdecStream.u32Len       = 4 + spsnalsize + 4 + ppsnalsize;
		    stVdecStream.u64PTS       = 0;
		    stVdecStream.bEndOfFrame  = 1;
		    stVdecStream.bEndOfStream = 0;
			usleep(10);
			//printf("data: %x,%x,%x,%x,%x\n",stVdecStream.pu8Addr[0],stVdecStream.pu8Addr[1],stVdecStream.pu8Addr[2],stVdecStream.pu8Addr[3],stVdecStream.pu8Addr[4]);
			if(MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(0, &stVdecStream, 0)))
		    {
		        printf("[%s %d]MI_VDEC_SendStream failed!\n", __FUNCTION__, __LINE__);
		        return;
		    }
		}
		free(data);
	}
 	nPlayerStatuc = MPLAYER_AIRPLAY;
}

void VideoMirroringStop(void *cls)
{
	printf("=====video_mirroring_stop=1=======\r\n");

#if SAVE_264_FILE
	if(FP_H264)
    {
        fclose(FP_H264);
    }
#endif

	Ss_Player_DeInit(0);
	//Ss_pthread_finish();
	printf("=====video_mirroring_stop=2=======\r\n");
	Ss_UI_Open();
	nPlayerStatuc = MPLAYER_IDLE;
	printf("=====video_mirroring_stop=3=======\r\n");
}


void AudioOpen(void *cls, int bits, int channels, int samplerate, int isaudio)
{

#if SAVE_PCM_FILE
	FP_Pcm = fopen("/customer/test.pcm", "wb");
	if(FP_Pcm == NULL)
    {
        printf("FP_Pcm null.\n");
    }
#endif

    printf("\n=====audio_open(bits:%d,Channel:%d,sr:%d,au:%d)",bits,channels,samplerate,isaudio);
}

void AudioSetVolume(void *cls, int volume)
{
   MI_AUDIO_DEV AoDevId = 0;
   volume = volume - 60;
   if(volume > 30)
   {
		volume = 30;
   }
   printf("=====audio_setvolume====%d====",volume);
   MI_AO_SetVolume(AoDevId, volume);
}

void AudioProcess(void *cls, const void *buffer, int buflen, double timestamp, uint32_t seqnum)
{
	#if SAVE_PCM_FILE
    if(FP_Pcm)
    {
        fwrite(buffer,1,buflen,FP_Pcm);
    }
	#endif

	MI_AUDIO_Frame_t stAoSendFrame;
	MI_S32 s32RetSendStatus = 0;
	MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;

	stAoSendFrame.u32Len = buflen;
    stAoSendFrame.apVirAddr[0] = (void *)buffer;
    stAoSendFrame.apVirAddr[1] = NULL;
	s32RetSendStatus = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, 0);
	if(s32RetSendStatus != MI_SUCCESS)
	{
		printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
	}
}

void AudioFlush(void *cls)
{
	printf("=====audio_flush====\n");
#if SAVE_PCM_FILE
	if(FP_Pcm)
    {
        fclose(FP_Pcm);
    }
#endif
}

void AudioMetadata(void *cls, const void *buffer, int buflen)
{

}

void AudioCoverart(void *cls, const void *buffer, int buflen)
{

}

void AudioStop(void *cls)
{

}

void AirplayOpen(void *cls, char *url, float fPosition)
{

}

void AirplayPlay(void *cls)
{

}

void AirplayPause(void *cls)
{

}

void AirplayStop(void *cls)
{

}

void AirplaySeek(void *cls, long fPosition)
{

}

void AirplaySetVolume(void *cls, int volume)
{

}

void AirplayShowPhoto(void *cls, unsigned char *data, long long size)
{

}

long AirplayGetDuration(void *cls)
{
	return 240 * 1000;
}

long AirplayGetPosition(void *cls)
{
	return 0;
}

int AirplayIsPlaying(void *cls)
{
	return 0;
}

int AirplayIsPaused(void *cls)
{
	return 0;
}


int AirplayIsStopped(void *cls)
{
	return 0;
}


void Airplay_setosversion(void *cls,double osversion)
{

}


 int AirplayServiceStart(char * pickname)
{
    int nMWeight = 1280;
    int nMHeight = 720;
    char *dllname = (char*)"/customer/lib/libmediaserver.so";
    char *keypath = (char*)"/customer/zyfly/xindawn.jpg";
    margs1.pcm_exit = 0;
    printf(" reception\n");
    airplay_callbacks_t ao;
    MI_DISP_VideoLayerAttr_t stVideoLayerPara;

    if (MI_SUCCESS != MI_DISP_GetVideoLayerAttr(0, &stVideoLayerPara))
    {
    	printf("PANEL PARA GET ERR!\n");

    	return -1;
    }
    nMWeight = (int)stVideoLayerPara.stVidLayerSize.u16Width;
    nMHeight = (int)stVideoLayerPara.stVidLayerSize.u16Height;
    Ss_Player_SetlayerSize(nMWeight, nMHeight);
    printf("Get panel w %d h %d\n", nMWeight, nMHeight);
    if(1)
    {
        signal(SIGPIPE, SIG_IGN);
        memset(&ao,0,sizeof(airplay_callbacks_t));

        ao.cls                          = NULL;
        ao.AirPlayMirroring_Play        = VideoMirroringOpen;
        ao.AirPlayMirroring_Process     = VideoMirroringProcess;
        ao.AirPlayMirroring_Stop        = VideoMirroringStop;

        ao.AirPlayAudio_Init            = AudioOpen;
        ao.AirPlayAudio_SetVolume       = AudioSetVolume;
        ao.AirPlayAudio_Process         = AudioProcess;
        ao.AirPlayAudio_destroy         = AudioStop;
        ao.AirPlayAudio_SetMetadata     = AudioMetadata;
        ao.AirPlayAudio_SetCoverart     = AudioCoverart;
        ao.AirPlayAudio_Flush           = AudioFlush;
        ao.AirPlayPlayback_Open         = AirplayOpen;
        ao.AirPlayPlayback_Play         = AirplayPlay;
        ao.AirPlayPlayback_Pause        = AirplayPause;
        ao.AirPlayPlayback_Stop         = AirplayStop;
        ao.AirPlayPlayback_Seek         = AirplaySeek;
        ao.AirPlayPlayback_SetVolume    = AirplaySetVolume;
        ao.AirPlayPlayback_ShowPhoto    = AirplayShowPhoto;
        ao.AirPlayPlayback_GetDuration  = AirplayGetDuration;
        ao.AirPlayPlayback_GetPostion   = AirplayGetPosition;
        ao.AirPlayPlayback_IsPlaying    = AirplayIsPlaying;
        ao.AirPlayPlayback_IsPaused     = AirplayIsPaused;
        ao.AirPlayPlayback_IsStopped    = AirplayIsStopped;
        ao.AirPlay_SetClientOsVersion   = Airplay_setosversion;
    }
    else
    {
        return -1;
    }

    if(m_pLibHandle == NULL)
    {
        m_pLibHandle = dlopen(dllname, RTLD_LAZY);
    }
    if(m_pLibHandle != NULL)
    {
        fStartMediaServerFuc  = (fpStartMediaServer)dlsym(m_pLibHandle, "XinDawn_StartMediaServer");
        if (dlerror() != NULL)
        {
            printf("!!!warning dlsym(XinDawn_StartMediaServer) failed.\n");
        }
        else
        {
          nStartMediaStatuc =fStartMediaServerFuc(pickname,dllname,keypath,nMWeight, nMHeight,60,(char*)"867589136",&ao);
           // SSLPRT_MSG("AriplayV.%d.%s-%s-StartMediaOption(%x)ServerResult(%d)WxH(%d,%d)\n",ZYFLAY_VERSION,__DATE__,__TIME__,nCastmode,nStartMediaStatuc,nMWeight,nMHeight);//20
        }
    }
    else
    {
        char *error = dlerror();
        if(error != NULL)
        {
            printf("!!!warning(%s) open fail| error.\n");
        }
        else
        {
            printf("!!!warning open fail.\n");
        }
        return -1;
    }
    return nStartMediaStatuc;
}

// close Airplay
int Ss_mAirplayServiceClose(void)
{
	printf("###Ss_mAirplayServiceClose###\n");
	fpStopMediaServer  fStopMediaServer =  (fpStopMediaServer)dlsym(m_pLibHandle, "XinDawn_StopMediaServer");
	if(fStopMediaServer)
	{
		//Ss_pthread_finish();
		fStopMediaServer();
	}
	nPlayerStatuc = MPLAYER_IDLE;
	return 0;
}

