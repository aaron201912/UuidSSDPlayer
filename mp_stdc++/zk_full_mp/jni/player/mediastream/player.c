﻿/*******************************************************************************
 * player.c
 *
 * history:
 *   2018-11-27 - [lei]     Create file: a simplest ffmpeg player
 *   2018-12-01 - [lei]     Playing audio
 *   2018-12-06 - [lei]     Playing audio&vidio
 *   2019-01-06 - [lei]     Add audio resampling, fix bug of unsupported audio 
 *                          format(such as planar)
 *   2019-01-16 - [lei]     Sync video to audio.
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   ffplay.c in FFmpeg 4.1 project.
 *******************************************************************************/
#ifdef SUPPORT_PLAYER_MODULE
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "player.h"
#include "frame.h"
#include "packet.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"

AVPacket a_flush_pkt, v_flush_pkt;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
//static int av_sync_type = AV_SYNC_VIDEO_MASTER;
static float seek_interval = 10;

player_stat_t *g_myplayer = NULL;

#ifndef SUPPORT_PLAYER_PROCESS
//player_stat_t *player_init(const char *p_input_file);
//int player_deinit(player_stat_t *is);
static int get_master_sync_type(player_stat_t *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->p_video_stream)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->p_audio_stream)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}


/* get the current master clock value */
double get_master_clock(player_stat_t *is)
{
    double val;

    switch (get_master_sync_type(is)) {
        case AV_SYNC_VIDEO_MASTER:
            val = get_clock(&is->video_clk);
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = get_clock(&is->audio_clk);
            break;
        default:
            val = get_clock(&is->extclk);
            break;
    }
    return val;
}


// 返回值：返回上一帧的pts更新值(上一帧pts+流逝的时间)
double get_clock(play_clock_t *c)
{
    if (*c->queue_serial != c->serial)
    {
        return NAN;
    }
    if (c->paused)
    {
        return c->pts;
    }
    else
    {
        double time = av_gettime_relative() / 1000000.0;
        double ret = c->pts_drift + time;   // 展开得： c->pts + (time - c->last_updated)
        return ret;
    }
}

