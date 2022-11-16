//***********************************************************************/
//    Author                    : zhangbing(mail:550695@qq.com)
//    Original Date             : Mar 28,2009
//    Module Name               : gdi.h
//    Module Funciton           : 
//                                Declares gdi object and related structures,
//                                constants and global routines.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GDI_H__
#define __GDI_H__

typedef DWORD   COLORREF;
typedef HANDLE  HDC; 
typedef HANDLE  HRGN;
typedef HANDLE HWND;

typedef struct tagRECT
{
    INT    left;
    INT    top;
    INT    right;
    INT    bottom;
} RECT, *PRECT;

typedef struct tagPOINT
{
    INT  x;
    INT  y;
} POINT, *PPOINT;


#define DCT_SCREEN    0x00000001L
#define DCT_PRINT      0x00000002L
#define DCT_MEMORY   0x00000004L

/*
* DC.iBkMode ����
*/
#define TRANSPARENT  1  /* ����������ʱ,��������͸���ı�����ʽ������DC���趨�ı�����ɫ */
#define OPAQUE       2  /* ��͸��������ɫ��� */



/*
 * �豸���� GetDCEx�����в���dcx_flags����
 */
#define DCX_WINDOW              0x00000001L
#define DCX_CLIPCHILDREN        0x00000002L
#define DCX_CLIPSIBLINGS        0x00000004L
#define DCX_EXCLUDERGN          0x00000008L
#define DCX_INTERSECTRGN        0x00000010L


/*
* DC.uTextAlign ���ֶ��뷽ʽ(��־)
*/
#define TA_NOUPDATECP   0
#define TA_UPDATECP     1

#define TA_LEFT         0
#define TA_RIGHT        2
#define TA_CENTER       6

#define TA_TOP          0
#define TA_BOTTOM       8
#define TA_BASELINE     24
#define TA_RTLREADING   256
#define TA_MASK         (TA_BASELINE+TA_CENTER+TA_UPDATECP+TA_RTLREADING)


/* ����߼����� */
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH

#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8


typedef struct tagDC {
    INT         iType;      /* the DC's type, screen DC | memory DC | print DC*/
    HANDLE   hDevice;       /* ��������豸��� */
    HWND     hWnd;          /* DC�����Ĵ��� */

    DWORD    dwFlags;       /* �����豸�ü���� */

	
    INT      iBkMode;       /* ����ģʽ */
    UINT     uTextAlign;    /* ���ֶ��뷽ʽ */
    COLORREF bkColor;       /* �������ʱ�ı�����ɫ */
    COLORREF textColor;     /* ���ֵ���ɫ */
    INT      iDrawMode;     /* rop2 ���ǻ�ͼģʽ */
    POINT    pt;            /* ��ǰ�Ļ�������*/


    HANDLE   pPen;          /* ��ǰ�Ļ��ʾ�� */
    HANDLE   pBrush;        /* ��ǰ�Ļ�ˢ��� */
    HANDLE   pFont;         /* ��ǰ�������� */
    HANDLE   pBitmap;       /* ��ǰ��λͼ */
    HRGN     hrgn;          /* �û�ָ����DC�ü��� */
}DC, *PDC;




#endif


