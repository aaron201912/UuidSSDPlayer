//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file MSrv_Airplay_Player.h
/// @warning This should not be called by the upper layer
/// @brief \b Introduction: AV Player Description
///
/// @author MStar Semiconductor Inc.
///
/// Features:
/// - Support the Init /Fialize for AV
/// - Support the get /set active player number for AV
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef MSRV_AIRPLAY_PLAYER_H
#define MSRV_AIRPLAY_PLAYER_H
//#include "ZM_IShare.h"
//----------------------------------------------------
#define ZYFLAY_VERSION             11040005
#define ZYCAST_VERSION             20190315

#define AIRPLAY_TIMEOUT             32
#if( EXPORT_THIRDPART_INTERFACE == 1)
#define MUTLI_WITH_DLNA            0
#else
#define MUTLI_WITH_DLNA            1
#endif
//----------------------------------------------------
#define DPLPAYER_INIT              0
#define DPLPAYER_RESRART           1
#define DPLPAYER_PLAY              2
#define DPLPAYER_RESETVED          3
#define DPLPAYER_RESETAUD          4
#define DPLPAYER_RESETALL          5
#define DPLPAYER_STOP              6
#define DPLPAYER_FINISH            7
#define DPLPAYER_AUDCFG            8
#define DPLPAYER_FLUSHALL          9
#define DPLPAYER_INIT2             20
#define DPLPAYER_STOP2             21
#define DPLPAYER_INIT3             22
//----------------------------------------------------
#define CASTMODE_MARK             0x0F
#define CASTMODE_FULL_720P        0x00
#define CASTMODE_FULL_480P        0x02
#define CASTMODE_MIRR             0x01

#define CASTOPTION_MARK           0xF0
#define CASTOPTION_ICAST          0x10
#define MIRROPTION_480P           0x20
#define MIRROPTION_576P           0x40
//----------------------------------------------------
extern "C"
{
	void mHW_AESCRT128_Init(unsigned char *key, unsigned char *iv);
	int  mHW_AESCRT128_Encrypt(unsigned char *pbuf,int nlen);
	void mHW_AESCBC128_Init(unsigned char *key, unsigned char *iv);
	int  mHW_AESCBC128_Encrypt(unsigned char *pbuf,int nlen);

};

typedef int (*fmHW_AESInit) (int nCmd,void *prama);//the callback
typedef int (*fmHW_AESEncrypt) (int nCmd,void *prama);//the callback
typedef struct 
{
  fmHW_AESInit 		pfHW_AESCRT128_Init;
  fmHW_AESEncrypt	pfHW_AESCRT128_Encrypt;
  fmHW_AESInit 		pfHW_AESCBC128_Init;
  fmHW_AESEncrypt	pfHW_AESCBC128_Encrypt;
} ExtraAirplayPara; 
//----------------------------------------------------
extern int gVedioWidth;
extern int gVedioHeight;
extern int gnDLNAStartFlag;
//----------------------------------------------------
int  ZyDLNAServiceStart(char * friendly_name,int nflag = 0);
int ZyDLNAServiceClose(void);
int AirplayServiceStart(char * pickname);
int AirplayServiceMode(int nMode);
int AirplayServiceClose(void);
unsigned char AirplayPlayer_GetStatus();
//FlyPlayerType GetFlyPlayerType();
bool IsZMFlyPlaying();
int  GetZMFlyVersion();
/*
	use for gst player ex:dlna url play and stop
	scene default is false
	mGstPlayerPlay("xxxx.mp4",false);
*/
void mGstPlayerPlay(char * url, bool bAirplayMode,float fPosition= 0.0f, char *buf = NULL );
void mGstPlayerStop();
void mGstPlayerSetStatus(unsigned char status);
void mGstPlayerPause();
void mGstPlayerResume();
void mGstPlayerSeek(int seekto);
unsigned char  mGstPlayerGetStatus();
void GstMmClose();
void SysDrop_caches();
void GstLowMemPatch();
int ZM_GetMemFree();
#endif // MSRV_AIRPLAY_PLAYER_H
