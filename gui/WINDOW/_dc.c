//***********************************************************************/
//    Author                    : zhangbing(mail:550695@qq.com)
//    Original Date             : April,21 2009
//    Module Name               : dc.c
//    Module Funciton           : 
//                                The implementation of dc object.
//                                This is the kernel object in GUI module of
//                                Hello China.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#include "..\INCLUDE\KAPI.H"
#endif

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif

#ifndef __GLOBAL_H__
#include "..\INCLUDE\GLOBAL.H"
#endif

#ifndef __WNDMGR_H__
#include "..\INCLUDE\WNDMGR.H"
#endif

#include "..\include\gdi.h"

BOOL CheckWndStyle(HWND hWnd,DWORD dwWndStyle)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;

    return ((pwnd->dwWndStyle & dwWndStyle) == dwWndStyle);
}

BOOL IsVisible(HWND hWnd)
{
#if 0
    if(CheckStyle(HWND, WS_VISIBLE))  // now no WS_VISIBLE style!
        return TRUE;
    return FALSE;
#endif
}

static void InitDC(HWND hWnd,  PDC pDc, HRGN hrgnClip, DWORD dcxFlags, INT iDcType)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;

    pDc->iType = iDcType;
    /* ����DC��ʵ�����(Ҳ����ʱ�ڴ�)����ϵ */

    if(iDcType == DCT_SCREEN)
         pDc->hDevice = &Video;
    else
         pDc->hDevice =  NULL;
    pDc->hWnd = hWnd;
    pwnd->hDC = (HDC)pDc;
    pDc->dwFlags = dcxFlags;

    /* ��ʼ��DC��ȴʡֵ */
    pDc->iBkMode = OPAQUE;
    pDc->uTextAlign = TA_LEFT | TA_TOP | TA_NOUPDATECP;
    pDc->bkColor = RGB(255, 255, 255);  /* ȴʡʱ���ֵ�����ǰ�ɫ����ɫ*/
    pDc->textColor = RGB(0, 0, 0);      /* ȴʡʱ���ֵ�����Ǻ�ɫ����ɫ*/

#ifdef ROP
    pDc->iDrawMode = R2_COPYPEN;
#endif
    pDc->pt.x = 0;
    pDc->pt.y = 0;

#if 0

    pDc->pPen = (PPENOBJ)GetStockObject(BLACK_PEN);
    pDc->pBrush = (PBRUSHOBJ)GetStockObject(WHITE_BRUSH);
    pDc->pFont = (FONTOBJ *)GetStockObject(SYSTEM_FONT);


    pDc->hrgn = CreateEmptyRgn();
    if(!hrgn)
        return NULL;

    if(CalcWindowVisRgn(hWnd, hrgn, dcxFlags) == FALSE)
    {
        DeleteObject((HRGN)hrgn);
        pDc->pPen = NULL;
        pDc->pBrush = NULL;
        GdItemFree(pDc);
        return NULL;
    }

    /* �������hrgnClip��Ϊ�ղ��Ҵ��ڴ��ڿ���������Լ�����visrgn������/��ȥ���� */

    /* ִ���󽻲���,hrgnClip������һ��region������һ��HRGN_FULL��־ */
    if (hrgnClip != NULL && hrgnClip > HRGN_FULL)
    {
        if(dcxFlags & DCX_INTERSECTRGN)
        {
            IntersectRgn(hrgn, hrgn, hrgnClip);
        }
        /* ִ���ų����� */
        else if(dcxFlags & DCX_EXCLUDERGN)
        {
            SubtractRgn(hrgn, hrgn, hrgnClip);
        }
    }
#endif
}

