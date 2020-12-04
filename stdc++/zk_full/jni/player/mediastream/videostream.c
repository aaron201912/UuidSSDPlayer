#ifdef SUPPORT_PLAYER_MODULE
#include "videostream.h"
#include "packet.h"
#include "frame.h"
#include "player.h"


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

extern AVPacket v_flush_pkt;

struct timeval time_start, time_end;
int64_t time0, time1;

#ifndef SUPPORT_PLAYER_PROCESS
static int alloc_for_frame(frame_t *vp, AVFrame *frame)
{
    int ret;

    vp->buf_size = av_image_get_buffer_size((enum AVPixelFormat)frame->format, frame->width, frame->height, 1);
    if (vp->buf_size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_image_get_buffer_size failed!\n");
        return -1;
    }
    //av_log(NULL, AV_LOG_WARNING, "malloc for frame = %d\n", vp->buf_size);

    //ret = MI_SYS_MMA_Alloc((MI_U8 *)"MMU_MMA", vp->buf_size, &vp->phy_addr);
    ret = MI_SYS_MMA_Alloc((MI_U8 *)"#frame", vp->buf_size, &vp->phy_addr);
    if (ret != MI_SUCCESS) {
        av_log(NULL, AV_LOG_ERROR, "MI_SYS_MMA_Alloc Falied!\n");
        return -1;
    }

    ret = MI_SYS_Mmap(vp->phy_addr, vp->buf_size, (void **)&vp->vir_addr, TRUE);
    if (ret != MI_SUCCESS) {
        av_log(NULL, AV_LOG_ERROR, "MI_SYS_Mmap Falied!\n");
        return -1;
    }

    ret = av_image_fill_arrays(vp->frame->data,     // dst data[]
                               vp->frame->linesize, // dst linesize[]
                               vp->vir_addr,        // src buffer
                               (enum AVPixelFormat)frame->format,       // pixel format
                               frame->width,
                               frame->height,
                               1                    // align
                               );
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_image_fill_arrays failed!\n");
        return -1;;
    }

    vp->frame->format = frame->format;
    vp->frame->width  = frame->width;
    vp->frame->height = frame->height;
    ret = av_frame_copy(vp->frame, frame);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_frame_copy failed!\n");
        return -1;
    }

    return 0;
}

static int queue_picture(player_stat_t *is, AVFrame *src_frame, double pts, double duration, int64_t pos)
{
    frame_t *vp;
    frame_queue_t *f = &is->video_frm_queue;

    //if (!(vp = frame_queue_peek_writable(&is->video_frm_queue)))
    //    return -1;
    vp = &f->queue[f->windex];

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    //vp->serial = serial;

    //set_default_window_size(vp->width, vp->height, vp->sar);

    // 将AVFrame拷入队列相应位置
    if (is->decode_type == SOFT_DECODING)
    {
        //av_frame_move_ref(vp->frame, src_frame);
        int ret = alloc_for_frame(vp, src_frame);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "alloc_for_frame failed!\n");
            return 0;
        }
        //printf("queue frame fomat: %d\n",vp->frame->format);
        // 更新队列计数及写索引
        //printf("before queue ridx: %d,widx: %d,size: %d,maxsize: %d\n ",is->video_frm_queue.rindex,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
        frame_queue_push(&is->video_frm_queue);
        //printf("after queue ridx: %d,widx: %d,size: %d,maxsize: %d\n ",is->video_frm_queue.rindex,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    }
    else
    {
        if (src_frame->opaque) {
            memset(vp->frame, 0, sizeof(*vp->frame));
            vp->frame->opaque = src_frame->opaque;
            vp->frame->width  = src_frame->width;
            vp->frame->height = src_frame->height;
            vp->frame->pts    = src_frame->pts;
            vp->frame->format = src_frame->format;
            frame_queue_push(&is->video_frm_queue);
        }
    }

    return 0;
}

