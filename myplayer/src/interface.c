#include "audiostream.h"
#include "videostream.h"
#include "demux.h"
#include "player.h"
#include "interface.h"

#define LOCAL_X             0
#define LOCAL_Y             0
#define MIN_WIDTH           128
#define MIN_HEIGHT          64
#define MAX_WIDTH           1920
#define MAX_HEIGHT          1080

/*
Note: 调用以下接口需要加上互斥锁
*/

static pthread_mutex_t myplayer_mutex = PTHREAD_MUTEX_INITIALIZER;
static player_stat_t *ssplayer = NULL;
static bool g_mute = false;
static int g_rotate = E_MI_DISP_ROTATE_NONE;

player_opts_t g_opts = {0, 0, AV_ONCE, 0, 0, 0, 0};

static void * my_layer_handler(void * arg)
{
    int ret;
    player_stat_t * mplayer = (player_stat_t *)arg;

    ret = open_demux(mplayer);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "open_demux failed!\n");
        return NULL;
    }
    av_log(NULL, AV_LOG_INFO, "open_demux successful\n");

    ret = open_video(mplayer);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "open_video failed!\n");
        mplayer->audio_idx = -1;
        return NULL;
    }
    av_log(NULL, AV_LOG_INFO, "open_video successful\n");

    ret = open_audio(mplayer);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "open_audio failed!\n");
        return NULL;
    }
    av_log(NULL, AV_LOG_INFO, "open_audio successful\n");

    return mplayer;
}

int my_player_open(const char *fp, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    if (ssplayer || width <= 0 || height <= 0) {
        av_log(NULL, AV_LOG_ERROR, "input parameter check error!\n");
        return -1;
    }

    av_log(NULL, AV_LOG_INFO, "### enter my_player_open\n");

    pthread_mutex_init(&myplayer_mutex, NULL);

    if (width < MIN_WIDTH || width > MAX_WIDTH)
    {
        printf("\033[33;2mset width must be in [%d, %d]\033[0m\n", MIN_WIDTH, MAX_HEIGHT);
        if (width < MIN_WIDTH)
            width = MIN_WIDTH;
        else if (width > MAX_WIDTH)
            width = MAX_WIDTH;
    }
    if (height < MIN_HEIGHT || height > MAX_HEIGHT)
    {
        printf("\033[33;2mset height must be in [%d, %d]\033[0m\n", MIN_HEIGHT, MAX_HEIGHT);
        if (height < MIN_HEIGHT)
            height = MIN_HEIGHT;
        else if (height > MAX_HEIGHT)
            height = MAX_HEIGHT;
    }

    ssplayer = player_init(fp);
    if (ssplayer == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "player init failed[%s]\n",fp);
        return -1;
    }

    gettimeofday(&ssplayer->tim_open, NULL);

    memcpy(&ssplayer->opts, &g_opts, sizeof(player_opts_t));
    ssplayer->display_mode = g_rotate;

    //如果没有提前设置windows大小,默认使用传入的参数
    if (g_opts.w < 8 || g_opts.h < 8) {
        ssplayer->in_height = height;
        ssplayer->in_width  = width;
        ssplayer->pos_x = x;
        ssplayer->pos_y = y;
    } else {
        ssplayer->in_height = g_opts.h;
        ssplayer->in_width  = g_opts.w;
        ssplayer->pos_x = g_opts.x;
        ssplayer->pos_y = g_opts.y;
    }

    av_log(NULL, AV_LOG_INFO, "set out width/height = [%d %d]\n", ssplayer->in_width, ssplayer->in_height);

    if(my_layer_handler(ssplayer) == NULL)
    {
        int ret = (ssplayer->play_status < 0) ? ssplayer->play_status : -1;
        my_player_close();
        return ret;
    }
    av_log(NULL, AV_LOG_INFO, "leave my_player_open ###\n");

    return 0;
}

int my_player_close(void)
{
    if(ssplayer == NULL) {
        av_log(NULL, AV_LOG_WARNING, "myplayer has been closed!\n");
        return -1;
    }

    int ret;

    av_log(NULL, AV_LOG_INFO, "### enter my_player_close\n");

    pthread_mutex_lock(&myplayer_mutex);

    ret = player_deinit(ssplayer);

    memset(&g_opts, 0x0, sizeof(player_opts_t));

    ssplayer = NULL;
    g_mute = false;

    pthread_mutex_unlock(&myplayer_mutex);
    pthread_mutex_destroy(&myplayer_mutex);

    av_log(NULL, AV_LOG_INFO, "leave my_player_close ###\n");

    return ret;
}

