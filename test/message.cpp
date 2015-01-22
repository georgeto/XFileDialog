// for debuging Windows Message Only
// 1. launch the helloworld.class and click the "thumbnail (single-selection)"
//
// 2. Use this utility to send some WM_COMMAND to the dialog
//
// 3. Check its response
//
//

#include <stdio.h>
#include <afx.h>
#include <afxdlgs.h>
#include <comdef.h>

#include "mytrace.h"

#define XLVM_XP_DO_NOT_PERSIST			(-1)
#define XLVM_XP_UNDEFINED				0
#define XLVM_XP_ICONS					0x7029
#define XLVM_XP_LIST					0x702B
#define XLVM_XP_DETAILS					0x702C
#define XLVM_XP_THUMBNAILS				0x702D
#define XLVM_XP_TILES					0x702E

#define XLVM_VISTA_DO_NOT_PERSIST		(-1)
#define XLVM_VISTA_UNDEFINED			0
#define XLVM_VISTA_DETAILS				0x704B
#define XLVM_VISTA_TILES				0x704C
#define XLVM_VISTA_EXTRA_LARGE_ICONS	0x704D
#define XLVM_VISTA_MEDIUM_ICONS			0x704E
#define XLVM_VISTA_LARGE_ICONS			0x704F
#define XLVM_VISTA_SMALL_ICONS			0x7050
#define XLVM_VISTA_LIST					0x7051




HWND lst2Handle;  // lst2=0x461 in Xp/Vista

BOOL   CALLBACK   wndProc(HWND   hWnd,   LPARAM   lParam) {
    printf("%ld\n",   hWnd);
    printf("controlID: 0x%x\n", GetDlgCtrlID(hWnd) );
    int temp = GetDlgCtrlID(hWnd);
    if(temp == 0x461) lst2Handle=hWnd;

    return true;
}

int main() {
    printf("hello xfiledialog\n");
    CWnd* pshell;


//      Get the dialog window handle
//      search according to both classname (#32770 dalog) and title
//
    HWND hWnd1=  FindWindow("#32770", "Open Thumbnail Dialog (single-selection)" );

    // Check its parent (the JFrame)
    HWND parent1 = GetParent(hWnd1);
    printf("parent jframe is: 0x%x\n", parent1);

    // Check whether it's the active window
    HWND active = GetForegroundWindow();
    printf("the foregroundwindow: 0x%x\n", active);

    if(hWnd1!= 0 ) {
        printf("Window handle is found 0x%x\n" ,hWnd1);
        lst2Handle=(HWND)0;

        // Enum child windows to get the shellview (lst2)
        CWnd   *pWnd   =   CWnd::FromHandle(hWnd1);
        if(pWnd != NULL) {
            EnumChildWindows(pWnd->GetSafeHwnd(),   (WNDENUMPROC)wndProc,   NULL);
        }

    }

    printf("lst2Handle: 0x%x\n", lst2Handle);

    HWND hWnd= lst2Handle;
    pshell = CWnd::FromHandle(hWnd);

    if (pshell) {
        printf("send a message to this window\n");
        pshell->SendMessage(WM_COMMAND, XLVM_VISTA_LARGE_ICONS);

    } else
        printf("The window can not be found\n");

}