// 从packet_queue中取一个packet，解码生成frame
static int video_decode_frame(AVCodecContext *p_codec_ctx, packet_queue_t *p_pkt_queue, AVFrame *frame)
{
    int ret;

    while (1)
    {
        AVPacket pkt;

        while (1)
        {
            //if(p_pkt_queue->abort_request) {
            //    return -1;
            //}
            pthread_mutex_lock(&g_myplayer->video_mutex);
            if (g_myplayer->seek_flags & (1 << 6)) {
                pthread_mutex_unlock(&g_myplayer->video_mutex);
                break;
            }
            pthread_mutex_unlock(&g_myplayer->video_mutex);
            // 3. 从解码器接收frame
            // 3.1 一个视频packet含一个视频frame
            //     解码器缓存一定数量的packet后，才有解码后的frame输出
            //     frame输出顺序是按pts的顺序，如IBBPBBP
            //     frame->pkt_pos变量是此frame对应的packet在视频文件中的偏移地址，值同pkt.pos
            ret = avcodec_receive_frame(p_codec_ctx, frame);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF)
                {
                    av_log(NULL, AV_LOG_INFO, "video avcodec_receive_frame(): the decoder has been fully flushed\n");
                    avcodec_flush_buffers(p_codec_ctx);
                    return 0;
                }
                else if (ret == AVERROR(EAGAIN))
                {
                    //av_log(NULL, AV_LOG_ERROR, "cann't fetch a frame, try again!\n");
                    break;
                }
                else
                {
                    av_log(NULL, AV_LOG_ERROR, "video avcodec_receive_frame(): other errors\n");
                    g_myplayer->play_error = -1;
                    av_usleep(10 * 1000);
                    continue;
                }
            }
            else
            {
                frame->pts = frame->best_effort_timestamp;
                //printf("best_effort_timestamp : %lld.\n", frame->pts);
                return 1;   // 成功解码得到一个视频帧或一个音频帧，则返回
            }
        }

        // 1. 取出一个packet。使用pkt对应的serial赋值给d->pkt_serial
        if (packet_queue_get(p_pkt_queue, &pkt, true) < 0)
        {
            printf("packet_queue_get fail\n");
            return -1;
        }

        if (pkt.data == v_flush_pkt.data)
        {
            pthread_mutex_lock(&g_myplayer->video_mutex);
            if ((g_myplayer->seek_flags & (1 << 6)) && p_codec_ctx->frame_number > 1) {
                g_myplayer->seek_flags &= ~(1 << 6);
                //frame_queue_flush(&g_myplayer->video_frm_queue);
            }
            pthread_mutex_unlock(&g_myplayer->video_mutex);

            // 复位解码器内部状态/刷新内部缓冲区。
            //p_codec_ctx->flags |= (1 << 7);
            avcodec_flush_buffers(p_codec_ctx);
            //p_codec_ctx->flags &= ~(1 << 7);

            printf("avcodec_flush_buffers for video!\n");
        }
        else
        {
            // 如果是最后一个空的packet,只取frame不再送packet
            if (pkt.data == NULL && pkt.size == 0)
            {
                p_codec_ctx->flags |= (1 << 5);
                printf("send a null paket to decoder\n");
            }
            else
            {
                p_codec_ctx->flags &= ~(1 << 5);
            }

            // 2. 将packet发送给解码器
            //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
            //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
            //printf("send packet to decoder!\n");
            ret = avcodec_send_packet(p_codec_ctx, &pkt);
            if (ret == AVERROR(EAGAIN))
            {
                av_log(NULL, AV_LOG_ERROR, "receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            }
            else if (ret == MI_ERR_VDEC_FAILED)
            {
                av_log(NULL, AV_LOG_ERROR, "vdec occur fatal erro in decoding!\n");
                g_myplayer->play_error = -1;
            }
        }
        av_packet_unref(&pkt);
    }
}

// 将视频包解码得到视频帧，然后写入picture队列
static void* video_decode_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVFrame *p_frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    int got_picture;
    AVRational tb = is->p_video_stream->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
    
    if (p_frame == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "av_frame_alloc() for p_frame failed\n");
        printf("%s[%d] %s: no memory\n", __FILE__, __LINE__, __FUNCTION__);
        return NULL;
    }

    printf("video time base : %.2fms.\n", 1000 * av_q2d(tb));
    printf("frame rate num : %d. frame rate den : %d.\n", frame_rate.num, frame_rate.den);

    printf("get in video decode thread!\n");

    while (1)
    {
        if(is->abort_request)
        {
            printf("video decode thread exit\n");
            break;
        }
        got_picture = video_decode_frame(is->p_vcodec_ctx, &is->video_pkt_queue, p_frame);
        if (got_picture < 0)
        {
            printf("got pic fail\n");
            goto exit;
        }
        else if (got_picture > 0)
        {
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);   // 当前帧播放时长
            pts = (p_frame->pts == AV_NOPTS_VALUE) ? NAN : p_frame->pts * av_q2d(tb);   // 当前帧显示时间戳

            //printf("frame duration : %f. video frame clock : %f.\n", duration, pts);

            ret = queue_picture(is, p_frame, pts, duration, p_frame->pkt_pos);   // 将当前帧压入frame_queue
            av_frame_unref(p_frame);

            if (ret < 0) {
                goto exit;
            }

            /*if (NULL == frame_queue_peek_writable(&is->video_frm_queue)) {
                continue;
            }*/
            frame_queue_t *f = &is->video_frm_queue;
            pthread_mutex_lock(&f->mutex);
            while (f->size >= f->max_size && !f->pktq->abort_request) {
                pthread_cond_wait(&f->cond, &f->mutex);
                if (is->seek_flags & (1 << 6) || is->abort_request) {
                    break;
                }
            }
            pthread_mutex_unlock(&f->mutex);

            if (f->pktq->abort_request) {
                continue;
            }
        }
    }

exit:
    av_frame_free(&p_frame);

    return NULL;
}

