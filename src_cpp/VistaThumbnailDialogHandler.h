// VistaThumbnailDialogHandler.h : interface of the VistaThumbnailDialogHandler class
/////////////////////////////////////////////////////////////////////////////

// to use com
//
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include <atlbase.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

// IFileDialog com interface
#include <shobjidl.h>

#include <comdef.h>

#define XLVM_VISTA_DO_NOT_PERSIST		(-1)
#define XLVM_VISTA_UNDEFINED			0
#define XLVM_VISTA_DETAILS				0x704B
#define XLVM_VISTA_TILES				0x704C
#define XLVM_VISTA_EXTRA_LARGE_ICONS	0x704D
#define XLVM_VISTA_MEDIUM_ICONS			0x704E
#define XLVM_VISTA_LARGE_ICONS			0x704F
#define XLVM_VISTA_SMALL_ICONS			0x7050
#define XLVM_VISTA_LIST					0x7051



#pragma once

class VistaThumbnailDialogHandler : public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<VistaThumbnailDialogHandler>,
    public IFileDialogEvents,
    public IFileDialogControlEvents {
public:
    VistaThumbnailDialogHandler();
    ~VistaThumbnailDialogHandler();

    BEGIN_COM_MAP(VistaThumbnailDialogHandler)
    COM_INTERFACE_ENTRY(IFileDialogEvents)
    COM_INTERFACE_ENTRY(IFileDialogControlEvents)
    END_COM_MAP()

    // IFileDialogEvents
    STDMETHODIMP OnFileOk(IFileDialog* pfd);
    STDMETHODIMP OnFolderChanging(IFileDialog* pfd, IShellItem* psiFolder);
    STDMETHODIMP OnFolderChange(IFileDialog* pfd);
    STDMETHODIMP OnSelectionChange(IFileDialog* pfd);
    STDMETHODIMP OnShareViolation(IFileDialog* pfd, IShellItem* psi, FDE_SHAREVIOLATION_RESPONSE* pResponse);
    STDMETHODIMP OnTypeChange(IFileDialog* pfd);
    STDMETHODIMP OnOverwrite(IFileDialog* pfd, IShellItem* psi, FDE_OVERWRITE_RESPONSE* pResponse);

    // IFileDialogControlEvents
    STDMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem);
    STDMETHODIMP OnButtonClicked(IFileDialogCustomize* pfdc, DWORD dwIDCtl);
    STDMETHODIMP OnCheckButtonToggled(IFileDialogCustomize* pfdc, DWORD dwIDCtl, BOOL bChecked);
    STDMETHODIMP OnControlActivating(IFileDialogCustomize* pfdc, DWORD dwIDCtl);
};
