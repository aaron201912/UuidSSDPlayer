#include "videostream.h"
#include "packet.h"
#include "frame.h"
#include "player.h"
#include "blitutil.h"

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

static int alloc_for_frame(frame_t *vp, AVFrame *frame)
{
    int ret;

    vp->buf_size = av_image_get_buffer_size(frame->format, frame->width, frame->height, 1);
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
                               frame->format,       // pixel format
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
    int ret;
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
    //printf("second frame->buf[0] addr : %p, vdec buf addr : %p\n", src_frame->buf[0], src_frame->opaque);

    // 将AVFrame拷入队列相应位置
    if (is->decoder_type == SOFT_DECODING)
    {
        av_frame_move_ref(vp->frame, src_frame);

        //gettimeofday(&trans_start, NULL);
        //ret = alloc_for_frame(vp, src_frame);
        //if (ret < 0) {
        //    av_log(NULL, AV_LOG_ERROR, "alloc_for_frame failed!\n");
        //    return 0;
        //}
        //gettimeofday(&trans_end, NULL);
        //time0 = ((int64_t)trans_end.tv_sec * 1000000 + trans_end.tv_usec) - ((int64_t)trans_start.tv_sec * 1000000 + trans_start.tv_usec);
        //printf("time of alloc_for_frame : %lldus\n", time0);

        //printf("three frame->buf[0] addr : %p, vdec buf addr : %p\n", vp->frame->buf[0], vp->frame->opaque); 
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
                    //av_log(NULL, AV_LOG_ERROR, "ret : %d, cann't fetch a frame, try again!\n", ret);
                    break;
                }
                else
                {
                    av_log(NULL, AV_LOG_ERROR, "video avcodec_receive_frame(): other errors\n");
                    continue;
                }
            }
            else
            {
                frame->pts = frame->best_effort_timestamp;
                //printf("best_effort_timestamp : %lld, frame number = %d\n", frame->pts, p_codec_ctx->frame_number);
                //printf("frame pos: %lld\n",frame->pkt_pos);
                return 1;   // 成功解码得到一个视频帧或一个音频帧，则返回
            }
        }

        // 1. 取出一个packet。使用pkt对应的serial赋值给d->pkt_serial
        //printf("packet_queue_get start! num : %d\n", p_pkt_queue->nb_packets);
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
                frame_queue_flush(&g_myplayer->video_frm_queue);
            }
            pthread_mutex_unlock(&g_myplayer->video_mutex);

            // 复位解码器内部状态/刷新内部缓冲区。
            avcodec_flush_buffers(p_codec_ctx);

            printf("avcodec_flush_buffers for video!\n");
        }
        else
        {
            // 如果是最后一个空的packet,只取frame不再送packet
            if (pkt.data == NULL || pkt.size == 0) {
                p_codec_ctx->flags |= (1 << 5);
                printf("send a null paket to decoder\n");
            } else{
                p_codec_ctx->flags &= ~(1 << 5);
            }
            // 2. 将packet发送给解码器
            //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
            //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
            //printf("send packet to decoder!\n");
            //printf("pkt pos: %lld\n",pkt.pos);
            if (avcodec_send_packet(p_codec_ctx, &pkt) == AVERROR(EAGAIN))
            {
                av_log(NULL, AV_LOG_ERROR, "receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            }
        }
        av_packet_unref(&pkt);
    }
}

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
        }

        av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%.3f\n", delay, -diff);
    }

    return delay;
}


#if 0
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
#endif

static void update_video_pts(player_stat_t *is, double pts, int64_t pos, int serial) {
    /* update current video pts */
    set_clock(&is->video_clk, pts, serial);            // 更新vidclock
    //-sync_clock_to_slave(&is->extclk, &is->vidclk);  // 将extclock同步到vidclock
}