// 根据视频时钟与同步时钟(如音频时钟)的差值，校正delay值，使视频时钟追赶或等待同步时钟
// 输入参数delay是上一帧播放时长，即上一帧播放后应延时多长时间后再播放当前帧，通过调节此值来调节当前帧播放快慢
// 返回值delay是将输入参数delay经校正后得到的值
static double compute_target_delay(double delay, player_stat_t *is)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (is->av_sync_type == AV_SYNC_AUDIO_MASTER && is->audio_idx >= 0) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = get_clock(&is->video_clk) - get_master_clock(is);

        if (!isnan(diff) && fabs(diff) > 2 * AV_SYNC_THRESHOLD_MAX) {
            /* skip or repeat frame. We take into account the
               delay to compute the threshold. I still don't know
               if it is the best guess */
            sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
            if (!isnan(diff)) {
                /*if (diff <= -sync_threshold) {
                    delay = FFMAX(0, delay + diff);
                }
                else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
                    delay = delay + diff;
                }
                else if (diff >= sync_threshold) {
                    delay = 2 * delay;
                }*/

                if (diff < -AV_SYNC_THRESHOLD_MAX) {
                    delay = FFMAX(0, delay + diff);
                }
                else if (diff > AV_SYNC_THRESHOLD_MAX) {
                    delay = 2 * delay;
                }
                else {
                    delay = delay;
                }
            }
            av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%.3f\n", delay, -diff);
        }
    }
    //printf("video: delay=%0.3f A-V=%f\n", delay, -diff);

    return delay;
}

static double vp_duration(player_stat_t *is, frame_t *vp, frame_t *nextvp) {
    if (vp->serial == nextvp->serial)
    {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0)
            return vp->duration;
        else
            return duration;
    } else {
        return 0.0;
    }
}

static void update_video_pts(player_stat_t *is, double pts, int64_t pos, int serial) {
    /* update current video pts */
    set_clock(&is->video_clk, pts, serial);            // 更新vidclock
    //-sync_clock_to_slave(&is->extclk, &is->vidclk);  // 将extclock同步到vidclock
}

//struct timeval trans_start, trans_tim;
//int64_t time0;
static void video_display(player_stat_t *is)
{
    frame_t *vp;

    vp = frame_queue_peek_last(&is->video_frm_queue);
    if (!vp->frame->width || !vp->frame->height)
    {
        printf("invalid video frame width and height!\n");
        return;
    }
    //printf("get vp disp ridx: %d,format: %d\n",is->video_frm_queue.rindex,vp->frame->format);

    // 图像转换：p_frm_raw->data ==> p_frm_yuv->data
    // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
    // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
    // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
    // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
    // AVFrame.*data[]: 每个数组元素指向对应plane
    // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
    if (is->decode_type == SOFT_DECODING)
    {
        sws_scale(is->img_convert_ctx,                    // sws context
                  (const uint8_t *const *)vp->frame->data,// src slice
                  vp->frame->linesize,                    // src stride
                  0,                                      // src slice y
                  is->p_vcodec_ctx->height,               // src slice height
                  is->p_frm_yuv->data,                    // dst planes
                  is->p_frm_yuv->linesize                 // dst strides
                  );
        //is->p_frm_yuv->width  = vp->frame->width;
        //is->p_frm_yuv->height = vp->frame->height;
        if (is->playerController.fpDisplayVideo) {
            is->playerController.fpDisplayVideo((void *)is->p_frm_yuv, false);
        }
    }
    else if (is->decode_type == HARD_DECODING)
    {
        if (vp->frame->opaque) {
            if (is->playerController.fpDisplayVideo) {
                is->playerController.fpDisplayVideo((void *)vp->frame, true);
            }
            av_freep(&vp->frame->opaque);
        }
    }

    //time_start.tv_sec  = time_end.tv_sec;
    //time_start.tv_usec = time_end.tv_usec;
    //gettimeofday(&time_end, NULL);
    //time0 = ((int64_t)time_end.tv_sec * 1000000 + time_end.tv_usec) - ((int64_t)time_start.tv_sec * 1000000 + time_start.tv_usec);
    //printf("time gap of sending data to divp: %lldus\n", time0);
}

