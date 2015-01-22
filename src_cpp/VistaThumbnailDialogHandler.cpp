/////////////////////////////////////////////////////////////////////////////
// ViustaThumbnailDialogHandler.h :
//
// interface of the VistaThumbnailDialogHandler class
//

#include <afxwin.h>
#include <afxext.h>         // MFC extensions
#include <stdio.h>

#include <afxdlgs.h>
#include <dlgs.h>

#include "VistaThumbnailDialogHandler.h"

HWND lst2Handle;  // lst2=0x461 in Xp/Vista

// This empty class is provide just for future extension
//
VistaThumbnailDialogHandler::VistaThumbnailDialogHandler() {

}

VistaThumbnailDialogHandler::~VistaThumbnailDialogHandler() {
}


/////////////////////////////////////////////////////////////////////////////
// IFileDialogEvents methods

STDMETHODIMP VistaThumbnailDialogHandler::OnFileOk ( IFileDialog* pfd ) {
//HRESULT hr;
    CComQIPtr<IFileOpenDialog> pDlg = pfd;
    CComPtr<IShellItemArray> pItemArray;

    printf(">> OnFileOk\n");

    if ( !pDlg )
        return S_OK;    // allow the dialog to close

    // do something if necessary
    return S_OK;    // allow the dialog to close
}

STDMETHODIMP VistaThumbnailDialogHandler::OnFolderChanging ( IFileDialog* pfd, IShellItem* psiFolder ) {
    CString sPath;

    printf(">> OnFolderChanging\n");

    // do something

    return S_OK;    // allow the change
}


STDMETHODIMP VistaThumbnailDialogHandler::OnFolderChange ( IFileDialog* pfd ) {
    HRESULT hr;
    CComQIPtr<IFileOpenDialog> pDlg = pfd;
    CComPtr<IShellItem> psiCurrFolder;

    printf(">> OnFolderChange\n");
    if ( !pDlg )
        return S_OK;


    // To find the window handle of thie filedialog is the first task
    IOleWindow *pWindow;
    hr = pfd->QueryInterface(IID_PPV_ARGS(&pWindow));

    HWND hWnd=0;
    if (SUCCEEDED(hr)) {
        hr = pWindow->GetWindow(&hWnd);
        if(SUCCEEDED(hr)) {
            PostMessage(hWnd, WM_COMMAND, IDOK, 0);
        }
        pWindow->Release();
    }
    printf("IFileDialog 's hWnd is: 0x%x", hWnd);

    // do something here

    if(hWnd!= 0) {

        // sendmessage does work not in COM+JNI
        // CFileDialog is still recommended.
    }


    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnSelectionChange ( IFileDialog* pfd ) {
//HRESULT hr;
    CComQIPtr<IFileOpenDialog> pDlg = pfd;
    CComPtr<IShellItemArray> pItemArray;

    printf(">> OnSelectionChange\n");

    if ( !pDlg )
        return S_OK;    // allow the dialog to close

    // do something here

    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnShareViolation (
    IFileDialog* pfd, IShellItem* psi, FDE_SHAREVIOLATION_RESPONSE* pResponse ) {
    CString sPath;

    printf(">> OnShareViolation\n");
    *pResponse = FDESVR_DEFAULT;

    // do something

    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnTypeChange ( IFileDialog* pfd ) {
    printf(">> OnTypeChange\n");

    // do something
    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnOverwrite (
    IFileDialog* pfd, IShellItem* psi, FDE_OVERWRITE_RESPONSE* pResponse ) {
    CString sPath;

    printf(">> OnOverwrite\n");
    *pResponse = FDEOR_DEFAULT;

    // do something

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileDialogControlEvents methods

STDMETHODIMP VistaThumbnailDialogHandler::OnItemSelected (
    IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem ) {
    CComQIPtr<IFileDialog> pDlg = pfdc;
    CString sCtrlText;

    printf(">> OnItemSelected");

    // do something


    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnButtonClicked (
    IFileDialogCustomize* pfdc, DWORD dwIDCtl ) {
    CComQIPtr<IFileDialog> pDlg = pfdc;
    CString sCtrlText;

    printf(">> OnButtonClicked, button ID: %u\n", dwIDCtl);

    // do something

    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnCheckButtonToggled (
    IFileDialogCustomize* pfdc, DWORD dwIDCtl, BOOL bChecked ) {
    CComQIPtr<IFileDialog> pDlg = pfdc;
    int state = bChecked ? CDCS_VISIBLE|CDCS_ENABLED : CDCS_VISIBLE;

    printf(">> OnCheckButtonToggled, button ID: %u, checked?: %d\n", dwIDCtl, bChecked);

    // do something

    return S_OK;
}

STDMETHODIMP VistaThumbnailDialogHandler::OnControlActivating (
    IFileDialogCustomize* pfdc, DWORD dwIDCtl ) {
    printf(">> OnControlActivating, control ID: %u\n", dwIDCtl);

    // do something
    //
    return S_OK;
}