static void sstar_video_rotate(player_stat_t *is, MI_PHY yAddr, MI_PHY uvAddr)
{
    Surface srcY, dstY;
    Surface srcUV, dstUV;
    RECT r;
    srcY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    srcY.h              = is->p_vcodec_ctx->height;
    srcY.phy_addr       = is->phy_addr;
    srcY.pitch          = is->p_vcodec_ctx->width;
    srcY.w              = is->p_vcodec_ctx->width;
    srcY.BytesPerPixel  = 1;

    dstY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    dstY.h              = srcY.w;
    dstY.phy_addr       = yAddr;
    dstY.pitch          = ALIGN_UP(srcY.h, 16);
    dstY.w              = srcY.h;
    dstY.BytesPerPixel  = 1;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcY.h;
    r.right  = srcY.w;
    if (is->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcY, &dstY, &r);
    }
    else if (is->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcY, &dstY, &r);
    }

    srcUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    srcUV.h             = is->p_vcodec_ctx->height / 2;
    srcUV.phy_addr      = is->phy_addr + is->p_vcodec_ctx->width * is->p_vcodec_ctx->height;
    srcUV.pitch         = is->p_vcodec_ctx->width;
    srcUV.w             = is->p_vcodec_ctx->width / 2;
    srcUV.BytesPerPixel = 2;

    dstUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    dstUV.h             = srcUV.w;
    dstUV.phy_addr      = uvAddr;
    dstUV.pitch         = ALIGN_UP(srcY.h, 16);
    dstUV.w             = srcUV.h;
    dstUV.BytesPerPixel = 2;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcUV.h;
    r.right  = srcUV.w;
    if (is->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcUV, &dstUV, &r);
    }
    else if (is->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcUV, &dstUV, &r);
    }
}


static int video_load_picture(player_stat_t *is, AVFrame *frame)
{
    if (is->decoder_type == SOFT_DECODING) {
        MI_SYS_ChnPort_t  stInputChnPort;
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId     = E_MI_MODULE_ID_DIVP;
        stInputChnPort.u32ChnId   = 0;
        stInputChnPort.u32DevId   = 0;
        stInputChnPort.u32PortId  = 0;

        //gettimeofday(&time_start, NULL);
        // YUV格式统一转换成NV12
        sws_scale(is->img_convert_ctx,                  // sws context
                  (const uint8_t *const *)frame->data,  // src slice
                  frame->linesize,                      // src stride
                  0,                                    // src slice y
                  is->p_vcodec_ctx->height,             // src slice height
                  is->p_frm_yuv->data,                  // dst planes
                  is->p_frm_yuv->linesize               // dst strides
                  );
        //gettimeofday(&time_end, NULL);
        //time0 = ((int64_t)time_end.tv_sec * 1000000 + time_end.tv_usec) - ((int64_t)time_start.tv_sec * 1000000 + time_start.tv_usec);

        //int length = is->p_frm_yuv->width * is->p_frm_yuv->height * 3 / 2;
        //fwrite(is->p_frm_yuv->data[0], length, 1, dump_fp);

        MI_SYS_BufConf_t stBufConf;
        MI_SYS_BufInfo_t stBufInfo;
        MI_SYS_BUF_HANDLE bufHandle;
        memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
        stBufConf.eBufType              = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.stFrameCfg.eFormat    = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
            stBufConf.stFrameCfg.u16Height  = is->p_frm_yuv->width;
            stBufConf.stFrameCfg.u16Width   = is->p_frm_yuv->height;
        } else {
            stBufConf.stFrameCfg.u16Height  = is->p_frm_yuv->height;
            stBufConf.stFrameCfg.u16Width   = is->p_frm_yuv->width;
        }
        stBufConf.u32Flags              = MI_SYS_MAP_VA;
        stBufConf.stFrameCfg.stFrameBufExtraConf.u16BufHAlignment = 16;

        if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stInputChnPort, &stBufConf, &stBufInfo, &bufHandle, 0))
        {
            // flush需要时间分辨率大于720P就不执行该操作但是会导致图像拉丝
            if (is->p_frm_yuv->width * is->p_frm_yuv->height < 1024 * 600) {
                MI_SYS_FlushInvCache(is->vir_addr, is->buf_size);
            }

            // GFX旋转图片
            if (is->display_mode != E_MI_DISP_ROTATE_NONE) {
                //gettimeofday(&time_start, NULL);
                sstar_video_rotate(is, stBufInfo.stFrameData.phyAddr[0], stBufInfo.stFrameData.phyAddr[1]);
                //gettimeofday(&time_end, NULL);

                //int length = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0] * 3 / 2;
                //fwrite(stBufInfo.stFrameData.pVirAddr[0], length, 1, dump_fp);
                //printf("stBufInfo width and height : [%d %d]\n", stBufInfo.stFrameData.u32Stride[0], stBufInfo.stFrameData.u16Height);
            } else {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType    = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream              = FALSE;

                int length = is->p_frm_yuv->width * is->p_frm_yuv->height;
                for (int index = 0; index < stBufInfo.stFrameData.u16Height; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[0] + index * stBufInfo.stFrameData.u32Stride[0], 
                                    is->phy_addr + index * is->p_frm_yuv->width, is->p_frm_yuv->width);
                }
                for (int index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[1] + index * stBufInfo.stFrameData.u32Stride[1], 
                                    is->phy_addr + length + index * is->p_frm_yuv->width, is->p_frm_yuv->width);
                }
            }

            MI_SYS_ChnInputPortPutBuf(bufHandle, &stBufInfo, FALSE);
        }

        //time1 = ((int64_t)time_end.tv_sec * 1000000 + time_end.tv_usec) - ((int64_t)time_start.tv_sec * 1000000 + time_start.tv_usec);
        //printf("time of sws_scale : %lldus, time of rotate : %lldus\n", time0, time1);
    }
    else if (is->decoder_type == HARD_DECODING) {
        MI_SYS_ChnPort_t  stInputChnPort;
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId                    = E_MI_MODULE_ID_DISP;
        stInputChnPort.u32ChnId                  = 0;
        stInputChnPort.u32DevId                  = 0;
        stInputChnPort.u32PortId                 = 0;

        av_assert0(frame->opaque);
        SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)frame->opaque; 

        if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(stVdecBuf->stVdecHandle, &stInputChnPort))
            av_log(NULL, AV_LOG_ERROR, "MI_SYS_ChnPortInjectBuf failed!\n");

        av_freep(&frame->opaque);
    }

    return 0;
}