/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time, double duration)
{
    player_stat_t *is = (player_stat_t *)opaque;
    double time, delay;
    frame_t *vp;

retry:
    // 暂停处理：不停播放上一帧图像
    if (is->paused)
        return;
recheck:
    while (is->seek_flags & (1 << 6)) {
        //is->start_play = false;
        pthread_cond_signal(&is->video_frm_queue.cond);
        av_usleep(10 * 1000);
    }

    while(is->no_pkt_buf && !is->abort_request) {
        av_usleep(10 * 1000);
        goto recheck;
    }

    if (frame_queue_nb_remaining(&is->video_frm_queue) <= 0)  // 所有帧已显示
    {    
        // if file is eof and there is no paket in queue, then do complete
        if (!is->video_complete && is->eof && is->video_pkt_queue.nb_packets == 0 && is->start_play)
        {
            is->video_complete = 1;
            printf("\033[32;2mvideo play complete!\033[0m\n");
        }
        return;
    }
    /* dequeue the picture */
    vp = frame_queue_peek(&is->video_frm_queue);              // 当前帧：当前待显示的帧
    //printf("refresh ridx: %d,rs:%d,widx: %d,size: %d,maxsize: %d\n",is->video_frm_queue.rindex,is->video_frm_queue.rindex_shown,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    // lastvp和vp不是同一播放序列(一个seek会开始一个新播放序列)，将frame_timer更新为当前时间
    /* compute nominal last_duration */
    delay = compute_target_delay(duration, is);    // 根据视频时钟和同步时钟的差值，计算delay值
    time= av_gettime_relative()/1000000.0;
    // 当前帧播放时刻(is->frame_timer+delay)大于当前时刻(time)，表示播放时刻未到
    if (time < is->frame_timer + delay && is->start_play) {
        // 播放时刻未到，则更新刷新时间remaining_time为当前时刻到下一播放时刻的时间差
        *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
        // 播放时刻未到，则不播放，直接返回
        //printf("not ready play\n");
        return;
    }

    // 更新frame_timer值
    is->frame_timer += delay;
    // 校正frame_timer值：若frame_timer落后于当前系统时间太久(超过最大同步域值)，则更新为当前系统时间
    if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
    {
        is->frame_timer = time;
    }

    pthread_mutex_lock(&is->video_frm_queue.mutex);
    if (!isnan(vp->pts))
    {
        update_video_pts(is, vp->pts, vp->pos, vp->serial); // 更新视频时钟：时间戳、时钟时间
    }
    pthread_mutex_unlock(&is->video_frm_queue.mutex);

    //printf("frame pts : %.2f. clock pts : %.2f\n", vp->pts, is->video_clk.pts);

    // update ui pos
    if (is->playerController.fpGetCurrentPlayPosFromVideo)
    {
        long long videoPts = (long long)(is->video_clk.pts * 1000000LL) - is->p_fmt_ctx->start_time;
        AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
        long long frame_duration = 1000000 / frame_rate.num * frame_rate.den;
        //printf("video pts: %f, drift_pts: %f, duration: %lld, frame_duration:%lld\n", is->video_clk.pts, is->video_clk.pts_drift,
        //  is->p_fmt_ctx->duration, frame_duration);
        is->playerController.fpGetCurrentPlayPosFromVideo(videoPts, frame_duration);
    }

    // 是否要丢弃未能及时播放的视频帧
    if (frame_queue_nb_remaining(&is->video_frm_queue) > 1)  // 队列中未显示帧数>1(只有一帧则不考虑丢帧)
    {
        // 当前帧vp未能及时播放，即下一帧播放时刻(is->frame_timer+duration)小于当前系统时刻(time)
        if (time > is->frame_timer + duration && is->start_play)
        {
            /*if (is->decode_type == HARD_DECODING) {
                av_assert0(vp->frame->opaque);
                if (is->playerController.fpVideoPutBufBack)
                    is->playerController.fpVideoPutBufBack(vp->frame);
                av_freep(&vp->frame->opaque);
            }*/
            frame_queue_next(&is->video_frm_queue);   // 删除上一帧已显示帧，即删除lastvp，读指针加1(从lastvp更新到vp)
            //printf("discarded current frame!\n");
            goto retry;
        }
    }
    // 删除当前读指针元素，读指针+1。若未丢帧，读指针从lastvp更新到vp；若有丢帧，读指针从vp更新到nextvp
    frame_queue_next(&is->video_frm_queue);

    if (is->step && !is->paused)
    {
        stream_toggle_pause(is);
    }

    video_display(is);                      // 取出当前帧vp(若有丢帧是nextvp)进行播放
    
    if (!is->start_play) {                  // 播放第一张图片后认为开始播放
        is->start_play = true;
    }
}