void set_clock_at(play_clock_t *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

void set_clock(play_clock_t *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

static void set_clock_speed(play_clock_t *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

void init_clock(play_clock_t *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

static void sync_play_clock_to_slave(play_clock_t *c, play_clock_t *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

static void audio_decoder_abort(player_stat_t *is)
{
    printf("$0.audio_decoder_abort start!\n");
    packet_queue_abort(&is->audio_pkt_queue);
    frame_queue_signal(&is->audio_frm_queue);
    pthread_cond_signal(&is->audio_cond);

    if (is->audioDecode_tid)
        pthread_join(is->audioDecode_tid, NULL); 
    printf("$1.audioDecode_tid pthread_join exit!\n");

    if (is->audioPlay_tid)
        pthread_join(is->audioPlay_tid, NULL);
    printf("$2.audioPlay_tid pthread_join exit!\n");

    packet_queue_flush(&is->audio_pkt_queue);
    printf("$3.audio packet_queue_flush exit!\n");

    avcodec_free_context(&is->p_acodec_ctx);
    printf("$4.audio avcodec_free_context exit!\n");
}

static void video_decoder_abort(player_stat_t *is)
{
    printf("#0.video_decoder_abort start!\n");
    packet_queue_abort(&is->video_pkt_queue);
    frame_queue_signal(&is->video_frm_queue);
    pthread_cond_signal(&is->video_cond);

    if (is->videoDecode_tid)
        pthread_join(is->videoDecode_tid, NULL);
    printf("#1.videoDecode_tid pthread_join exit!\n");

    if (is->videoPlay_tid)
        pthread_join(is->videoPlay_tid, NULL);
    printf("#2.videoPlay_tid pthread_join exit!\n");

    video_buffer_flush(is);
    printf("#3.video_flush_buffer free!\n");

    packet_queue_flush(&is->video_pkt_queue);
    printf("#4.video packet_queue_flush free!\n");

    if (is->enable_video && is->playerController.fpResetVideoDisplay)
        is->playerController.fpResetVideoDisplay();
    printf("#5.player fpResetVideoDisplay exit!\n");

    avcodec_free_context(&is->p_vcodec_ctx);
    printf("#6.video avcodec_free_context exit!\n");
}

static void stream_component_close(player_stat_t *is, int stream_index)
{
    AVFormatContext *ic = is->p_fmt_ctx;
    AVCodecParameters *codecpar;
    printf("stream_index: %d,nb_streams: %d\n",stream_index,ic->nb_streams);
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        audio_decoder_abort(is);
        if (is->audio_swr_ctx)
            swr_free(&is->audio_swr_ctx);
        printf("$5.audio_swr_ctx free!\n");
        if (is->p_audio_frm)
            av_freep(&is->p_audio_frm);
        printf("$6.p_audio_frm free!\n");
        is->audio_frm_size = 0;
        is->audio_frm_rwr_size = 0;
        is->audio_frm_rwr = NULL;
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_decoder_abort(is);
        if (is->img_convert_ctx)
            sws_freeContext(is->img_convert_ctx);
        printf("#7.img_convert_ctx free!\n");
        if (is->p_frm_yuv)
            av_frame_free(&is->p_frm_yuv);
        printf("#8.av_frame_free p_frm_yuv!\n");
        if (is->vir_addr && is->phy_addr) {
            MI_SYS_FlushInvCache(is->vir_addr, is->buf_size);
            MI_SYS_Munmap(is->vir_addr, is->buf_size);
            MI_SYS_MMA_Free(is->phy_addr);
        }
        printf("#9.MI_SYS_Munmap and MI_SYS_MMA_Free!\n");
        break;

    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->p_audio_stream = NULL;
        is->audio_idx = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->p_video_stream = NULL;
        is->video_idx = -1;
        break;
    default:
        break;
    }
}

static void set_player_defaults(player_stat_t *is)
{
    if (is) {
        memset(is, 0x00, sizeof(player_stat_t));

        is->video_idx  = -1;
        is->audio_idx  = -1;
        is->video_complete = 1;
        is->audio_complete = 1;
        is->display_mode = E_MI_DISP_ROTATE_NONE;
    }
}

static void* idle_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;

    printf("get in idle pthread!\n");

    while(is)
    {
        if(is->abort_request)
        {
            break;
        }

        av_usleep((unsigned)(50 * 1000));   //阻塞50ms

        if (is->play_error < 0)
        {
            if (is->playerController.fpPlayError)
            {
                is->playerController.fpPlayError(is->play_error);
                is->play_error = 0;
                break;
            }
        }
        else
        {
            if ((is->enable_video || is->enable_audio) && is->audio_complete && is->video_complete)
            {
                if (is->playerController.fpPlayComplete)
                {
                    is->playerController.fpPlayComplete();
                    break;
                }
            }
        }
    }
    
    printf("idle pthread exit\n");

    return NULL;
}

player_stat_t *player_init(const char *p_input_file)
{
    player_stat_t *is;

    is = (player_stat_t *)av_mallocz(sizeof(player_stat_t));
    if (!is)
    {
        return NULL;
    }
    set_player_defaults(is);
    g_myplayer = is;

    is->filename = av_strdup(p_input_file);
    if (is->filename == NULL)
    {
        goto fail;
    }

    /* start video display */
    if (frame_queue_init(&is->video_frm_queue, &is->video_pkt_queue, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0 ||
        frame_queue_init(&is->audio_frm_queue, &is->audio_pkt_queue, SAMPLE_QUEUE_SIZE, 1) < 0)
    {
        goto fail;
    }

    if (packet_queue_init(&is->video_pkt_queue) < 0 ||
        packet_queue_init(&is->audio_pkt_queue) < 0)
    {
        goto fail;
    }

    av_init_packet(&a_flush_pkt);
    av_init_packet(&v_flush_pkt);
    a_flush_pkt.data = (uint8_t *)&a_flush_pkt;
    v_flush_pkt.data = (uint8_t *)&v_flush_pkt;
    
    packet_queue_put(&is->video_pkt_queue, &a_flush_pkt);
    packet_queue_put(&is->audio_pkt_queue, &v_flush_pkt);

    if (pthread_cond_init(&is->continue_read_thread,NULL) != SUCCESS)
    {
        printf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);
        goto fail;
    }

    init_clock(&is->video_clk, &is->video_pkt_queue.serial);
    init_clock(&is->audio_clk, &is->audio_pkt_queue.serial);

    pthread_mutex_init(&is->video_mutex, NULL);
    pthread_mutex_init(&is->audio_mutex, NULL);
    pthread_cond_init(&is->audio_cond, NULL);
    pthread_cond_init(&is->video_cond, NULL);

    is->abort_request = 0;
    is->av_sync_type  = av_sync_type;
    is->play_error    = 0;

    pthread_create(&is->idle_tid, NULL, idle_thread, is);

    return is;
fail:
    av_free(is->filename);
    frame_queue_destory(&is->video_frm_queue);
    frame_queue_destory(&is->audio_frm_queue);
    packet_queue_flush(&is->video_pkt_queue);
    packet_queue_flush(&is->audio_pkt_queue);
    pthread_cond_destroy(&is->continue_read_thread);
    av_freep(&is);

    return NULL;
}


int player_deinit(player_stat_t *is)
{
    printf("00.player_deinit start\n");
    
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1;
    is->seek_flags = 0;

    // 如果运行期间没有错误或者解码速度不够,按正常流程退出
    if (true == is->demux_status)
    {
        if(is->read_tid != 0)
            pthread_join(is->read_tid, NULL);
        printf("01.demux read_tid pthread finish\n");
        /* close each stream */
        if (is->audio_idx >= 0)
        {
            stream_component_close(is, is->audio_idx);
            printf("02.audio stream_component_close finish\n");
        }
        if (is->video_idx >= 0)
        {
            stream_component_close(is, is->video_idx);
            printf("03.video stream_component_close finish\n");
        }

        av_dict_free(&is->p_dict);
        printf("04.av_dict_free finish!\n");

        avformat_close_input(&is->p_fmt_ctx);
        printf("05.avformat_close_input finish!\n");
    }
    /* free all pictures */
    packet_queue_destroy(&is->video_pkt_queue);
    printf("06.video packet_queue_destroy!\n");
    
    packet_queue_destroy(&is->audio_pkt_queue);
    printf("07.audio packet_queue_destroy!\n");
    
    frame_queue_destory(&is->video_frm_queue);
    printf("08.video frame_queue_destory!\n");
    
    frame_queue_destory(&is->audio_frm_queue);
    printf("09.audio frame_queue_destory!\n");

    pthread_mutex_destroy(&is->audio_mutex);
    pthread_mutex_destroy(&is->video_mutex);
    pthread_cond_destroy(&is->audio_cond);
    pthread_cond_destroy(&is->video_cond);
    
    pthread_cond_destroy(&is->continue_read_thread);
    printf("10.read pthread_cond_destroy!\n");
    
    av_free(is->filename);  
    printf("11.av_free filename!\n");
    
    av_freep(&is);
    printf("12.av_free is!\n");

    printf("##### player_deinit exit #####\n");

    return 0;
}

/* pause or resume the video */
void stream_toggle_pause(player_stat_t *is)
{
    if (is->paused)
    {
        // 这里表示当前是暂停状态，将切换到继续播放状态。在继续播放之前，先将暂停期间流逝的时间加到frame_timer中
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->video_clk.last_updated;
        if (is->read_pause_return != AVERROR(ENOSYS)) {
            is->video_clk.paused = 0;
        }
        set_clock(&is->video_clk, get_clock(&is->video_clk), is->video_clk.serial);
    }
    is->paused = is->audio_clk.paused = is->video_clk.paused = !is->paused;
}

void toggle_pause(player_stat_t *is)
{
    stream_toggle_pause(is);
    is->step = 0;
}

// 此处输入的pos定义为相对总时长的相对位置
void stream_seek(player_stat_t *is, int64_t pos, int64_t rel, int seek_by_bytes)
{
    if (!is->seek_req) {
        is->seek_pos = pos;
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE;
        if (seek_by_bytes)
            is->seek_flags |= AVSEEK_FLAG_BYTE;
        is->seek_req = 1;
        pthread_cond_signal(&is->continue_read_thread);
    }
}

#endif
#endif