static void video_display(player_stat_t *is)
{
    frame_t *vp;

    //av_log(NULL, AV_LOG_ERROR, "rindex : %d, shownidex : %d, size : %d\n", is->video_frm_queue.rindex, is->video_frm_queue.rindex_shown, is->video_frm_queue.size);
    vp = frame_queue_peek_last(&is->video_frm_queue);
    //vp = frame_queue_peek(&is->video_frm_queue);
    if (!vp->frame->width || !vp->frame->height)
    {
        av_log(NULL, AV_LOG_ERROR, "invalid frame width and height!\n");
        return;
    }

    video_load_picture(is, vp->frame);

    //gettimeofday(&time_end, NULL);
    //time0 = ((int64_t)time_end.tv_sec * 1000000 + time_end.tv_usec) - ((int64_t)time_start.tv_sec * 1000000 + time_start.tv_usec);
    //time_start.tv_sec  = time_end.tv_sec;
    //time_start.tv_usec = time_end.tv_usec;
    //printf("time of video_display : %lldus\n", time0);
}

/* called to display each frame */
static int video_refresh(void *opaque, double *remaining_time, double duration)
{
    player_stat_t *is = (player_stat_t *)opaque;
    double time, delay;
    frame_t *vp;

    if (is->frame_timer < 0.1) {
        is->frame_timer = av_gettime_relative() / 1000000.0;
        //av_log(NULL, AV_LOG_INFO, "is->frame_timer first value : %.3f\n", is->frame_timer);
    }
retry:
    // 暂停处理：不停播放上一帧图像
    if (is->paused)
        return 0;
recheck:
    while (is->seek_flags & (1 << 6)) {
        is->start_play = false;
        pthread_cond_signal(&is->video_frm_queue.cond);
        av_usleep(10 * 1000);
    }

    while(is->no_pkt_buf && !is->abort_request) {
        av_usleep(10 * 1000);
        goto recheck;
    }

    //printf("f->size = %d, f->rindex_shown = %d\n",is->video_frm_queue.size, is->video_frm_queue.rindex_shown);
    if (frame_queue_nb_remaining(&is->video_frm_queue) <= 0)  // 所有帧已显示
    {    
        // nothing to do, no picture to display in the queue
        //printf("already last frame: %d\n",is->video_frm_queue.size);
        if (!is->video_complete && is->eof && is->video_pkt_queue.nb_packets == 0 && is->start_play)
        {
            is->video_complete = 1;
            if (is->video_complete && is->audio_complete) {
                //stream_seek(is, is->p_fmt_ctx->start_time, 0, 0);
                is->play_status = 1;
            }
            NANOX_LOG("video play completely!\n");
        }
        return 0;
    }
    //av_log(NULL, AV_LOG_ERROR, "frame_queue_nb_remaining done!\n");
    /* dequeue the picture */
    //lastvp = frame_queue_peek_last(&is->video_frm_queue);
    vp = frame_queue_peek(&is->video_frm_queue);              // 当前帧：当前待显示的帧
    //printf("refresh ridx: %d,rs:%d,widx: %d,size: %d,maxsize: %d\n",is->video_frm_queue.rindex,is->video_frm_queue.rindex_shown,is->video_frm_queue.windex,is->video_frm_queue.size,is->video_frm_queue.max_size);
    /* compute nominal last_duration */
    //duration = vp_duration(is, lastvp, vp);
    delay = compute_target_delay(duration, is);    // 根据视频时钟和同步时钟的差值，计算delay值
    //printf("last_duration: %lf,delay: %lf\n", last_duration, delay);
    time= av_gettime_relative()/1000000.0;
    // 当前帧播放时刻(is->frame_timer+delay)大于当前时刻(time)，表示播放时刻未到
    if (time < is->frame_timer + delay && is->start_play) {
        // 播放时刻未到，则更新刷新时间remaining_time为当前时刻到下一播放时刻的时间差
        *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
        // 播放时刻未到，则不播放，直接返回
        //printf("not ready play\n");
        return 0;
    }
    //printf("remaining time : %f. duration : %f.\n", *remaining_time, last_duration);
    // 更新frame_timer值
    is->frame_timer += delay;
    //printf("frame_timer : %0.6lf, video pts : %0.6lf, mremaining : %0.6lf\n", is->frame_timer, vp->pts, *remaining_time);
    // 校正frame_timer值：若frame_timer落后于当前系统时间太久(超过最大同步域值)，则更新为当前系统时间
    if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
    {
        is->frame_timer = time;
        //printf("adjust frame timer to system time!\n");
    }

    pthread_mutex_lock(&is->video_frm_queue.mutex);
    if (!isnan(vp->pts))
    {
        //int64_t video_time = av_gettime_relative();
        //double diff = video_time / 1000000.0 - is->audio_clk.last_updated;
        //printf("video call back time : %lld, diff = %.6f\n", video_time, diff);
        update_video_pts(is, vp->pts, vp->pos, vp->serial); // 更新视频时钟：时间戳、时钟时间
    }
    pthread_mutex_unlock(&is->video_frm_queue.mutex);

    // 是否要丢弃未能及时播放的视频帧
    if (frame_queue_nb_remaining(&is->video_frm_queue) > 1)  // 队列中未显示帧数>1(只有一帧则不考虑丢帧)
    {
        // 当前帧vp未能及时播放，即下一帧播放时刻(is->frame_timer+duration)小于当前系统时刻(time)
        if (time > is->frame_timer + duration && is->start_play)
        {
            /*if (is->decoder_type == HARD_DECODING) {
                if (vp->frame->opaque) {
                    frame_queue_putbuf(vp->frame);
                    av_freep(&vp->frame->opaque);
                }
            }*/
            frame_queue_next(&is->video_frm_queue);   // 删除上一帧已显示帧，即删除lastvp，读指针加1(从lastvp更新到vp)
            //av_log(NULL, AV_LOG_WARNING, "discard current frame!\n");
            goto retry;
        }
    }

    //AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
    //*remaining_time = av_q2d((AVRational){frame_rate.den, frame_rate.num});    //no sync
    //printf("remaining time : %f.\n", (*remaining_time) * AV_TIME_BASE);
    // 删除当前读指针元素，读指针+1。若未丢帧，读指针从lastvp更新到vp；若有丢帧，读指针从vp更新到nextvp
    frame_queue_next(&is->video_frm_queue);

    if (is->step && !is->paused) {
        stream_toggle_pause(is);
    }

    video_display(is);                      // 取出当前帧vp(若有丢帧是nextvp)进行播放

    if (!is->start_play) {              // 播放第一张图片后认为开始播放
        is->start_play = true;
    }

    return 0;
}