#if 0
static int video_refresh_async(void *opaque, double *remaining_time, double duration)
{
    player_stat_t *is = (player_stat_t *)opaque;
    AVRational tb = is->p_video_stream->time_base;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    double delay, time, pts;
    uint8_t *frame_ydata, *frame_uvdata;
    int ret;
    static int counter;

    // 暂停处理
    if (is->paused)
        return 0;

    if (is->video_pkt_queue.nb_packets == 0 && is->eof)
    {
        if (!is->video_complete)
        {
            is->video_complete = 1;
            printf("\033[32;2mvideo play complete!\033[0m\n");
        }
        return 0;
    }

    // 音画同步处理
    delay = compute_target_delay(duration, is);
    time = av_gettime_relative() / 1000000.0 + duration;
    //printf("real time : %0.3lf, frame_timer : %.3lf, video with audio delay time : %.3lf\n", time, is->frame_timer, delay);
    if (!isnan(delay))
    {
        if (time < is->frame_timer + delay)
        {
            *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
            return 0;
        }
        else
        {
            *remaining_time = 0;
        }
        //printf("real delay time : %0.3lf\n", *remaining_time);
        // 更新frame_timer值
        is->frame_timer += delay;
        // 校正frame_timer值：若frame_timer落后于当前系统时间太久(超过最大同步域值)，则更新为当前系统时间
        if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
        {
            is->frame_timer = time;
        }
    }

    // 显示图像
    if (packet_queue_get(&is->video_pkt_queue, packet, true) < 0)
    {
        printf("packet_queue_get video fail\n");
        return -1;
    }
    
    if (packet->data == v_flush_pkt.data)
    {
        // 复位解码器内部状态/刷新内部缓冲区。
        avcodec_flush_buffers(is->p_vcodec_ctx);
        if (is->decode_type == HARD_DECODING)
        {
            if (is->p_vcodec_ctx->codec->flush)
                is->p_vcodec_ctx->codec->flush(is->p_vcodec_ctx);
        }
        printf("avcodec_flush_buffers for video!\n");
    }
    else
    {
        // 如果是最后一个空的packet,只取frame不再送packet
        if (packet->data == NULL && packet->size == 0)
        {
            is->p_vcodec_ctx->flags |= (1 << 5);
            printf("send a null paket to decoder\n");
        }
        else
        {
            is->p_vcodec_ctx->flags &= ~(1 << 5);
        }

        ret = avcodec_send_packet(is->p_vcodec_ctx, packet);
        if (ret == AVERROR(EAGAIN))
        {
            av_log(NULL, AV_LOG_ERROR, "receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
        }

        ret = avcodec_receive_frame(is->p_vcodec_ctx, frame);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
            {
                av_log(NULL, AV_LOG_INFO, "video decoder has been fully flushed\n");
                avcodec_flush_buffers(is->p_vcodec_ctx);
            }
            else
            {
                //av_log(NULL, AV_LOG_ERROR, "ravcodec_receive_frame failed!\n");
                *remaining_time = REFRESH_RATE;
                counter ++;
                if (counter > 300)
                {
                    av_log(NULL, AV_LOG_ERROR, "can't get a frame from vdec for long time!\n");
                    is->play_error = -4;
                }
            }
        }
        else
        {
            counter = 0;
            // 此时的pts为当前显示图像的pts
            frame->pts = frame->best_effort_timestamp;
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            //printf("frame pts : %.4lf, pos : %d\n", pts, frame->pkt_pos);
            pthread_mutex_lock(&is->video_frm_queue.mutex);
            if (!isnan(pts))
            {
                update_video_pts(is, pts, frame->pkt_pos, 0); // 更新视频时钟：时间戳、时钟时间
            }
            pthread_mutex_unlock(&is->video_frm_queue.mutex);

            // update ui pos
            if (is->playerController.fpGetCurrentPlayPosFromVideo)
            {
                long long videoPts = (long long)(is->video_clk.pts * 1000000LL) - is->p_fmt_ctx->start_time;
                long long frame_duration = 1000000 * duration;
                is->playerController.fpGetCurrentPlayPosFromVideo(videoPts, frame_duration);
            }

            // 如果视频时间落后于音频时间太久则跳帧
            if (time < is->frame_timer + 2 * duration)
            {
                // send data to disp
                if (is->decode_type == SOFT_DECODING)
                {
                    sws_scale(is->img_convert_ctx,                    // sws context
                              (const uint8_t *const *)frame->data,    // src slice
                              frame->linesize,                        // src stride
                              0,                                      // src slice y
                              is->p_vcodec_ctx->height,               // src slice height
                              is->p_frm_yuv->data,                    // dst planes
                              is->p_frm_yuv->linesize                 // dst strides
                              );
                    frame_ydata  = is->p_frm_yuv->data[0];
                    frame_uvdata = is->p_frm_yuv->data[1];
                }
                else if (is->decode_type == HARD_DECODING)
                {
                    frame_ydata  = frame->data[0];
                    frame_uvdata = frame->data[1];
                }
                // 跳转到UI执行显示函数
                if (is->playerController.fpDisplayVideo) {
                    is->playerController.fpDisplayVideo(frame->width, frame->height, frame_ydata, frame_uvdata);
                }
            }
        }
    }

    av_packet_free(&packet);
    av_frame_free(&frame);

    if (is->step && !is->paused)
    {
        stream_toggle_pause(is);
    }

    return 0;
}
#endif

static void* video_playing_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
    double remaining_time = 0.0;

    double duration = av_q2d((AVRational){frame_rate.den, frame_rate.num});
    printf("video image rate time: %.3lf\n", duration);

    printf("video_playing_thread in\n");
    is->frame_timer = av_gettime_relative() / 1000000.0;

    while (1)
    {
        if(is->abort_request)
        {
            printf("video play thread exit\n");
            break;
        }
        if (remaining_time > 0.0)
        {
            av_usleep((unsigned)(remaining_time * 1000000.0));
        }
        remaining_time = REFRESH_RATE;
        // 立即显示当前帧，或延时remaining_time后再显示
        video_refresh(is, &remaining_time, duration);
    }

    return NULL;
}

