#ifndef __COAPI_PLAYER_H_
#define __COAPI_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "coapi_api/coapi_media.h"

int dev_player_task_init(void);
int dev_player_task_deinit(void);
void dev_player_start(char *toneurl, char *audiourl);
/*
* 播放本地音乐 
*/
void dev_player_local_start(char *play_file);

/* 播放器是否正在播放 */
int dev_player_is_play(void);
/******************************************************************/
/********************** USED BY COAPI *****************************/
/****************** need to be compeleted *************************/

/**
 * @brief 控制播放器停止
 */
void dev_player_stop(void);

/**
 * @brief 控制播放器下一曲
 */
void dev_player_next(void);

/**
 * @brief 控制播放器上一曲
 */
void dev_player_prev(void);

/**
 * @brief 控制播放器暂停
 */
void dev_player_pause(void);
/**
 * @brief 控制播放器恢复
 */
void dev_player_resume(void);

/**
 * @brief　设置音量大小
 * @volume 0——100　被设置的音量大小
 */
void dev_set_volume(int volume);

/**
 * @brief　获取音量大小
 * @return
 *      success: 0——100　音量大小
 *      fail : -1
 */
int dev_get_volume(void);

/**
 * @brief　音量加
 */
void dev_volume_up(void);

/**
 * @brief　音量减
 */
void dev_volume_down(void);


/**
 * @brief 判断当前播放的音乐类型　
 * @return
 *       0: 播放的是在线音频
 *       1: 播放的是本地音频
 */
int  dev_play_local_music(void);

/**
 * @brief 获取媒体状态
 * @media 参考coapi_api/coapi_meida.h media_item的定义
 * @return
 *      success: 0
 *      fail : -1
 */
int  dev_media_status(media_info *media);

/**
 * @brief 获取本地音频的数量
 * @return
 *      n: 本地音频数量
 *      -1: fail
 */
int  dev_get_song_num(void);

/******************************************************************/
/*************************** END **********************************/
/******************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COAPI_PLAYER_H_ */
