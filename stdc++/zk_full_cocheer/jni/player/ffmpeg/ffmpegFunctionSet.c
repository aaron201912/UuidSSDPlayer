/*
 * ffmpegFunctionSet.c
 *
 *  Created on: 2020年8月20日
 *      Author: koda.xu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include "ffmpegFunctionSet.h"

int OpenAirplayFfmpegLib(AirplayFfmpegAssembly_t *pstAirplayFfmpegAssembly)
{
	// avutil
	pstAirplayFfmpegAssembly->pAvutilHandle = dlopen("libavutil.so", RTLD_NOW);
	if(NULL == pstAirplayFfmpegAssembly->pAvutilHandle)
	{
		printf(" %s: Can not load libavutil.so!\n", __func__);
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_malloc = (void *(*)(size_t size))dlsym(pstAirplayFfmpegAssembly->pAvutilHandle, "av_malloc");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_malloc)
	{
		printf(" %s: dlsym av_malloc failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_free = (void (*)(void *ptr))dlsym(pstAirplayFfmpegAssembly->pAvutilHandle, "av_free");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_free)
	{
		printf(" %s: dlsym av_free failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_dict_set = (int (*)(AVDictionary **pm, const char *key, const char *value, int flags))dlsym(pstAirplayFfmpegAssembly->pAvutilHandle, "av_dict_set");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_dict_set)
	{
		printf(" %s: dlsym av_dict_set failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_dict_free = (void (*)(AVDictionary **m))dlsym(pstAirplayFfmpegAssembly->pAvutilHandle, "av_dict_free");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_dict_free)
	{
		printf(" %s: dlsym av_dict_free failed, %s\n", __func__, dlerror());
		return -1;
	}

	// avformat
	pstAirplayFfmpegAssembly->pAvformatHandle = dlopen("libavformat.so", RTLD_NOW);
	if(NULL == pstAirplayFfmpegAssembly->pAvformatHandle)
	{
		printf(" %s: Can not load libavformat.so!\n", __func__);
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_avformat_alloc_context = (AVFormatContext *(*)())dlsym(pstAirplayFfmpegAssembly->pAvformatHandle, "avformat_alloc_context");
	if(NULL == pstAirplayFfmpegAssembly->pfn_avformat_alloc_context)
	{
		printf(" %s: dlsym avformat_alloc_context failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_avformat_open_input = (int (*)(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options))dlsym(pstAirplayFfmpegAssembly->pAvformatHandle, "avformat_open_input");
	if(NULL == pstAirplayFfmpegAssembly->pfn_avformat_open_input)
	{
		printf(" %s: dlsym avformat_open_input failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_avformat_find_stream_info = (int (*)(AVFormatContext *ic, AVDictionary **options))dlsym(pstAirplayFfmpegAssembly->pAvformatHandle, "avformat_find_stream_info");
	if(NULL == pstAirplayFfmpegAssembly->pfn_avformat_find_stream_info)
	{
		printf(" %s: dlsym avformat_find_stream_info failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_read_frame = (int (*)(AVFormatContext *s, AVPacket *pkt))dlsym(pstAirplayFfmpegAssembly->pAvformatHandle, "av_read_frame");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_read_frame)
	{
		printf(" %s: dlsym av_read_frame failed, %s\n", __func__, dlerror());
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_avformat_close_input = (void (*)(AVFormatContext **s))dlsym(pstAirplayFfmpegAssembly->pAvformatHandle, "avformat_close_input");
	if(NULL == pstAirplayFfmpegAssembly->pfn_avformat_close_input)
	{
		printf(" %s: dlsym avformat_close_input failed, %s\n", __func__, dlerror());
		return -1;
	}
	// avcodec
	pstAirplayFfmpegAssembly->pAvcodecHandle = dlopen("libavcodec.so", RTLD_NOW);
	if(NULL == pstAirplayFfmpegAssembly->pAvcodecHandle)
	{
		printf(" %s: Can not load libavcodec.so!\n", __func__);
		return -1;
	}

	pstAirplayFfmpegAssembly->pfn_av_packet_unref = (void (*)(AVPacket *pkt))dlsym(pstAirplayFfmpegAssembly->pAvcodecHandle, "av_packet_unref");
	if(NULL == pstAirplayFfmpegAssembly->pfn_av_packet_unref)
	{
		printf(" %s: dlsym av_packet_unref failed, %s\n", __func__, dlerror());
		return -1;
	}

	return 0;
}

void CloseAirplayFfmpegLib(AirplayFfmpegAssembly_t *pstAirplayFfmpegAssembly)
{
	if(pstAirplayFfmpegAssembly->pAvcodecHandle)
	{
		dlclose(pstAirplayFfmpegAssembly->pAvcodecHandle);
		pstAirplayFfmpegAssembly->pAvcodecHandle = NULL;
	}

	if(pstAirplayFfmpegAssembly->pAvformatHandle)
	{
		dlclose(pstAirplayFfmpegAssembly->pAvformatHandle);
		pstAirplayFfmpegAssembly->pAvformatHandle = NULL;
	}

	if(pstAirplayFfmpegAssembly->pAvutilHandle)
	{
		dlclose(pstAirplayFfmpegAssembly->pAvutilHandle);
		pstAirplayFfmpegAssembly->pAvutilHandle = NULL;
	}

	memset(pstAirplayFfmpegAssembly, 0, sizeof(AirplayFfmpegAssembly_t));
}
