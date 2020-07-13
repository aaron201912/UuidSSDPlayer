#include "demux.h"
#include "packet.h"
#include "frame.h"
#include "player.h"
#include "interface.h"
#include <sys/time.h>

extern AVPacket a_flush_pkt, v_flush_pkt;

#define PLAYER_STRT_TIMEOUT  10  //单位秒

static int decode_interrupt_cb(void *ctx)
{
    player_stat_t *is = (player_stat_t *)ctx;
    bool flag;

    gettimeofday(&is->now, NULL);
    flag = (is->now.tv_sec - is->start.tv_sec > PLAYER_STRT_TIMEOUT) ? true : false;
    if (flag) {
        is->time_out = true;
        av_log(NULL, AV_LOG_ERROR, "timeout of reading packet from network!\n");
    }
    return flag;
}

/*static int stream_has_enough_packets(AVStream *st, int stream_id, packet_queue_t *queue, player_stat_t *is)
{
    //printf("id: %d,disposition: %d,nb_packets: %d,duration: %d\n",stream_id,st->disposition,queue->nb_packets,queue->duration);
    if (stream_id == is->audio_idx)
    {
        return (stream_id < 0) || queue->abort_request ||
               (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
               (queue->nb_packets > MIN_AUDIO_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
    }
    if (stream_id == is->video_idx)
    {
        return (stream_id < 0) || queue->abort_request ||
               (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
               (queue->nb_packets > MIN_VIDEO_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
    }
    return 1;
}*/

static void step_to_next_frame(player_stat_t *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused) {
        stream_toggle_pause(is);
        av_log(NULL, AV_LOG_WARNING, "seeking after puase!\n");
    }
    is->step = 1;
}

