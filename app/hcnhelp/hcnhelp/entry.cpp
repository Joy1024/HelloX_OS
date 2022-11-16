#include <kapi.h>

//Window procedure of the main window.
static DWORD HCNHelpWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         static HANDLE hDC = GetClientDC(hWnd);
          __RECT rect;
		  int   ypos = 5;
 
         switch(message)
         {
         case WM_CREATE:    //Will receive this message when the window is created.
             break;
         case WM_TIMER:     //Only one timer can be set for one window in current version.
             break;
         case WM_DRAW:
               //GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
			   TextOut(hDC,10,ypos,"Hello China��һ������Ƕ��ʽ����ϵͳ��Ӧ���ڸ������ܿ����豸�С�");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"��ǰ�汾��V1.75��������һЩ��صĲ���������");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"------------------------------------------");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"��ǰģʽ��ͼ��ģʽ������CTRL + ALT + DEL��ϼ����ɷ����ַ����棻");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"���ַ�ģʽ�£�����GUI�����س����ɽ���ͼ��ģʽ��");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"���ַ�ģʽ�£�����help�����س���������ַ�ģʽ�İ�����Ϣ��");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"�κμ������⣬�����QQȺ��38467832 �������ۣ�");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"�ٷ�blog����:http://blog.csdn.net/hellochina15");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"����֧���ʼ���ַ:garryxin@yahoo.com.cn");
			   ypos += 20;
             break;
         case WM_CLOSE:
             PostQuitMessage(0);  //Exit the application.
             break;
         default:
             break;
         }
         return DefWindowProc(hWnd,message,wParam,lParam);
}
 
//Entry point of Hello World.
extern "C"
{
DWORD HCNMain(LPVOID pData)
{
         MSG msg;
         HANDLE hMainFrame = NULL;
         __WINDOW_MESSAGE wmsg;
 
         //Create hello world's window.
         hMainFrame = CreateWindow(WS_WITHBORDER | WS_WITHCAPTION,
                   "Help information for Hello China V1.75",
                   150,
                   150,
                   600,
                   400,
                   HCNHelpWndProc,  //Window procedure.
                   NULL,
                   NULL,
                   0x00FFFFFF,    //Window's background color.
                   NULL);
         if(NULL == hMainFrame)
         {
                   MessageBox(NULL,"Can not create the main frame window.","Error",MB_OK);
                   goto __TERMINAL;
         }
 
         //Message loop of this application.
         while(TRUE)
         {
                   if(GetMessage(&msg))
                   {
                            switch(msg.wCommand)
                            {
                            case KERNEL_MESSAGE_TIMER:
                                     wmsg.hWnd = (HANDLE)msg.dwParam;
                                     wmsg.message = WM_TIMER;
                                     SendWindowMessage(wmsg.hWnd,&wmsg);
                                     break;
                            case KERNEL_MESSAGE_WINDOW:
                                     DispatchWindowMessage((__WINDOW_MESSAGE*)msg.dwParam);
                                     break;
                            case KERNEL_MESSAGE_TERMINAL:  //Post by PostQuitMessage.
                                     goto __TERMINAL;
                            default:
                                     break;
                            }
                   }
         }
 
__TERMINAL:
         if(hMainFrame)
         {
                   DestroyWindow(hMainFrame);
         }
         return 0;
}
}
