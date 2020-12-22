/*
 * rtspclient.h
 *
 *  Created on: 2020年8月18日
 *      Author: koda.xu
 */

#ifndef JNI_RTSP_RTSPCLIENT_H_
#define JNI_RTSP_RTSPCLIENT_H_

int SSTAR_RTSPClinet_Init(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height);
void SSTAR_RTSPClient_Deinit();

#endif /* JNI_RTSP_RTSPCLIENT_H_ */