int my_player_pause(void)
{
    pthread_mutex_lock(&myplayer_mutex);

    if (ssplayer == NULL || ssplayer->paused) {
        NANOX_ERR("sstar_player_pause failed\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    NANOX_MARK("sstar_player_pause!\n");
    toggle_pause(ssplayer);

    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_resume(void)
{
    pthread_mutex_lock(&myplayer_mutex);

    if(ssplayer == NULL || !ssplayer->paused) {
        NANOX_ERR("sstar_player_resume failed\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    NANOX_MARK("sstar_player_resume!\n");
    toggle_pause(ssplayer);

    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_getposition(double *position)
{
    pthread_mutex_lock(&myplayer_mutex);

    if(ssplayer == NULL) {
        NANOX_ERR("sstar_player_getposition failed\n");
        *position = NAN;
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    *position = get_master_clock(ssplayer);

    if (isnan(*position)) {
        NANOX_MARK("get invalid position time\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    } else {
        double start_time = ssplayer->p_fmt_ctx->start_time * av_q2d(AV_TIME_BASE_Q);
        *position = (isnan(start_time)) ? *position : (*position - start_time);
    }
    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_getduration(double *duration)
{
    pthread_mutex_lock(&myplayer_mutex);

    if(ssplayer == NULL) {
        NANOX_ERR("sstar_player_getduration failed\n");
        *duration = NAN;
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    if (ssplayer->p_fmt_ctx->duration != AV_NOPTS_VALUE) {
        *duration = ssplayer->p_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q);
        NANOX_MARK("get file duration time [%.3lf]\n", *duration); 
    } else {
        NANOX_MARK("get invalid duration time\n"); 
        *duration = NAN;
    }

    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_seek(double time)
{
    double pos = 0.0f;

    pthread_mutex_lock(&myplayer_mutex);

    if(ssplayer == NULL) {
        NANOX_ERR("sstar_player_seek failed\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    if (ssplayer->seek_by_bytes) {
        av_log(NULL, AV_LOG_WARNING, "this file don't support to seek!\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    pos = get_master_clock(ssplayer);
    if (isnan(pos))
        pos = (double)ssplayer->seek_pos / AV_TIME_BASE;
    pos += time;
    if (ssplayer->p_fmt_ctx->start_time != AV_NOPTS_VALUE && pos <= ssplayer->p_fmt_ctx->start_time / (double)AV_TIME_BASE)
        pos = ssplayer->p_fmt_ctx->start_time / (double)AV_TIME_BASE;
    if (ssplayer->p_fmt_ctx->duration != AV_NOPTS_VALUE && pos >= ssplayer->p_fmt_ctx->duration / (double)AV_TIME_BASE)
        pos = ssplayer->p_fmt_ctx->duration / (double)AV_TIME_BASE;
    NANOX_MARK("start to seek to %.3f\n", pos);
    stream_seek(ssplayer, (int64_t)(pos * AV_TIME_BASE), (int64_t)(time * AV_TIME_BASE), ssplayer->seek_by_bytes);

    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_seek2time(double time)
{
    double pos = 0.0f, diff = 0.0f, target = 0.0f;

    pthread_mutex_lock(&myplayer_mutex);

    if(ssplayer == NULL) {
        NANOX_ERR("sstar_player_seek2time failed\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    if (ssplayer->seek_by_bytes) {
        av_log(NULL, AV_LOG_WARNING, "this file don't support to seek!\n");
        pthread_mutex_unlock(&myplayer_mutex);
        return -1;
    }

    pos = get_master_clock(ssplayer);
    if (isnan(pos))
    {
        pos = (double)ssplayer->seek_pos / AV_TIME_BASE;
    }
    diff = time - pos;
    target = time;
    if (ssplayer->p_fmt_ctx->start_time != AV_NOPTS_VALUE && time <= ssplayer->p_fmt_ctx->start_time / (double)AV_TIME_BASE)
        target = ssplayer->p_fmt_ctx->start_time / (double)AV_TIME_BASE;
    if (ssplayer->p_fmt_ctx->duration != AV_NOPTS_VALUE && time >= ssplayer->p_fmt_ctx->duration / (double)AV_TIME_BASE)
        target= ssplayer->p_fmt_ctx->duration / (double)AV_TIME_BASE;
    NANOX_MARK("start to seek2 to %.3f\n", target);
    stream_seek(ssplayer, (int64_t)(target * AV_TIME_BASE), (int64_t)(diff * AV_TIME_BASE), ssplayer->seek_by_bytes);

    pthread_mutex_unlock(&myplayer_mutex);

    return 0;
}

int my_player_set_chlayout(int soundmode)
{
    if (soundmode <= 0 || soundmode > 2) {
        NANOX_ERR("invalid sound mode setting!\n");
        g_audio_chlayout = AV_CH_LAYOUT_MONO;
        return -1;
    }

    if (soundmode >= 2) {
        g_audio_chlayout = AV_CH_LAYOUT_STEREO;
    } else {
        g_audio_chlayout = AV_CH_LAYOUT_MONO;
    }

    return 0;
}

int my_player_set_aodev(int dev)
{
    if (dev < 0 || dev > 3) {
        NANOX_ERR("invalid audio device setting!\n");
        gplayer_AoDevId = 0;
        return -1;
    }

    gplayer_AoDevId = dev;
    NANOX_LOG("my_player_set_aodev[%d]\n", gplayer_AoDevId);

    return 0;
}

int my_player_set_rotate(int rotate)
{
    if (rotate < 0 || rotate > 4) {
        NANOX_ERR("my_player_set_rotate failed!\n");
        return -1;
    }

    g_rotate = rotate;
    NANOX_LOG("my_player_set_rotate[%d]\n", g_rotate);

    return 0;
}

int my_player_set_volumn(int volumn)
{
    if (ssplayer == NULL) {
        NANOX_ERR("my_player_set_volumn failed!\n");
        return -1;
    }

    if (ssplayer->audio_idx >= 0) {
        MI_S32 vol;
        MI_AO_ChnState_t stAoState;

        if (volumn > 0) {
            //vol = volumn * (MAX_ADJUST_AO_VOLUME - MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
            //vol = (vol > MAX_ADJUST_AO_VOLUME) ? MAX_ADJUST_AO_VOLUME : vol;
            //vol = (vol < MIN_ADJUST_AO_VOLUME) ? MIN_ADJUST_AO_VOLUME : vol;
            vol = (int)(log10(volumn * 1.0) * 45 - 60);
            vol = (vol > 30) ? 30 : vol;
            g_mute = false;
        } else {
            vol = MIN_AO_VOLUME;
            g_mute = true;
        }

        memset(&stAoState, 0, sizeof(MI_AO_ChnState_t));
        if (MI_SUCCESS == MI_AO_QueryChnStat(AUDIO_DEV, AUDIO_CHN, &stAoState))
        {
            MI_AO_SetVolume(AUDIO_DEV, vol);
            MI_AO_SetMute(AUDIO_DEV, g_mute);
        }
    }

    return 0;
}

int my_player_set_mute(bool mute)
{
    if (ssplayer == NULL) {
        NANOX_ERR("my_player_set_mute failed!\n");
        return -1;
    }

    if (ssplayer->audio_idx >= 0) {
        g_mute = mute;
        MI_AO_SetMute(AUDIO_DEV, mute);
    }

    return 0;
}

int my_player_set_window(int x, int y, int width, int height)
{
    if (width < 8 || height < 8) {
        NANOX_ERR("set window param is invalid!\n");
        return -1;
    }

    if (ssplayer == NULL) {
        g_opts.x = x;
        g_opts.y = y;
        g_opts.w = width;
        g_opts.h = height;
    } else {
#if USE_DIVP_MODULE
        ssplayer->src_width  = width;
        ssplayer->src_height = height;
        ssplayer->out_width  = width;
        ssplayer->out_height = height;
        ssplayer->pos_x = x;
        ssplayer->pos_y = y;

        MI_DIVP_OutputPortAttr_t stOutputPortAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;

        MI_DIVP_GetOutputPortAttr(0, &stOutputPortAttr);
        stOutputPortAttr.u32Width  = ALIGN_BACK(ssplayer->src_width , 32);
        stOutputPortAttr.u32Height = ALIGN_BACK(ssplayer->src_height, 32);
        MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);

        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_BACK(ssplayer->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_BACK(ssplayer->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = ssplayer->pos_x;
        stInputPortAttr.stDispWin.u16Y      = ssplayer->pos_y;
        stInputPortAttr.stDispWin.u16Width  = ssplayer->out_width;
        stInputPortAttr.stDispWin.u16Height = ssplayer->out_height;
        MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
#else
        NANOX_ERR("divp module is not enable!\n");
#endif
    }
    NANOX_LOG("my_player_set_window =  [%d %d %d %d]\n", x, y, width, height);

    return 0;
}


