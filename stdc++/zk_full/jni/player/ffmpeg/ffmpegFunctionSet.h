/*
 * ffmpegFunctionSet.h
 *
 *  Created on: 2020年8月20日
 *      Author: koda.xu
 */

#ifndef JNI_PLAYER_FFMPEG_FFMPEGFUNCTIONSET_H_
#define JNI_PLAYER_FFMPEG_FFMPEGFUNCTIONSET_H_

#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

typedef struct
{
	void *pAvutilHandle;
	void *pAvformatHandle;
	void *pAvcodecHandle;

	void *(*pfn_av_malloc)(size_t size);
	void (*pfn_av_free)(void *ptr);
	int (*pfn_av_dict_set)(AVDictionary **pm, const char *key, const char *value, int flags);
	void (*pfn_av_dict_free)(AVDictionary **m);

	AVFormatContext *(*pfn_avformat_alloc_context)(void);
	int (*pfn_avformat_open_input)(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
	int (*pfn_avformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
	int (*pfn_av_read_frame)(AVFormatContext *s, AVPacket *pkt);
	void (*pfn_avformat_close_input)(AVFormatContext **s);

	void (*pfn_av_packet_unref)(AVPacket *pkt);
} AirplayFfmpegAssembly_t;


int OpenAirplayFfmpegLib(AirplayFfmpegAssembly_t *pstAirplayFfmpegAssembly);
void CloseAirplayFfmpegLib(AirplayFfmpegAssembly_t *pstAirplayFfmpegAssembly);



#endif /* JNI_PLAYER_FFMPEG_FFMPEGFUNCTIONSET_H_ */