// 将视频包解码得到视频帧，然后写入picture队列
static void * video_decode_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    double pts, duration;
    int ret, got_picture;
    AVRational tb = is->p_video_stream->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);

    AVFrame *p_frame = av_frame_alloc();
    if (p_frame == NULL) {
        av_log(NULL, AV_LOG_ERROR, "av_frame_alloc() for p_frame failed\n");
        return NULL;
    }

    duration = av_q2d((AVRational){frame_rate.den, frame_rate.num});
    printf("video time base : %f ms.\n", 1000 * av_q2d(tb));
    printf("fps : %.3f, frame rate num : %d. frame rate den : %d.\n", duration, frame_rate.num, frame_rate.den);
    printf("get in video decode thread!\n");

    while (1)
    {
        if(is->abort_request) {
            printf("video decode thread exit\n");
            break;
        }
        
        got_picture = video_decode_frame(is->p_vcodec_ctx, &is->video_pkt_queue, p_frame);
        if (got_picture < 0)
        {
            printf("video got pic fail\n");
            goto exit;
        } 
        else if (got_picture > 0)
        {
            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);   // 当前帧播放时长
            pts = (p_frame->pts == AV_NOPTS_VALUE) ? NAN : p_frame->pts * av_q2d(tb);   // 当前帧显示时间戳

            //printf("frame duration : %f. video frame clock : %f.\n", duration, pts);
            ret = queue_picture(is, p_frame, pts, duration, p_frame->pkt_pos);   // 将当前帧压入frame_queue
            av_frame_unref(p_frame);
            //if (NULL == frame_queue_peek_writable(&is->video_frm_queue)) {
            //    ret = -1;
            //    goto exit;
            //}
            frame_queue_t *f = &is->video_frm_queue;
            pthread_mutex_lock(&f->mutex);
            while (f->size >= f->max_size && !f->pktq->abort_request) {
                pthread_cond_wait(&f->cond, &f->mutex);
                if (is->seek_flags & (1 << 6)) {
                    break;
                }
            }
            pthread_mutex_unlock(&f->mutex);

            if (f->pktq->abort_request) {
                continue;
            }
        }
        if (ret < 0) {
            printf("queue_picture exit\n");
            goto exit;
        }
    }