/* this thread gets the stream from the disk or the network */
static void * demux_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    //AVFormatContext *p_fmt_ctx = is->p_fmt_ctx;
    int ret;
    //AVPacket pkt1, *pkt = &pkt1;

    struct timeval now;
    struct timespec outtime;

    pthread_mutex_t wait_mutex;

    //AVPacket *pkt = av_packet_alloc();
    AVPacket packet, *pkt = &packet;
    CheckFuncResult(pthread_mutex_init(&wait_mutex, NULL));
    is->eof = 0;

    // 4. 解复用处理
    while (1)
    {
        if (is->abort_request)
        {
            printf("demux thread exit\n");
            break;
        }
        //printf("loop start paused: %d,last_paused: %d\n",is->paused,is->last_paused);
        if (is->paused != is->last_paused) {
            is->last_paused = is->paused;
            if (is->paused)
            {
                is->read_pause_return = av_read_pause(is->p_fmt_ctx);
            }
            else
            {
                av_read_play(is->p_fmt_ctx);
            }
        }

        if (is->seek_req) {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;

            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            // of the seek_pos/seek_rel variables
            //printf("video stream pos : %lld\n", seek_target);
            is->seek_flags |= AVSEEK_FLAG_BACKWARD;
            //ret = av_seek_frame(is->p_fmt_ctx, is->video_idx, seek_target, is->seek_flags);
            ret = avformat_seek_file(is->p_fmt_ctx, -1, seek_min, seek_target, seek_max, is->seek_flags);

            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "%s: error while seeking\n", is->p_fmt_ctx->url);
            } else {
                if (is->audio_idx >= 0) {
                    pthread_mutex_lock(&is->audio_mutex);
                    is->seek_flags |= (1 << 5);
                    packet_queue_flush(&is->audio_pkt_queue);
                    packet_queue_put(&is->audio_pkt_queue, &a_flush_pkt);
                    pthread_mutex_unlock(&is->audio_mutex);
                }
 
                if (is->video_idx >= 0) {
                    pthread_mutex_lock(&is->video_mutex);
                    is->seek_flags |= (1 << 6);
                    packet_queue_flush(&is->video_pkt_queue);
                    packet_queue_put(&is->video_pkt_queue, &v_flush_pkt);
                    pthread_mutex_unlock(&is->video_mutex);
                    //is->p_vcodec_ctx->flags |= (d1 << 7);
                }
                /*if (is->seek_flags & AVSEEK_FLAG_BYTE) {
                   set_clock(&is->extclk, NAN, 0);
                } else {
                   set_clock(&is->extclk, seek_target / (double)AV_TIME_BASE, 0);
                }*/
            }
            is->seek_req = 0;
            is->eof = 0;
            is->audio_complete = (is->audio_idx >= 0) ? 0 : 1;
            is->video_complete = (is->video_idx >= 0) ? 0 : 1;
            if (is->paused)
                step_to_next_frame(is);
        }

        /* if the queue are full, no need to read more */
        if (is->audio_pkt_queue.size + is->video_pkt_queue.size > MAX_QUEUE_SIZE /*||
            (stream_has_enough_packets(is->p_audio_stream, is->audio_idx, &is->audio_pkt_queue, is) &&
             stream_has_enough_packets(is->p_video_stream, is->video_idx, &is->video_pkt_queue, is))*/)
        {
            /* wait 100 ms */
            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread,&wait_mutex,&outtime);
            pthread_mutex_unlock(&wait_mutex);
            //printf("queue size: %d\n",is->audio_pkt_queue.size + is->video_pkt_queue.size);

            if (is->video_idx >= 0 && is->video_pkt_queue.nb_packets < 10) {
                //printf("wait video queue avalible pktnb: %d\n",is->video_pkt_queue.nb_packets);
            }
            if (is->audio_idx >= 0 && is->audio_pkt_queue.nb_packets == 0) {
                //printf("wait audio queue avalible pktnb: %d\n",is->audio_pkt_queue.nb_packets);
                is->play_status = -3;
            }

            if (is->no_pkt_buf) {
                //av_log(NULL, AV_LOG_INFO, "packets queue is full!\n");
                is->no_pkt_buf = 0;
            }
            //av_log(NULL, AV_LOG_WARNING, "packets queue is full!\n");
            continue;
        }

        // 4.1 从输入文件中读取一个packet
        gettimeofday(&is->start, NULL);
        ret = av_read_frame(is->p_fmt_ctx, pkt);
        if (ret < 0)
        {
            if (((ret == AVERROR_EOF) || avio_feof(is->p_fmt_ctx->pb)) && !is->eof)
            {
                // 输入文件已读完，则往packet队列中发送NULL packet，以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                if (is->video_idx >= 0)
                {
                    packet_queue_put_nullpacket(&is->video_pkt_queue, is->video_idx); 
                }

                if (is->audio_idx >= 0)
                {
                    packet_queue_put_nullpacket(&is->audio_pkt_queue, is->audio_idx);
                }

                is->eof = 1;
                //av_log(is->p_fmt_ctx, AV_LOG_INFO, "read packet over!\n");
                av_log(NULL, AV_LOG_ERROR, "ret : %d, feof : %d\n", ret, avio_feof(is->p_fmt_ctx->pb));
            }

            if (is->time_out) {
                av_log(NULL, AV_LOG_ERROR, "av_read_frame time out!\n");
                is->time_out = false;
                is->play_status = -4;
            }

            pthread_mutex_lock(&wait_mutex);
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread,&wait_mutex,&outtime);
            pthread_mutex_unlock(&wait_mutex);

            //av_log(NULL, AV_LOG_ERROR, "av_read_frame failed! ret : %d\n", ret);
            continue;
        }
        else
        {
            is->eof = 0;
            if (!is->no_pkt_buf && !strncmp(is->p_fmt_ctx->iformat->name, "hls", 3)
                && is->audio_pkt_queue.size + is->video_pkt_queue.size < MIN_QUEUE_SIZE) {
                av_log(NULL, AV_LOG_WARNING, "no packets wait for buffer!\n");
                is->no_pkt_buf = 1;
            }
        }

        // 4.3 根据当前packet类型(音频、视频、字幕)，将其存入对应的packet队列

        if (pkt->stream_index == is->audio_idx)
        {
            packet_queue_put(&is->audio_pkt_queue, pkt);
            //printf("put audio pkt end, size = %d\n", is->audio_pkt_queue.nb_packets);
        }
        else if (pkt->stream_index == is->video_idx)
        {
            packet_queue_put(&is->video_pkt_queue, pkt);
            //printf("put video pkt end, size = %d\n", is->video_pkt_queue.nb_packets);
        }
        else
        {
            av_packet_unref(pkt);
        }
    }
    //av_packet_free(pkt);

    pthread_mutex_destroy(&wait_mutex);
    return NULL;
}

