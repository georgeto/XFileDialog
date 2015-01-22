/////////////////////////////////////////////////////////////////////////
// A dialog for Selecting image files
// XPThumbnailDialog.cpp :
// The implementation file
//
// This class works in both XP and Vista/Win7
// The IFileDialog in Vista/Win7 could not work now, this is the
// only practical solution for Vista/Win7
//
// This class use lst2 (defined as 0x461 in Dlgs.h of Windows SDK 5/6)
// to find the real shellview control in the dialog
//
// First of all, in all OSes, the FileDialog window is not
// equal to the CFileDialog. E.g., in XP, the FileDialog
// window is the parent of CFileDialog.
//
// In XP, lst2 is a direct child of the FileDialog window.
//
// In Vista, lst2 is not a direct child of the FileDialog window
// (after several layers, The EnumWindow function is used to get it)
//
// In Win7 32bit, things are even worse,
// The FileDialog's handle must be determined by the current active
// window or findWindow.
//
// In Win7 64bit, things are similar to Vista (Win 6)
//
//

#include <afxwin.h>
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxcmn.h>  	    // MFC support for Windows 95 Common Controls

#include <afxdlgs.h>
#include <comdef.h>
#include <stdio.h>
#include <dlgs.h> // for (MULTI)FILEOPENORD
#include "XPThumbnailDialog.h"

extern int traceLevel;
extern TCHAR tracebuffer[1000];
extern void trace();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



HWND lst2Handle1;  // lst2=0x461 in Xp/Vista

BOOL   CALLBACK   wndProc1(HWND   hWnd,   LPARAM   lParam) {
    int temp = GetDlgCtrlID(hWnd);
    if(temp == 0x461) lst2Handle1=hWnd;

    return true;
}


const int MYWM_POSTINIT = WM_USER+1;

IMPLEMENT_DYNAMIC(XPThumbnailDialog, CFileDialog)

BEGIN_MESSAGE_MAP(XPThumbnailDialog, CFileDialog)
    //{{AFX_MSG_MAP(XPThumbnailDialog)
    ON_WM_CREATE()
    ON_MESSAGE(MYWM_POSTINIT, OnPostInit)
END_MESSAGE_MAP()


XPThumbnailDialog::XPThumbnailDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pWnd) :
    CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, OFN_HIDEREADONLY|OFN_EXPLORER|dwFlags, lpszFilter, pWnd) {

    m_bExplorer = FALSE;
    //}}AFX_DATA_INIT

    if (dwFlags & OFN_EXPLORER) {
        m_bExplorer = TRUE;
    } else if (m_ofn.Flags & OFN_EXPLORER) {
        // MFC added it, but we don't want it
        m_ofn.Flags &= ~(OFN_EXPLORER | OFN_SHOWHELP);
    }
    swprintf_s(tracebuffer, L"%s", L"DLL>>: XPThumbnailDialog is initialized\n");
    trace();
}

