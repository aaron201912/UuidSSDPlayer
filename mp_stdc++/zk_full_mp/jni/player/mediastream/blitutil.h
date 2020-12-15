#ifndef __BLITUTIL_H__
#define __BLITUTIL_H__

#ifdef __cplusplus
extern "C" {               // 告诉编译器下列代码要以C链接约定的模式进行链接
#endif

#ifdef SUPPORT_PLAYER_MODULE
#include "mi_gfx_datatype.h"

#define RECTW(r) (r.right-r.left)
#define RECTH(r) (r.bottom-r.top)

typedef struct
{
    int top;
    int bottom;
    int left;
    int right;
}RECT_t;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
}Rect_t;

typedef struct
{
    int w;
    int h;
    int pitch;
    int BytesPerPixel;
    MI_GFX_ColorFmt_e eGFXcolorFmt;
    unsigned long long phy_addr;
}Surface_t;


void SstarBlitCCW(Surface_t * pSrcSurface, Surface_t *pDstSurface, RECT_t* pRect);
void SstarBlitCW(Surface_t * pSrcSurface, Surface_t *pDstSurface, RECT_t* pRect);
void SstarBlitHVFlip(Surface_t * pSrcSurface, Surface_t *pDstSurface, RECT_t* pRect);
void SstarBlitNormal(Surface_t * pSrcSurface, Surface_t *pDstSurface, RECT_t* pRect);

#endif

#ifdef __cplusplus
}
#endif
#endif
