/*
 * rtspclient.c
 *
 *  Created on: 2020年8月18日
 *      Author: koda.xu
 */

#include "rtspclient.h"
#include "rtsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef struct
{
	void *pHandle;
	int (*pfnrtsp_init)(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height);
	void (*pfnrtsp_deinit)();
} RTSPClientAssembly_t;

static RTSPClientAssembly_t g_stRTSPClientAssembly;

static int OpenRTSPLibrary()
{
	g_stRTSPClientAssembly.pHandle = dlopen("librtsp.so", RTLD_NOW);
	if(NULL == g_stRTSPClientAssembly.pHandle)
	{
		printf(" %s: Can not load librtsp.so!\n", __func__);
		return -1;
	}

	g_stRTSPClientAssembly.pfnrtsp_init = (int(*)(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height))dlsym(g_stRTSPClientAssembly.pHandle, "rtsp_init");
	if(NULL == g_stRTSPClientAssembly.pfnrtsp_init)
	{
		printf(" %s: dlsym rtsp_init failed, %s\n", __func__, dlerror());
		return -1;
	}

	g_stRTSPClientAssembly.pfnrtsp_deinit = (void(*)())dlsym(g_stRTSPClientAssembly.pHandle, "rtsp_deinit");
	if(NULL == g_stRTSPClientAssembly.pfnrtsp_deinit)
	{
		printf(" %s: dlsym rtsp_deinit failed, %s\n", __func__, dlerror());
		return -1;
	}

	return 0;
}

static void CloseRTSPLibrary()
{
	if(g_stRTSPClientAssembly.pHandle)
	{
		dlclose(g_stRTSPClientAssembly.pHandle);
		g_stRTSPClientAssembly.pHandle = NULL;
	}
	memset(&g_stRTSPClientAssembly, 0, sizeof(g_stRTSPClientAssembly));
}

int SSTAR_RTSPClinet_Init(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height)
{
	if (OpenRTSPLibrary())
		return -1;

	return g_stRTSPClientAssembly.pfnrtsp_init(liv555Url, configIni, width, height);
}

void SSTAR_RTSPClient_Deinit()
{
	if(NULL == g_stRTSPClientAssembly.pfnrtsp_deinit)
	{
		g_stRTSPClientAssembly.pfnrtsp_deinit();
	}

	CloseRTSPLibrary();
}
