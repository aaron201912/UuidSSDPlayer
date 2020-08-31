#ifndef __RTSP_H__
#define __RTSP_H__

int rtsp_init(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height);
void rtsp_deinit();


#endif