static int open_video_playing(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    int ret;
    int dst_width, dst_height;
    const AVPixFmtDescriptor *desc;

    if (is->decode_type == SOFT_DECODING)
    {
        is->p_frm_yuv = av_frame_alloc();
        if (is->p_frm_yuv == NULL)
        {
            printf("av_frame_alloc() for p_frm_raw failed\n");
            return -1;
        }

        if (is->p_vcodec_ctx->width > is->p_vcodec_ctx->height) {
            dst_width  = FFMIN(is->p_vcodec_ctx->width, 1920);
            dst_height = FFMIN(is->p_vcodec_ctx->height, 1080);
        } else {
            dst_width  = FFMIN(is->p_vcodec_ctx->width, 1080);
            dst_height = FFMIN(is->p_vcodec_ctx->height, 1920);
        }

        // 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
        is->buf_size = av_image_get_buffer_size(AV_PIX_FMT_NV12, 
                                                dst_width,
                                                dst_height,
                                                1
                                                );
        printf("alloc size: %d,width: %d,height: %d\n", is->buf_size, dst_width, dst_height);

        // buffer将作为p_frm_yuv的视频数据缓冲区
        //is->vir_addr = (uint8_t *)av_malloc(is->buf_size);
        //if (buffer == NULL)
        //{
        //    printf("av_malloc() for buffer failed\n");
        //    return -1;
        //}

        //ret = MI_SYS_MMA_Alloc((MI_U8 *)"MMU_MMA", is->buf_size, &is->phy_addr);
        ret = MI_SYS_MMA_Alloc((MI_U8 *)"#yuv420p", is->buf_size, &is->phy_addr);
        if (ret != MI_SUCCESS) {
            av_log(NULL, AV_LOG_ERROR, "MI_SYS_MMA_Alloc Falied!\n");
            return -1;
        }

        ret = MI_SYS_Mmap(is->phy_addr, is->buf_size, (void **)&is->vir_addr, TRUE);
        if (ret != MI_SUCCESS) {
            av_log(NULL, AV_LOG_ERROR, "MI_SYS_Mmap Falied!\n");
            return -1;
        }

        // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
        is->p_frm_yuv->width  = dst_width;
        is->p_frm_yuv->height = dst_height;
        ret = av_image_fill_arrays(is->p_frm_yuv->data,     // dst data[]
                                   is->p_frm_yuv->linesize, // dst linesize[]
                                   is->vir_addr,            // src buffer
                                   AV_PIX_FMT_NV12,         // pixel format
                                   dst_width,
                                   dst_height,
                                   1                        // align
                                   );
        if (ret < 0)
        {
            printf("av_image_fill_arrays() failed %d\n", ret);
            return -1;;
        }

        // A2. 初始化SWS context，用于后续图像转换
        //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B3
        //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
        //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
        //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
        //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
        desc = av_pix_fmt_desc_get(is->p_vcodec_ctx->pix_fmt);
        printf("video prefix format : %s.\n", desc->name);

        is->img_convert_ctx = sws_getContext(is->p_vcodec_ctx->width,   // src width
                                             is->p_vcodec_ctx->height,  // src height
                                             is->p_vcodec_ctx->pix_fmt, // src format
                                             dst_width,
                                             dst_height,
                                             AV_PIX_FMT_NV12,           // dst format
                                             SWS_FAST_BILINEAR,         // flags
                                             NULL,                      // src filter
                                             NULL,                      // dst filter
                                             NULL                       // param
                                             );
        if (is->img_convert_ctx == NULL)
        {
            printf("sws_getContext() failed\n");
            return -1;
        }
    }

    prctl(PR_SET_NAME, "video_play");
    ret = pthread_create(&is->videoPlay_tid, NULL, video_playing_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "video_playing_thread create failed!\n");
        is->videoPlay_tid = 0;
        return ret;
    }

    return 0;
}