exit:

    av_frame_free(&p_frame);

    return NULL;
}

static void * video_playing_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVRational frame_rate = av_guess_frame_rate(is->p_fmt_ctx, is->p_video_stream, NULL);
    double duration = av_q2d((AVRational){frame_rate.den, frame_rate.num});
    double remaining_time = 0.0;

    printf("video fps time : %.3lf\n", duration);

    printf("video_playing_thread in\n");

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

    if (is->decoder_type == SOFT_DECODING)
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
                                             SWS_POINT,                 // flags
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
    ret = pthread_create(&is->video_play_tid, NULL, video_playing_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "video_playing_thread create failed!\n");
        is->video_play_tid = 0;
        return -1;
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
    switch(p_codec_par->codec_id) 
    {
        case AV_CODEC_ID_H264 : 
            p_codec = avcodec_find_decoder_by_name("ssh264"); 
            is->decoder_type = HARD_DECODING;
            break;

        case AV_CODEC_ID_HEVC : 
            p_codec = avcodec_find_decoder_by_name("sshevc"); 
            is->decoder_type = HARD_DECODING;
            break;

        default : 
            p_codec = avcodec_find_decoder(p_codec_par->codec_id); 
            is->decoder_type = SOFT_DECODING;
            break;
    }
    //p_codec = avcodec_find_decoder(p_codec_par->codec_id); 
    if (p_codec == NULL)
    {
        printf("Cann't find video codec!\n");
        return -1;
    }
    printf("open video codec: %s\n", p_codec->name);
    if (0 == strcmp(p_codec->name, "mjpeg")) {
        is->flush = true;
    } else {
        is->flush = false;
    }

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

    if (is->decoder_type == HARD_DECODING) {
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
        // 针对竖屏进行旋转设置
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

    ret = my_display_set(is);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "my_display_set failed!\n");
        return ret;
    }
    is->enable_video = true;

    is->p_vcodec_ctx = p_codec_ctx;
    is->p_vcodec_ctx->debug  = true;
    printf("codec width: %d,height: %d\n",is->p_vcodec_ctx->width,is->p_vcodec_ctx->height);
    //printf("bistream width : %d, height : %d\n", is->p_vcodec_ctx->coded_width,is->p_vcodec_ctx->coded_height);

    // 2. 创建视频解码线程
    prctl(PR_SET_NAME, "video_decode_tid");
    ret = pthread_create(&is->video_decode_tid, NULL, video_decode_thread, (void *)is);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "video_decode_thread create failed!\n");
        is->video_decode_tid = 0;
        return -1;
    }

    return 0;
}

