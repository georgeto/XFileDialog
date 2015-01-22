/////////////////////////////////////////////////////////////////////////////
//
// Selecting folders under XP
//
// XPFolderDialog.cpp : The implementation file
// It use CListCtrl to read the real names.
// This method only works in XP
//
// Selecting folders in Vista/Win7 is implemented with IFileDialog
// in another file.
//
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
#include "XPFolderDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString currentDir;
extern int traceLevel;
extern TCHAR tracebuffer[1000];
extern void trace();


WNDPROC	XPFolderDialog::m_wndProc = NULL;

const int MYWM_POSTINIT = WM_USER+1;

IMPLEMENT_DYNAMIC(XPFolderDialog, CFileDialog)

BEGIN_MESSAGE_MAP(XPFolderDialog, CFileDialog)
    //{{AFX_MSG_MAP(XPFolderDialog)
    ON_WM_CREATE()
END_MESSAGE_MAP()


// A global counter used in the Window hook
INT_PTR selectioncounter=0;

// The Constructor
// Force it to use OFN_EXPLORER
XPFolderDialog::XPFolderDialog(BOOL bOpenFileDialog, DWORD dwFlags, LPCTSTR lpszFilter, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, CWnd* pWnd) :
    CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, OFN_HIDEREADONLY|OFN_EXPLORER|dwFlags, lpszFilter, pWnd, 0, false) {

    //{{AFX_DATA_INIT(XPFolderDialog)
    m_bExplorer = FALSE;
    //}}AFX_DATA_INIT

    if (dwFlags & OFN_EXPLORER) {
        m_bExplorer = TRUE;
    } else if (m_ofn.Flags & OFN_EXPLORER) {
        // MFC added it, but we don't want it
        m_ofn.Flags &= ~(OFN_EXPLORER | OFN_SHOWHELP);
    }
    swprintf_s(tracebuffer, L"DLL>>: XPFolderDialog is initialized.\n");
    trace();
}



// This is a hack from MSDN
// Although it is not offcially supported by MS,
// a lot of people are using this method.
//
// It works well in XP, and I do not think MS will break it
// with future XP updates (no such updates?)
//
//
//
BOOL XPFolderDialog::ReadListViewNames(CStringList& strlist) {

    if (!m_bExplorer)  // only supported for Explorer Dialog
        return FALSE;

    CWnd* pWnd = GetParent()->GetDlgItem(lst2);
    if (pWnd == NULL)
        return FALSE;

    CListCtrl* wndLst1 = (CListCtrl*)(pWnd->GetDlgItem(1));

    UINT nSelected = wndLst1->GetSelectedCount();
    if (!nSelected)
        // nothing selected -- don't retrieve list
        return FALSE;

    CString strDirectory = GetFolderPath();
    // /\ 4.2: undocumented, but non-implementation
    CString strItemText;
    // Could this iteration code be cleaner?
    for (int nItem = wndLst1->GetNextItem(-1,LVNI_SELECTED);
            nSelected-- > 0; nItem = wndLst1->GetNextItem(nItem, LVNI_SELECTED)) {
        strItemText = wndLst1->GetItemText(nItem,0);
        if (strDirectory.Right(1) == _T("\\"))
            strItemText = strDirectory + strItemText;
        else
            strItemText = strDirectory + _T("\\") + strItemText;

        m_wndSelectionsList.AddString(strItemText);
        m_listDisplayNames.AddHead(strItemText);
    }

    return TRUE;
}

// Validate filename
BOOL XPFolderDialog::OnFileNameOK() {
    // Set the selections
    //
    // External programs will use GetItemName, GetItemNumber
    // to read the internal List and get all returned folders.
    //
    //
    ReadListViewNames(m_listDisplayNames);

    return FALSE;
}



BOOL XPFolderDialog::OnInitDialog() {
    CFileDialog::OnInitDialog();

    m_wndProc = NULL;


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}