static int open_video_stream(player_stat_t *is)
{
    AVCodecParameters* p_codec_par = NULL;
    AVCodec* p_codec = NULL;
    AVCodecContext* p_codec_ctx = NULL;
    AVStream *p_stream = is->p_video_stream;
    int ret, tmp;

    // 1. 为视频流构建解码器AVCodecContext
    // 1.1 获取解码器参数AVCodecParameters
    p_codec_par = p_stream->codecpar;

    // 1.2 获取解码器
    // 对于h264/h265编码的视频选择硬解,其他编码格式使用ffmpeg软解
    switch (p_codec_par->codec_id) 
    {
        case AV_CODEC_ID_H264 :
            p_codec = avcodec_find_decoder_by_name("ssh264"); 
            is->decode_type = HARD_DECODING;
            break;

        case AV_CODEC_ID_HEVC :
            p_codec = avcodec_find_decoder_by_name("sshevc"); 
            is->decode_type = HARD_DECODING;
            break;

        default :
            p_codec = avcodec_find_decoder(p_codec_par->codec_id);
            is->decode_type = SOFT_DECODING;
            break;
    }  
    if (p_codec == NULL)
    {
        printf("Cann't find video codec!\n");
        return -1;
    }
    printf("open video codec: %s\n", p_codec->name);

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        printf("avcodec_alloc_context3() failed\n");
        return -1;
    }

    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        printf("avcodec_parameters_to_context() failed\n");
        return -1;
    }

    if (p_codec_par->width <= 0 || p_codec_par->height <= 0)
    {
        printf("video w/h = [%d %d] isn't invalid!\n", p_codec_par->width, p_codec_par->height);
        return -1;
    }

    /*if ((is->in_width > is->in_height && p_codec_par->width < p_codec_par->height)
     || (is->in_width < is->in_height && p_codec_par->width > p_codec_par->height)) {
        is->display_mode = E_MI_DISP_ROTATE_270;    // 如果视频的宽高与显示屏的宽高不匹配自动旋转270度
        tmp = is->in_width;
        is->in_width  = is->in_height;
        is->in_height = tmp;                        // 交换宽高
    } else {
        is->display_mode = E_MI_DISP_ROTATE_NONE;
    }*/

    if (is->decode_type == HARD_DECODING) {
        /*if (1.0 * p_codec_par->width / p_codec_par->height > 1.0 * is->in_width / is->in_height) {
            is->out_width  = is->in_width;
            is->out_height = is->in_width * p_codec_par->height / p_codec_par->width;
            is->src_width  = FFMIN(p_codec_par->width , is->out_width);
            is->src_height = FFMIN(p_codec_par->height, is->out_height);
            p_codec_ctx->flags  = ALIGN_BACK(is->src_width , 32);
            p_codec_ctx->flags2 = ALIGN_BACK(is->src_height, 32);
            if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
                is->pos_x = FFMAX((is->in_height - is->out_height) / 2, 0);
                is->pos_y = 0;
            } else {
                is->pos_x = 0;
                is->pos_y = FFMAX((is->in_height - is->out_height) / 2, 0);
            }
        } else {
            is->out_width  = is->in_height * p_codec_par->width / p_codec_par->height;
            is->out_height = is->in_height;
            is->src_width  = FFMIN(p_codec_par->width , is->out_width);
            is->src_height = FFMIN(p_codec_par->height, is->out_height);
            p_codec_ctx->flags  = ALIGN_BACK(is->src_width , 32);
            p_codec_ctx->flags2 = ALIGN_BACK(is->src_height, 32);
            if (is->display_mode != 0) {
                is->pos_x = 0;
                is->pos_y = FFMAX((is->in_width - is->out_width) / 2, 0);
            } else {
                is->pos_x = FFMAX((is->in_width - is->out_width) / 2, 0);
                is->pos_y = 0;
            }
        }*/

        if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
            p_codec_ctx->flags  = FFMIN(ALIGN_BACK(is->in_height, 32), ALIGN_BACK(p_codec_par->width , 32));
            p_codec_ctx->flags2 = FFMIN(ALIGN_BACK(is->in_width , 32), ALIGN_BACK(p_codec_par->height, 32));
            is->out_width  = is->in_width;
            is->out_height = is->in_height;
            is->src_width  = FFMIN(is->out_height, p_codec_par->width);
            is->src_height = FFMIN(is->out_width , p_codec_par->height);
            // 使用disp旋转时需要开启tilemode
            p_codec_ctx->flags |= (1 << 17);
            av_log(NULL, AV_LOG_WARNING, "set rotate attribute!\n");
        } else {
            // 硬解时选择VDEC缩小画面,传入宽高,由于VDEC只能缩小这里对宽高进行判断
            p_codec_ctx->flags  = FFMIN(ALIGN_BACK(is->in_width , 32), ALIGN_BACK(p_codec_par->width , 32));
            p_codec_ctx->flags2 = FFMIN(ALIGN_BACK(is->in_height, 32), ALIGN_BACK(p_codec_par->height, 32));
            is->out_width  = is->in_width;
            is->out_height = is->in_height;
            is->src_width  = FFMIN(is->out_width , p_codec_par->width);
            is->src_height = FFMIN(is->out_height, p_codec_par->height);
        }

        printf("vdec out w/h = [%d %d], display x/y/w/h = [%d %d %d %d]\n", 
        (p_codec_ctx->flags & 0xFFFF), (p_codec_ctx->flags2 & 0xFFFF), is->pos_x, is->pos_y, is->out_width, is->out_height);
    }
    else {
        /*if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
            // 根据输入信号的横竖属性设置显示窗口
            if (p_codec_ctx->width > p_codec_ctx->height) {
                is->src_height = FFMIN(p_codec_ctx->width , is->in_width);
                is->src_width  = FFMIN(p_codec_ctx->height, is->in_height);
                is->out_width  = FFMIN(p_codec_ctx->height * is->in_width / p_codec_ctx->width, is->in_height);
                is->out_height = is->in_width;
                is->pos_x = FFMAX((is->in_height - is->out_width) / 2, 0);
                is->pos_y = 0;
                is->dst_height = FFMIN(p_codec_ctx->width, 1920);
                is->dst_width  = FFMIN(p_codec_ctx->height, 1080);
            } else {
                is->src_height = FFMIN(p_codec_ctx->width * is->in_height / p_codec_ctx->height, p_codec_ctx->width);
                is->src_width  = FFMIN(p_codec_ctx->height, is->in_height);
                is->out_width  = is->in_height;
                is->out_height = p_codec_ctx->width * is->in_height / p_codec_ctx->height;
                is->pos_x = 0;
                is->pos_y = FFMAX((is->in_width - is->out_height) / 2, 0);
                is->dst_height = FFMIN(p_codec_ctx->width, 1080);
                is->dst_width  = FFMIN(p_codec_ctx->height, 1920);
            }
        } else {
            if (p_codec_ctx->width > p_codec_ctx->height) {
                is->src_width  = FFMIN(p_codec_ctx->width , is->in_width);
                is->src_height = FFMIN(p_codec_ctx->height , is->in_height);
                is->out_width  = is->in_width;
                is->out_height = FFMIN(p_codec_par->height * is->in_width / p_codec_par->width, is->in_height);
                is->pos_x = 0;
                is->pos_y = FFMAX((is->in_height - is->out_height) / 2, 0);
                is->dst_width  = FFMIN(p_codec_ctx->width, 1920);
                is->dst_height = FFMIN(p_codec_ctx->height, 1080);
            } else {
                is->src_width  = FFMIN(p_codec_ctx->width, p_codec_ctx->width * is->in_height / p_codec_ctx->height);
                is->src_height = FFMIN(p_codec_ctx->height , is->in_height);
                is->out_width  = p_codec_ctx->width * is->in_height / p_codec_ctx->height;
                is->out_height = is->in_height;
                is->pos_x = FFMAX((is->in_width - is->out_width) / 2, 0);
                is->pos_y = 0;
                is->dst_width  = FFMIN(p_codec_ctx->width, 1080);
                is->dst_height = FFMIN(p_codec_ctx->height, 1920);
            }
        }*/
        is->out_width  = is->in_width;
        is->out_height = is->in_height;
        if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
            is->src_width  = FFMIN(is->out_width , p_codec_par->height);
            is->src_height = FFMIN(is->out_height, p_codec_par->width);
        } else {
            is->src_width  = FFMIN(is->out_width , p_codec_par->width);
            is->src_height = FFMIN(is->out_height, p_codec_par->height);
        }
        printf("scaler src w/h = [%d %d], dst x/y/w/h = [%d %d %d %d]\n", is->src_width, is->src_height, is->pos_x, is->pos_y, is->out_width, is->out_height);
    }
    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2() failed %d\n", ret);
        return -1;
    }

    // 初始化video
    if (is->playerController.fpSetVideoDisplay)
        is->playerController.fpSetVideoDisplay();

    is->enable_video = true;

    is->p_vcodec_ctx = p_codec_ctx;
    is->p_vcodec_ctx->debug  = true;
    printf("video codec width: %d,height: %d\n", is->p_vcodec_ctx->width,is->p_vcodec_ctx->height);

    // 2. 创建视频解码线程
    prctl(PR_SET_NAME, "video_decode");
    ret = pthread_create(&is->videoDecode_tid, NULL, video_decode_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "video_decode_thread create failed!\n");
        is->videoDecode_tid = 0;
        return ret;
    }

    return 0;
}

int video_buffer_flush(player_stat_t *is)
{
    int i, ret = 0;
    frame_queue_t *f = &is->video_frm_queue;

    pthread_mutex_lock(&f->mutex);
    for (i = 0; i < f->max_size; i ++) {
        frame_t *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    f->rindex = 0;
    f->rindex_shown = 0;
    f->windex = 0;
    f->size   = 0;
    f->max_size = 0;
    pthread_cond_signal(&f->cond);
    pthread_mutex_unlock(&f->mutex);
    return ret;
}


int open_video(player_stat_t *is)
{
    int ret;
    if (is && !is->play_error && is->video_idx >= 0)
    {
        ret = open_video_stream(is);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "open_video_stream failed!\n");
            return ret;
        }

        ret = open_video_playing(is);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "open_video_playing failed!\n");
            return ret;
        }

        av_log(NULL, AV_LOG_INFO, "open_video success!\n");
        return 0;
    }
    else
    {
        av_log(NULL, AV_LOG_ERROR, "open_video failed!\n");
        return -1;
    }
}
#endif
#endif