int open_video(player_stat_t *is)
{
    int ret;

    if (is && is->video_idx >= 0) {
        ret = open_video_stream(is);
        if (ret < 0)
            return ret;

        ret = open_video_playing(is);
        if (ret < 0)
            return ret;
    } 

    return 0;
}

int video_flush_buffer(frame_queue_t *f)
{
    int i, ret = 0;

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

int my_display_set(player_stat_t *is)
{
    MI_DISP_RotateConfig_t stRotateConfig;

    if (!is) {
        av_log(NULL, AV_LOG_ERROR, "sstar_display_set failed!\n");
        return -1;
    }

    av_log(NULL, AV_LOG_WARNING, "display width and height = [%d %d]!\n", is->out_width, is->out_height);

    MI_DISP_DisableInputPort(0, 0);
    memset(&stRotateConfig, 0, sizeof(MI_DISP_RotateConfig_t));

    if (is->decoder_type == SOFT_DECODING)
    {
        MI_SYS_ChnPort_t stDivpChnPort;
        MI_DIVP_ChnAttr_t stDivpChnAttr;
        MI_DIVP_OutputPortAttr_t stOutputPortAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_SYS_ChnPort_t stDispChnPort;

        MI_GFX_Open();

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(is->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(is->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = is->pos_x;
        stInputPortAttr.stDispWin.u16Y      = is->pos_y;
        stInputPortAttr.stDispWin.u16Width  = is->out_width;
        stInputPortAttr.stDispWin.u16Height = is->out_height;

        memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
        stDivpChnAttr.bHorMirror            = FALSE;
        stDivpChnAttr.bVerMirror            = FALSE;
        stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
        stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
        stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
        stDivpChnAttr.stCropRect.u16X       = 0;
        stDivpChnAttr.stCropRect.u16Y       = 0;
        stDivpChnAttr.stCropRect.u16Width   = 0;
        stDivpChnAttr.stCropRect.u16Height  = 0;
        stDivpChnAttr.u32MaxWidth           = 1920;
        stDivpChnAttr.u32MaxHeight          = 1080;

        MI_DIVP_CreateChn(0, &stDivpChnAttr);
		MI_DIVP_SetChnAttr(0, &stDivpChnAttr);

        memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
        stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
        stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stOutputPortAttr.u32Width           = ALIGN_BACK(is->src_width , 32);
        stOutputPortAttr.u32Height          = ALIGN_BACK(is->src_height, 32);
        MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);
		MI_DIVP_StartChn(0);

        MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
        MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);
        MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = DISP_INPUTPORT;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = 0;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
        MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

        // 软解采用GFX旋转, 无须设置DISP
        stRotateConfig.eRotateMode = E_MI_DISP_ROTATE_NONE;
    }
    else
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(is->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(is->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = is->pos_x;
        stInputPortAttr.stDispWin.u16Y      = is->pos_y;
        stInputPortAttr.stDispWin.u16Width  = is->out_width;
        stInputPortAttr.stDispWin.u16Height = is->out_height;

        MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
        MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);
        MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);
        // 硬解时使用DISP旋转
        stRotateConfig.eRotateMode = is->display_mode;
    }

    MI_DISP_SetVideoLayerRotateMode(DISP_LAYER, &stRotateConfig);

    return MI_SUCCESS;
}