// Window hook that deals with MESSAGE directly
// Since Vista style dialog does not support hook,
// this class can not be used in Vista/Win7.
//
//
LRESULT CALLBACK WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam) {

    swprintf_s(tracebuffer, L"DLL>>: Window Hook in XPFolderDialog\n");
    trace();


    if (message ==  WM_COMMAND)	{

        if (HIWORD(wParam) == BN_CLICKED)	{

            if (LOWORD(wParam) == IDOK) {

                if (CFileDialog* pDlg = (CFileDialog*)CWnd::FromHandle(hwnd)) {
                    // determine whether select anything
                    // before exit
                    if(selectioncounter>0) {
                        swprintf_s(tracebuffer, L"DLL>>: XPFolderDialog: Select %d\n", selectioncounter);
                        trace();

                        pDlg->EndDialog(IDOK);
                        XPFolderDialog::m_wndProc = NULL;


                        return NULL;
                    } else {
                        swprintf_s(tracebuffer, L"DLL>>: XPFolderDialog: Select nothing\n");
                        trace();
                        pDlg->EndDialog(IDOK);
                        XPFolderDialog::m_wndProc = NULL;

                    }

                }

            }

        }

    }


    return CallWindowProc(XPFolderDialog::m_wndProc, hwnd, message, wParam, lParam);

}

void XPFolderDialog::OnInitDone() {
    // After the GUI layout, you can hide some controls in this method
    swprintf_s(tracebuffer, L"XPFolder DLL OnInitDone! \n");
    trace();


    // Hide the "file types" label and combobox
    // Folder Dialog does not need them
    HideControl(cmb1);
    HideControl(stc2);

    CWnd* pFD = GetParent();

    swprintf_s(tracebuffer, L"DLL>>: create Window Hook in X64: m_hWnd: %d  callback: %d\n", pFD->m_hWnd, (long)WindowProcNew);
    trace();

#ifdef OS_ARCH_X86
    m_wndProc = (WNDPROC)SetWindowLong(pFD->m_hWnd, GWL_WNDPROC, (long)WindowProcNew);
    swprintf_s(tracebuffer, L"DLL>>: m_hWnd In X64: %d\n", pFD->m_hWnd );
    trace();

    swprintf_s(tracebuffer, L"DLL>>: Last Window HooK proc In X86: %d\n", m_wndProc);
    trace();

#endif

#ifdef OS_ARCH_X64
    //SetWindowLong in Win2000/XP has been superseded with SetWindowLongPtr
    //
    swprintf_s(tracebuffer, L"DLL>>: create Window Hook in X64: m_hWnd: %d  callback: %d\n", pFD->m_hWnd, (long)WindowProcNew);
    trace();

    m_wndProc = (WNDPROC)SetWindowLongPtr(pFD->m_hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProcNew);
    swprintf_s(tracebuffer, L"DLL>>: m_hWnd In X64: %d\n", pFD->m_hWnd );
    trace();
    swprintf_s(tracebuffer, L"DLL>>: Last Window HooK proc In X64: %d\n", m_wndProc);
    trace();

#endif





}


void XPFolderDialog::OnFolderChange() {
    swprintf_s(tracebuffer, L"DLL>>: Folder change\n");
    trace();

    // initialize currentDir for the firstTime

    if( currentDir.IsEmpty() ) {
        // give it a value for the first time
        currentDir= GetFolderPath();
    }

    // Reset the file list when you change the folder
    m_listDisplayNames.RemoveAll();
    ReadListViewNames(m_listDisplayNames);
    POSITION pos;
    for(pos=m_listDisplayNames.GetHeadPosition(); pos!=NULL; ) {
        swprintf_s(tracebuffer, L"DLL>>: Selected items: %s \n" , m_listDisplayNames.GetNext(pos) );
        trace();
    }
    selectioncounter= m_listDisplayNames.GetSize();  //sync

}

void XPFolderDialog::OnFileNameChange() {

    OnFolderChange();
    SetCmb13();
}

// key elements
void XPFolderDialog::SetCmb13() {
    // set the filename text field, actually a combo box
    GetParent()->GetDlgItem(cmb13)->SetWindowText( GetItemName() );
}

INT_PTR XPFolderDialog::GetItemNumber() {
    return m_listDisplayNames.GetSize();
}

CString XPFolderDialog::GetItemName() {
    if(!m_listDisplayNames.IsEmpty()) {
        CString str1 = m_listDisplayNames.GetHead();
        return str1;
    } else {
        CString str1("\0");
        return str1;
    }
}

CString XPFolderDialog::GetItemName(int i) {
    if(!m_listDisplayNames.IsEmpty()) {
        POSITION pos;
        pos = m_listDisplayNames.FindIndex(i);
        CString str1 = m_listDisplayNames.GetAt(pos);
        return str1;
    } else {
        CString str1("\0");
        return str1;
    }
}


