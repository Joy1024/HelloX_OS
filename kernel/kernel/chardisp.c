//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Nov,02 2014
//    Module Name               : CharDisplay
//    Module Funciton           : 
//                                
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "console.h"
#include "chardisplay.h"

#include "../arch/x86/biosvga.h"


//##################�ϰ汾�ַ���ʾ�ӿ�ʵ��,�Ѿ�ȫ�������½ӿ�ʵ��####################

void PrintStr(const char* pszMsg)
{
	CD_PrintString((LPSTR)pszMsg,FALSE);
}

void ClearScreen()
{
	CD_Clear();
}

void PrintCh(unsigned short ch)
{
	CD_PrintChar((CHAR)ch);
}

VOID CD_GotoHome()
{
#ifdef __I386__
	WORD CursorX = 0;
	WORD CursorY = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	CD_SetCursorPos(0,CursorY);
#endif 

#ifdef __CFG_SYS_CONSOLE
	Console.GotoHome();
#endif
}

VOID CD_GotoPrev()
{
#ifdef __I386__

	WORD CursorX = 0;
	WORD CursorY = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	if(CursorX > 0 )
		{
		CD_SetCursorPos(CursorX-1,CursorY);
		}

#endif

#ifdef __CFG_SYS_CONSOLE
	Console.GotoPrev();
#endif
}

void GotoHome()
{
	CD_GotoHome();
}
void ChangeLine()
{
	CD_ChangeLine();
}

VOID GotoPrev()
{
	CD_GotoPrev();
}

//###############�°��ַ���ʾ�ӿ�ʵ��######################
//��ʼ����ʾ�豸
VOID CD_InitDisplay(INT nDisplayMode)
{	
	return ;
}

//������ʾģʽ
VOID CD_SetDisplayMode(INT nMode)
{
	return ;
}

//�õ��к���
VOID CD_GetDisPlayRang(WORD* pLines,WORD* pColums)
{
#ifdef __I386__
	VGA_GetDisplayRange((INT*)pLines,(INT*)pColums);
#endif	
}

//�õ���ǰ���λ��
VOID  CD_GetCursorPos(WORD* pCursorX,WORD* pCursorY)
{
#ifdef __I386__
	VGA_GetCursorPos(pCursorX,pCursorY);
#endif 
}

//����
VOID CD_ChangeLine()
{
#ifdef __I386__
	VGA_ChangeLine();
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.GotoHome();
	Console.ChangeLine();
#endif

}
//���õ�ǰ���λ��
VOID  CD_SetCursorPos(WORD nCursorX,WORD nCursorY)
{
	#ifdef __I386__
		VGA_SetCursorPos(nCursorX,nCursorY);
	#endif
}

//��ӡ�ַ���,cl��ʾ�Ƿ���
VOID CD_PrintString(LPSTR pStr,BOOL cl)
{
#ifdef __I386__
	VGA_PrintString(pStr,FALSE);
	if(cl)
	{
		CD_GotoHome();
		CD_ChangeLine();
	}
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.PrintStr(pStr);
	if(cl == TRUE)
	{
		CD_ChangeLine();
	}
#endif	
}

/* Show a char on screen or terminal. */
VOID CD_PrintChar(CHAR ch)
{
#ifdef __I386__
		VGA_PrintChar(ch);
#endif

#ifdef __CFG_SYS_CONSOLE
		Console.PrintCh(ch);
#endif
}

//��ָ��λ�õõ��ַ���
VOID  CD_GetString(WORD nCursorX,WORD nCursorY,LPSTR pString,INT nBufLen)
{
#ifdef __I386__
	VGA_GetString(nCursorX,nCursorY,pString,nBufLen);	
#endif
}

//�õ�������Ļ�ַ���
VOID  CD_GetScreen(LPSTR pBuf,INT nBufLen)
{
#ifdef __I386__
	VGA_GetScreen(pBuf,nBufLen);	
#endif
}

//ɾ���ַ���
VOID  CD_DelString(WORD nCursorX,WORD nCursorY,INT nDelLen)
{
#ifdef __I386__
	VGA_DelString(nCursorX,nCursorY,nDelLen);
#endif
}

//ɾ���ַ�
VOID  CD_DelChar(INT nDelMode)
{
#ifdef __I386__
	VGA_DelChar(nDelMode);
#endif
}

//����
VOID  CD_Clear()
{
#ifdef __I386__
	VGA_Clear();
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.ClearScreen();
#endif
}

//���õ�ǰ�ַ���ʾ����
VOID  CD_SetCharAttr(BYTE cAttr)
{
	#ifdef __I386__
		VGA_SetCharAttr(cAttr);
	#endif
}