static int demux_init(player_stat_t *is)
{
    AVFormatContext *p_fmt_ctx = NULL;
    int err, i, ret;
    int a_idx;
    int v_idx;

    //avformat_network_init();

    p_fmt_ctx = avformat_alloc_context();
    if (!p_fmt_ctx)
    {
        printf("Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    is->p_fmt_ctx = p_fmt_ctx;

    // 中断回调机制。为底层I/O层提供一个处理接口，比如中止IO操作。
    p_fmt_ctx->interrupt_callback.callback = decode_interrupt_cb;
    p_fmt_ctx->interrupt_callback.opaque = is;
    // 1. 构建AVFormatContext
    // 1.1 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    av_dict_set(&is->p_dict, "max_delay", "10000000", 0);//设置超时10秒
    //av_dict_set(&is->p_dict, "probesize", "102400", 0); //探测长度设置为100K
    gettimeofday(&is->start, NULL);
    //p_fmt_ctx->probesize = 100 * 1024;
    //p_fmt_ctx->format_probesize = 100 * 1024;
    //p_fmt_ctx->max_analyze_duration = 7 * AV_TIME_BASE;
    err = avformat_open_input(&p_fmt_ctx, is->filename, NULL, &is->p_dict);
    if (err < 0)
    {
        if (is->time_out) {
            av_log(NULL, AV_LOG_ERROR, "avformat_open_input time out!\n");
            is->time_out = false;
            is->play_status = -4;
        } else {
            if (err == -101) {
                av_log(NULL, AV_LOG_ERROR, "network can't reachable!\n");
                is->play_status = err;
            } else {
                printf("avformat_open_input(%s) failed %d\n",is->filename,err);
                is->play_status = -1;
            }
        }
        ret = err;
        goto fail;
    }
    // 1.2 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入p_fmt_ctx->streams
    //     ic->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    err = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (err < 0)
    {
        printf("avformat_find_stream_info() failed %d\n", err);
        ret = -1;
        goto fail;
    }
    av_log(NULL, AV_LOG_INFO, "avformat demuxer name : %s\n", p_fmt_ctx->iformat->name);
    is->no_pkt_buf = 0;
    av_log(NULL, AV_LOG_ERROR, "avio buffer size = %d, probesize = %lld\n", p_fmt_ctx->pb->buffer_size, p_fmt_ctx->probesize);

    // 2. 查找第一个音频流/视频流
    a_idx = -1;
    v_idx = -1;
    for (i=0; i<(int)p_fmt_ctx->nb_streams; i++)
    {
        if ((p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) &&
            (a_idx == -1))
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
        }
        if ((p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) &&
            (v_idx == -1))
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
        }
        if (a_idx != -1 && v_idx != -1)
        {
            break;
        }
    }
    if (a_idx == -1 && v_idx == -1)
    {
        printf("Cann't find any audio/video stream\n");
        ret = -1;
        goto fail;
    }
    printf("audio idx: %d,video idx: %d\n",a_idx,v_idx);

    double totle_seconds = p_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q);
    printf("start time : %.3f, total time of input file : %0.3f\n", p_fmt_ctx->start_time * av_q2d(AV_TIME_BASE_Q), totle_seconds);
    av_dump_format(p_fmt_ctx, 0, is->filename, 0);

    if (a_idx >= 0) {
        is->p_audio_stream = p_fmt_ctx->streams[a_idx];
        is->audio_complete = 0;
    }

    if (v_idx >= 0) {
        is->p_video_stream = p_fmt_ctx->streams[v_idx];

        if (is->p_video_stream->codecpar->width <= 0 || is->p_video_stream->codecpar->height <= 0) {
            printf("read video stream info error!\n");
            ret = -1;
            is->play_status = -1;
            goto fail;
        }

        if (is->p_video_stream->codecpar->codec_id != AV_CODEC_ID_H264 && is->p_video_stream->codecpar->codec_id != AV_CODEC_ID_HEVC)
        {
            if (is->p_video_stream->codecpar->width * is->p_video_stream->codecpar->height > 1280 * 720) 
            {
                if(a_idx != -1)
                {
                    printf("soft solution of video cannot over 720P[%d]!\n",is->p_video_stream->codecpar->codec_id);
                    ret = -1;
                    is->play_status = -2;
                    goto fail;
                }
                else
                {
                    printf("soft solution of pic[%dx%d],codec:[%d]!\n",is->p_video_stream->codecpar->width,is->p_video_stream->codecpar->height,is->p_video_stream->codecpar->codec_id);
                    if (is->p_video_stream->codecpar->width * is->p_video_stream->codecpar->height > 16 * 1024 * 1024)
                    {
                        av_log(NULL, AV_LOG_ERROR, "not support, picture too big!\n");
                        ret = -1;
                        is->play_status = -7;
                        goto fail;
                    }
                }
            }
        }
        else 
        {
            if (is->p_video_stream->codecpar->width * is->p_video_stream->codecpar->height > 1920 * 1080) 
            {
                printf("hard solution of video cannot over 1080P\n");
                ret = -1;
                is->play_status = -6;
                goto fail;
            }
        }
        is->video_complete = 0;
    }

    prctl(PR_SET_NAME, "demux_read");
    ret = pthread_create(&is->read_tid, NULL, demux_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "demux_thread create failed!\n");
        ret = -1;
        is->read_tid = 0;
        goto fail;
    }

    is->audio_idx = a_idx;
    is->video_idx = v_idx; 

    return 0;

fail:
    av_dict_free(&is->p_dict);
    if (p_fmt_ctx != NULL)
    {
       avformat_close_input(&p_fmt_ctx);
    }
    return ret;
}


int demux_deinit()
{
    return 0;
}

int open_demux(player_stat_t *is)
{
    if (demux_init(is) != 0)
    {
        printf("demux_init() failed\n");
        is->demux_status = false;
        return -1;
    }
    is->demux_status = true;

    return 0;
}