int my_display_unset(player_stat_t *is)
{
    if (is->decoder_type == SOFT_DECODING) {
        MI_SYS_ChnPort_t stDispChnPort;
        MI_SYS_ChnPort_t stDivpChnPort;

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = 0;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = 0;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = 0;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

        MI_DIVP_StopChn(0);
        MI_DIVP_DestroyChn(0);

        MI_GFX_Close();
    }

    //MI_DISP_ClearInputPortBuffer(DISP_LAYER, DISP_INPUTPORT, TRUE);
    //MI_DISP_HideInputPort(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
    return 0;
}


int my_video_init(player_stat_t *is)
{
    MI_DISP_InputPortAttr_t stInputPortAttr;
    
    MI_DIVP_OutputPortAttr_t stDivpOutAttr;
    MI_DIVP_ChnAttr_t stDivpChnAttr;

    MI_SYS_ChnPort_t stDispChnPort;
    MI_SYS_ChnPort_t stDivpChnPort;

    MI_GFX_Open();

    // 1.初始化DISP
    MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
    stInputPortAttr.u16SrcWidth         = ALIGN_BACK(is->src_width , 32);
    stInputPortAttr.u16SrcHeight        = ALIGN_BACK(is->src_height, 32);
    stInputPortAttr.stDispWin.u16X      = is->pos_x;
    stInputPortAttr.stDispWin.u16Y      = is->pos_y;
    stInputPortAttr.stDispWin.u16Width  = is->out_width;
    stInputPortAttr.stDispWin.u16Height = is->out_height;

    MI_DISP_DisableInputPort(0, 0);
    MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr);
    MI_DISP_EnableInputPort(0, 0);
    MI_DISP_SetInputPortSyncMode(0, 0, E_MI_DISP_SYNC_MODE_FREE_RUN);

    // 2.初始化DIVP模块
    memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = 0;
    stDivpChnAttr.stCropRect.u16Height  = 0;
    stDivpChnAttr.u32MaxWidth           = 1920;
    stDivpChnAttr.u32MaxHeight          = 1080;

    MI_DIVP_CreateChn(0, &stDivpChnAttr);
    MI_DIVP_StartChn(0);

    memset(&stDivpOutAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
    stDivpOutAttr.eCompMode             = E_MI_SYS_COMPRESS_MODE_NONE;
    stDivpOutAttr.ePixelFormat          = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stDivpOutAttr.u32Width              = ALIGN_BACK(is->src_width , 32);
    stDivpOutAttr.u32Height             = ALIGN_BACK(is->src_height, 32);

    MI_DIVP_SetOutputPortAttr(0, &stDivpOutAttr);

    // 3.绑定DIVP与DISP
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = 0;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = 0;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = 0;
    stDivpChnPort.u32PortId             = 0;
    
    MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
    MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

    return MI_SUCCESS;
}

int my_video_deinit(void)
{
    MI_SYS_ChnPort_t stDispChnPort;
    MI_SYS_ChnPort_t stDivpChnPort;

    // 解绑DIVP与DISP模块
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = 0;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = 0;

    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = 0;
    stDivpChnPort.u32PortId             = 0;
    
    MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

    MI_DIVP_StopChn(0);
    MI_DIVP_DestroyChn(0);

    MI_GFX_Close();

    return MI_SUCCESS;
}


