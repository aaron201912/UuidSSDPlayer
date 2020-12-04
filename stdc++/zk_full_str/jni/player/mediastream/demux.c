#ifdef SUPPORT_PLAYER_MODULE
#include "demux.h"
#include "packet.h"
#include "frame.h"
#include <sys/time.h>

extern AVPacket a_flush_pkt, v_flush_pkt;

#ifndef SUPPORT_PLAYER_PROCESS
static int decode_interrupt_cb(void *ctx)
{
    player_stat_t *is = (player_stat_t *)ctx;
    return is->abort_request;
}

static int stream_has_enough_packets(AVStream *st, int stream_id, packet_queue_t *queue)
{
    //printf("id: %d,disposition: %d,nb_packets: %d,duration: %d\n",stream_id,st->disposition,queue->nb_packets,queue->duration);
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           (queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
}

static void step_to_next_frame(player_stat_t *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused)
        stream_toggle_pause(is);
    is->step = 1;
}

/* this thread gets the stream from the disk or the network */
static void* demux_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVFormatContext *p_fmt_ctx = is->p_fmt_ctx;
    int ret;
    AVPacket pkt1, *pkt = &pkt1;

    struct timeval now;
    struct timespec outtime;

    pthread_mutex_t wait_mutex;

    if (pthread_mutex_init(&wait_mutex, NULL) != SUCCESS)
    {
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }

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

        if(is->paused)
        {
            /* wait 10 ms */
            pthread_mutex_lock(&wait_mutex);
            
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms

            pthread_cond_timedwait(&is->continue_read_thread,&wait_mutex,&outtime);

            //SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            pthread_mutex_unlock(&wait_mutex);

            continue;
        }

        if (is->seek_req) {
            // 播放器根据进度条拖动位置进行跳转，此pos是相对的需换算为据对pos
            int64_t seek_target = is->seek_pos + is->p_fmt_ctx->start_time;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;

            // FIXME the +-2 is due to rounding being not done in the correct direction in generation
            // of the seek_pos/seek_rel variables
            is->seek_flags |= AVSEEK_FLAG_BACKWARD;
            ret = avformat_seek_file(is->p_fmt_ctx, -1, seek_min, seek_target, seek_max, is->seek_flags);
            //ret = av_seek_frame(is->p_fmt_ctx, is->video_idx, seek_target, AVSEEK_FLAG_ANY);

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
                    //is->p_vcodec_ctx->flags |= (1 << 7);
                }
                /*
                if (is->seek_flags & AVSEEK_FLAG_BYTE) {
                   set_clock(&is->extclk, NAN, 0);
                } else {
                   set_clock(&is->extclk, seek_target / (double)AV_TIME_BASE, 0);
                }
                */
            }
            is->seek_req = 0;
            //is->queue_attachments_req = 1;
            is->eof = 0;
            is->audio_complete = (is->audio_idx >= 0) ? 0 : 1;
            is->video_complete = (is->video_idx >= 0) ? 0 : 1;
            if (is->paused)
                step_to_next_frame(is);
        }
        
        
        /* if the queue are full, no need to read more */
        if (is->audio_pkt_queue.size + is->video_pkt_queue.size > MAX_QUEUE_SIZE /*||
            (stream_has_enough_packets(is->p_audio_stream, is->audio_idx, &is->audio_pkt_queue) &&
             stream_has_enough_packets(is->p_video_stream, is->video_idx, &is->video_pkt_queue))*/)
        {
            /* wait 10 ms */
            pthread_mutex_lock(&wait_mutex);

            gettimeofday(&now, NULL);

            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms

            pthread_cond_timedwait(&is->continue_read_thread,&wait_mutex,&outtime);

            //SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            pthread_mutex_unlock(&wait_mutex);
            //printf("queue size: %d\n",is->audio_pkt_queue.size + is->video_pkt_queue.size);
            //printf("wait video queue avalible pktnb: %d\n",is->video_pkt_queue.nb_packets);
            //printf("wait audio queue avalible pktnb: %d\n",is->audio_pkt_queue.nb_packets);
            if (is->audio_idx >= 0 && is->video_idx >= 0 && is->audio_pkt_queue.nb_packets <= 0 && !is->play_error)
            {
                double diff = get_clock(&is->audio_clk) - get_clock(&is->video_clk);
                if (!isnan(diff) && diff > AV_NOSYNC_THRESHOLD) {
                    av_log(NULL, AV_LOG_WARNING, "WARNING: Please Reduce The Resolution Of Video!!!\n");
                    is->play_error = -3;
                    printf("queue size: %d, video pkt size: %d, audio pkt size: %d\n",
                           is->audio_pkt_queue.size + is->video_pkt_queue.size,
                           is->video_pkt_queue.nb_packets,
                           is->audio_pkt_queue.nb_packets);
                }
            }

            if (is->no_pkt_buf) {
                //av_log(NULL, AV_LOG_INFO, "packets queue is full!\n");
                is->no_pkt_buf = 0;
            }

            continue;
        }

        // 4.1 从输入文件中读取一个packet
        ret = av_read_frame(is->p_fmt_ctx, pkt);
        if (ret < 0)
        {
            if (((ret == AVERROR_EOF) || avio_feof(is->p_fmt_ctx->pb)) && !is->eof)
            {
                // 输入文件已读完，则往packet队列中发送NULL packet，以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                if (is->video_idx >= 0)
                {
                    if (is->video_pkt_queue.nb_packets > 0)
                        packet_queue_put_nullpacket(&is->video_pkt_queue, is->video_idx);
                    else
                        is->video_complete = 1;
                }
                if (is->audio_idx >= 0)
                {
                    if (is->audio_pkt_queue.nb_packets > 0)
                        packet_queue_put_nullpacket(&is->audio_pkt_queue, is->audio_idx);
                    else
                        is->audio_complete = 1;
                }
                is->eof = 1;
            }

            pthread_mutex_lock(&wait_mutex);

            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec;
            outtime.tv_nsec = now.tv_usec * 1000 + 10 * 1000 * 1000;//timeout 10ms
            pthread_cond_timedwait(&is->continue_read_thread,&wait_mutex,&outtime);

            pthread_mutex_unlock(&wait_mutex);

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
            //printf("put audio pkt end\n");
        }
        else if (pkt->stream_index == is->video_idx)
        {
            packet_queue_put(&is->video_pkt_queue, pkt);

            //printf("put video pkt end\n");
        }
        else
        {
            av_packet_unref(pkt);
        }
    }

    pthread_mutex_destroy(&wait_mutex);
    return NULL;
}