BOOL XPThumbnailDialog::SetListViewMode(UINT cmd) {
    if (!m_bExplorer) { // only supported for Explorer Dialog
        swprintf_s(tracebuffer, L"DLL>>:: ONLY_EXPLORER supported \n");
        trace();

        return FALSE;
    }

    // this function can be called in OnFolderChange
    // in XP

    // lst2 is the  SHELLDLL_DefView
    // We need to  send message to lst2 (container of the listview)
    // to change its view mode
    //
    // Using SendDlgItemMessage  is another option
    //

    swprintf_s(tracebuffer, L"The CFileDialog 's hWnd is: 0x%x\n", m_hWnd);
    trace();


    // The following method is a combination of the current active Window
    // and findWindow function.
    //
    // After some testing, I think that it's more reliable than
    // the old GetParent() function in XP.
    //
    // This method works in XP, Vista, Win7.
    //
    // It seems that Microsoft is unlikely to break it, otherwise, a lot of
    // XP apps will be broken too.
    //

    CWnd* active = GetForegroundWindow();
    CWnd *pWnd= FindWindow(_T("#32770"), m_ofn.lpstrTitle);

    if(pWnd != NULL && active != NULL) {
        swprintf_s(tracebuffer, L"DLL>>: The Filedialog handle: 0x%x\n", pWnd->m_hWnd);
        trace();
        swprintf_s(tracebuffer, L"DLL>>: The Active Foreground Window handle: 0x%x\n", active->m_hWnd);
        trace();

        // In normal cases, the two handle are same because
        // the modal filedialog should be the active window
        //
        CWnd* parent= active;

        // checking it further
        // #32770 is the Dialog class name defined in MFC
        if(true) {
            TCHAR szClassName[255];
            if(::GetClassName(m_hWnd, szClassName, 255)) {
                swprintf_s(tracebuffer, L"DLL>>: Active window's classname: %s\n", szClassName);
                trace();
                CString s1(_T("#32770"));
                CString s2(szClassName);
                if(s1.Compare(s2)!= 0) {
                    swprintf_s(tracebuffer, L"DLL>>: Warning: The active window is not a dialog\n");
                    trace();

                    parent= pWnd; // use another handle instead
                }

            }
        }

        swprintf_s(tracebuffer, L"DLL>>: Handle of the real FileDialog : 0x%x\n", parent->m_hWnd);
        trace();

        CWnd* pshell= parent->GetDlgItem(lst2);
        // XP
        if (pshell) {
            swprintf_s(tracebuffer, L"DLL>>: Shellview lst2 in XP: 0x%x\n", pshell->m_hWnd );
            trace();
            LRESULT result=pshell->SendMessage(WM_COMMAND, cmd);
            swprintf_s(tracebuffer, L"DLL>>: Sendmessage returns: %d\n", result);
            trace();
            return TRUE;
        } else { // Vista
            EnumChildWindows(pWnd->GetSafeHwnd(),   (WNDENUMPROC)wndProc1,   NULL);
            swprintf_s(tracebuffer, L"DLL>>: Shellview lst2 in Vista: 0x%x\n", lst2Handle1);
            trace();

            pshell = CWnd::FromHandle(lst2Handle1);
            LRESULT result=pshell->SendMessage(WM_COMMAND, XLVM_VISTA_LARGE_ICONS);
            swprintf_s(tracebuffer, L"DLL>>: Sendmessage returns: %d\n", result);
            trace();
        }

    }

    return FALSE;
}





BOOL XPThumbnailDialog::OnInitDialog() {
    // this function is not supported in Vista
    CFileDialog::OnInitDialog();


    // This message is essential for getting the correct
    // listview in XP
    PostMessage(MYWM_POSTINIT,0,0);

    return TRUE;
    // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT XPThumbnailDialog::OnPostInit(WPARAM wp, LPARAM lp) {
    // this is a customized method
    // do nothing here

    swprintf_s(tracebuffer, L"DLL>>: OnPostInit in XPThumbnailDialog\n");
    trace();

    return 0;
}


void XPThumbnailDialog::OnInitDone() {
    // This function will be called when all controls are ready
    // But tt's not supported in Vista now.

    swprintf_s(tracebuffer, L"DLL>>: OnInitDone in XPThumbnailDialog\n");
    trace();

//	  HideControl(cmb1); // hide the file types combobox
//	  HideControl(stc2); // hide the label before the file types combobox
//	swprintf_s(tracebuffer, L"DLL>>: Hide file types combobox!\n");
    trace();
}



void XPThumbnailDialog::OnFolderChange() {

    swprintf_s(tracebuffer, L"DLL>>: Folder change\n\n");
    trace();

    // Whenever the folder is changed, the viewmode need to be
    // set to thumbnail view.

    if(true) {
        swprintf_s(tracebuffer, L"DLL>>: SetViewMode in XPThumbnailDialog OnFolderChange\n");
        trace();
        SetListViewMode(XLVM_XP_THUMBNAILS);
    }

}

void XPThumbnailDialog::OnFileNameChange() {
    swprintf_s(tracebuffer, L"DLL>>:: OnFileNameChange\n\n");
    trace();

}