/*==========================================================================
* HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
* ����:����ĳ�����ڵ�DC���ú����ṩ�˶Դ��ڿ�������ĸ��ӿ��ƣ����Ը��ݲ����ṩ
*      ��һЩ��־��Ϣ��������Ҫ�Ĳü����ơ�
* ����:
*      hWnd:     ���󴰿�DC�Ĵ��ھ����
*      hrgnClip  ָ�����ܺ��豸�������صĴ��ڿ�������������Ĳü���,���������
*                �����ڿ����������ų����hrgnClipҲ����������������к�hrgnClip
*                �Ľ�����
*      dcxFlags ָ���豸��������α�������һЩ��ǡ�
* ---------------------------------------------------------------------------
* DCX_WINDOW         ����DC�������ڵķǿͻ�����
* DCX_CLIPSIBLINGS   �ڴ���hWnd��Z��֮�ϵ����пɼ��ֵܴ��ڶ�Ҫ��DC�ļ��������ų�
*
* DCX_CLIPCHILREN    ���пɼ����Ӵ�������Ҫ��DC�ļ��������ų�
* DCX_EXCLUDERGN     �Ӵ��ڵĿ����������ų���hrgnclipָ��������(VisRgn - hrgnClip)
*
* DCX_INTERSECTRGN   ��hrgnclipָ���������봰�ڵĿ��������ཻ(VisRgn ^ hrgnClip)
*
* ����ֵ������ɹ�����ָ�����ڵ��豸��������,ʧ�ܷ���NULL�������hWndΪ����
*         �������DC.
*
* ��ʷ��¼:
* 2009/04/21  ����
===========================================================================*/
HDC GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD dcxFlags)
{
    HRGN  hrgn = NULL;
    PDC pDc = NULL;

    if(hWnd == NULL)
    {
#if 0 // unimplemented
        hWnd = GetDesktopWindow();
#endif
        if(hWnd == NULL)
            return NULL;
    }

    /* �����ڴ� */
    pDc = KMemAlloc(sizeof(DC), KMEM_SIZE_TYPE_ANY);
    if(!pDc)
        return NULL;


    /* �����жϸô����Ƿ����,�Ǿ߱���ʾ */
    if(!IsVisible(hWnd))
        return NULL;

        dcxFlags &= ~(DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN);

#if 0  // now in wndmgr.h has no WS_CLIPCHILDREN and WS_CLIPSIBLINGS Description
	if (CheckWndStyle(hWnd, WS_CLIPCHILDREN))
		dcxFlags |= DCX_CLIPCHILDREN;

	if (CheckWndStyle(hWnd, WS_CLIPSIBLINGS))
		dcxFlags |= DCX_CLIPSIBLINGS;

        /* ȡ�ͻ���DC */
        if (!(dcxFlags & DCX_WINDOW))
        {

            /*
            * ������С������Ҫ�ü��Ӵ���.
            */
            if (CheckWndStyle(hWnd, WS_MINIMIZE))
            {
                dcxFlags &= ~DCX_CLIPCHILDREN;

            }

        }
#endif
    InitDC(hWnd, pDc, hrgnClip, dcxFlags,  DCT_SCREEN);

    return (HANDLE)pDc;
}


HDC GetWindowDC(HWND hWnd)
{
    return GetDCEx(hWnd, NULL, DCX_WINDOW);
}


HDC GetDC(HWND hWnd)
{
    return GetDCEx(hWnd, NULL, 0);
}


HDC CreateCompatibleDC(HDC hdc)
{
    PDC pDc = (PDC)hdc;

    return (HANDLE)pDc;
}

int ReleaseDC(HWND hWnd, HDC hdc)
{
    __WINDOW *pwnd = (__WINDOW *)hWnd;
    PDC pDc = (PDC)hdc;
	
    /* ����ReleaseDC�ͷ��ڴ�DC����DeleteDc���ͷ� */
    if(!pDc || (pDc->iType == DCT_MEMORY))
        return 0;

#if 0 // not support now
    DeleteObject((HPEN)pDc->pPen);
    DeleteObject((HBRUSH)pDc->pBrush);
    DeleteObject((HBRUSH)pDc->pFont);
    DeleteObject((HRGN)pDc->hrgn);
    DeleteObject((HRGN)pDc->pBitmap);
#endif
    // process Device context of this window's field.
    pwnd->hDC = NULL;
    KMemFree(pDc, KMEM_SIZE_TYPE_ANY, sizeof(DC));
    return 1;
}


BOOL DeleteDC(HDC hdc)
{
    PDC pDc = (PDC)hdc;
    /* DeleteDCֻ�����ͷ��ڴ�DC */
    if(!pDc || !(pDc->iType & DCT_MEMORY))
        return FALSE;

    /* �ͷ�Ӳ��������ڴ� */

    pDc->iType  = DCT_SCREEN;
    return ReleaseDC(NULL, hdc);
}