static int demux_init(player_stat_t *is)
{
    AVFormatContext *p_fmt_ctx = NULL;
    AVCodecParameters* p_codec_par;
    double totle_seconds;
    int err, i, ret;
    int a_idx = -1, v_idx = -1;

    p_fmt_ctx = avformat_alloc_context();
    if (!p_fmt_ctx)
    {
        printf("Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    // 中断回调机制。为底层I/O层提供一个处理接口，比如中止IO操作。
    p_fmt_ctx->interrupt_callback.callback = decode_interrupt_cb;
    p_fmt_ctx->interrupt_callback.opaque = is;

    // 1. 构建AVFormatContext
    // 1.1 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    //av_dict_set(&is->p_dict, "buffer_size", "1024000", 0);  //设置udp的接收缓冲
    err = avformat_open_input(&p_fmt_ctx, is->filename, NULL, &is->p_dict);
    if (err < 0)
    {
        if (err == -101)
            av_log(NULL, AV_LOG_WARNING, "WARNING: Network Is Not Reachable!!!\n");
        else
            printf("avformat_open_input() failed %d\n", err);
        ret = err;
        goto fail;
    }
    is->p_fmt_ctx = p_fmt_ctx;

    // 构建私人结构体保存视频信息
    if (!p_fmt_ctx->opaque) {
        p_fmt_ctx->opaque = (AVH2645HeadInfo *)av_malloc(sizeof(AVH2645HeadInfo));
        if (!p_fmt_ctx->opaque) {
            printf("Could not allocate AVH2645HeadInfo.\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        memset(p_fmt_ctx->opaque, 0x0, sizeof(AVH2645HeadInfo));
    }

    // 1.2 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入p_fmt_ctx->streams
    //     ic->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    err = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (err < 0)
    {
        printf("avformat_find_stream_info() failed %d\n", err);
        if (p_fmt_ctx->opaque) {
            av_freep(&p_fmt_ctx->opaque);
        }
        ret = -1;
        goto fail;
    }

    if (p_fmt_ctx->opaque) {
        AVH2645HeadInfo *head_info = (AVH2645HeadInfo *)p_fmt_ctx->opaque;
        printf("frame_mbs_only_flag = %d\n", head_info->frame_mbs_only_flag);
        printf("max_bytes_per_pic_denom = %d\n", head_info->max_bytes_per_pic_denom);
        printf("frame_cropping_flag = %d\n", head_info->frame_cropping_flag);
        printf("conformance_window_flag = %d\n", head_info->conformance_window_flag);
    }

    is->seek_by_bytes = !!(p_fmt_ctx->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", p_fmt_ctx->iformat->name);
    //av_log(NULL, AV_LOG_WARNING, "seek_by_bytes value = %d\n", is->seek_by_bytes);

    // get media duration
    if (is->p_fmt_ctx->start_time == AV_NOPTS_VALUE)
        is->p_fmt_ctx->start_time = 0;
    printf("probesize is %lld, duration is %lld ms, start_time is %lld\n", is->p_fmt_ctx->probesize, is->p_fmt_ctx->duration, is->p_fmt_ctx->start_time);
    if (is->playerController.fpGetDuration)
        is->playerController.fpGetDuration(is->p_fmt_ctx->duration);

    // 2. 查找所有音频流/视频流
    for (i = 0; i < (int)p_fmt_ctx->nb_streams; i ++)
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

    printf("main audio idx: %d, main video idx: %d\n", a_idx, v_idx);

    totle_seconds = p_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q);
    printf("total time of file : %f\n", totle_seconds);
    av_dump_format(p_fmt_ctx, 0, is->filename, 0);

    // set GetCurPlayPos callback
    if (v_idx >= 0 && p_fmt_ctx->streams[v_idx]->codec_info_nb_frames >= 1)
    {
        is->playerController.fpGetCurrentPlayPosFromVideo = is->playerController.fpGetCurrentPlayPos;
        printf("get play pos from video stream\n");

        p_codec_par = p_fmt_ctx->streams[v_idx]->codecpar;
        if (p_codec_par->width <= 0 || p_codec_par->height <= 0) {
            printf("read video stream info error!\n");
            ret = -1;
            goto fail;
        }
        // 提示软解视频质量不超过720P
        if (p_codec_par->codec_id != AV_CODEC_ID_H264 && p_codec_par->codec_id != AV_CODEC_ID_HEVC)
        {
            if (p_codec_par->width * p_codec_par->height > 1280 * 720)
            {
                av_log(NULL, AV_LOG_WARNING, "WARNNING: The resolution of video is over 720P!!!\n");
                ret = -2;
                goto fail;
            }
        }
        else
        {
            if (p_codec_par->width * p_codec_par->height > 1920 * 1080)
            {
                av_log(NULL, AV_LOG_WARNING, "WARNNING: The resolution of video is over 1080P!!!\n");
                ret = -2;
                goto fail;
            }
        }
    }
    else
    {
        is->playerController.fpGetCurrentPlayPosFromAudio = is->playerController.fpGetCurrentPlayPos;
        printf("get play pos from audio stream\n");
    }

    if (a_idx >= 0)
    {
        is->p_audio_stream = p_fmt_ctx->streams[a_idx];
        is->audio_complete = 0;
        is->av_sync_type = AV_SYNC_AUDIO_MASTER;
        printf("audio codec_info_nbframes:%d, nb_frames:%lld, probe_packet:%d\n", is->p_audio_stream->codec_info_nb_frames, is->p_audio_stream->nb_frames, is->p_audio_stream->probe_packets);
        //printf("audio duration:%lld, nb_frames:%lld\n", is->p_audio_stream->duration, is->p_audio_stream->nb_frames);
    }
    if (v_idx >= 0)
    {
        is->p_video_stream = p_fmt_ctx->streams[v_idx];
        is->video_complete = 0;
        if (a_idx < 0) {
            is->av_sync_type = AV_SYNC_VIDEO_MASTER;
        }
        printf("video codec_info_nbframes:%d, nb_frames:%lld, probe_packet:%d\n", is->p_video_stream->codec_info_nb_frames, is->p_video_stream->nb_frames, is->p_video_stream->probe_packets);
        //printf("video duration:%lld, nb_frames:%lld\n", is->p_video_stream->duration, is->p_video_stream->nb_frames);
    }

    prctl(PR_SET_NAME, "demux_read");
    ret = pthread_create(&is->read_tid, NULL, demux_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "demux_thread create failed!\n");
        is->read_tid = 0;
        goto fail;
    }

    // must be assigned at the end
    is->video_idx = v_idx;
    is->audio_idx = a_idx;

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
    int ret;
    if (0 != (ret = demux_init(is)))
    {
        printf("demux_init() failed\n");
        is->play_error = ret;    // 更新错误代号
        is->demux_status = false;
        return -1;
    }

    is->demux_status = true;
    av_log(NULL, AV_LOG_INFO, "open_demux success!\n");

    return 0;
}
#endif
#endif
