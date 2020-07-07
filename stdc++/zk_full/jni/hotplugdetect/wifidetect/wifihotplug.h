/*
 * wifihotplug.h
 *
 *  Created on: 2020年2月14日
 *      Author: koda.xu
 */

#ifndef JNI_HOTPLUGDETECT_WIFIDETECT_WIFIHOTPLUG_H_
#define JNI_HOTPLUGDETECT_WIFIDETECT_WIFIHOTPLUG_H_


#ifdef  __cplusplus
extern "C"
{
#endif

#include "wifiprofile.h"

typedef struct
{
	WLAN_HANDLE hWlan;
	MI_WLAN_ConnectParam_t stConnParam;
}WifiConnInfo_t;

typedef struct
{
	char ssid[MI_WLAN_MAX_SSID_LEN];
	char mac[MI_WLAN_MAX_MAC_LEN];
	bool bEncrypt;
	int signalSTR;
} ScanResult_t;

typedef void (*WifiConnCallback)(char *pSsid, int status);
typedef void (*WifiScanCallback)(ScanResult_t *pstScanResult, int resCnt);

int Wifi_StartCheckHotplug();
void Wifi_StopCheckHotplug();
int Wifi_RegisterConnectCallback(WifiConnCallback pfnCallback);
void Wifi_UnRegisterConnectCallback(WifiConnCallback pfnCallback);
int Wifi_RegisterScanCallback(WifiScanCallback pfnCallback);
void Wifi_UnRegisterScanCallback(WifiScanCallback pfnCallback);

void Wifi_Connect(MI_WLAN_ConnectParam_t *pstConnParam);
void Wifi_Disconnect();
int Wifi_GetEnableStatus();
void Wifi_SetEnableStatus(int enable);
int Wifi_GetSupportStatus();
int Wifi_GetCurConnStatus(MI_WLAN_Status_t *status);

#ifdef  __cplusplus
}
#endif

#endif /* JNI_HOTPLUGDETECT_WIFIDETECT_WIFIHOTPLUG_H_ */
