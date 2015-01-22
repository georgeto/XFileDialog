//  predefined symbols
//
// XPFolderDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// XPFolderDialog dialog

// The following 3 strings can used as filters,
// It can help CFileDialog to display only folders


static const TCHAR * ONLYFOLDERS = _T(" | ||");
static const TCHAR * FOLDERSANDFILES= NULL;
static const TCHAR * ONLYFILES = NULL;

class XPFolderDialog : public CFileDialog {
    DECLARE_DYNAMIC(XPFolderDialog)
public:
    XPFolderDialog(BOOL bOpenFileDialog = TRUE, // TRUE for FileOpen, FALSE for FileSaveAs
                   DWORD dwFlags = OFN_EXPLORER|OFN_HIDEREADONLY,
                   LPCTSTR lpszFilter = NULL,
                   LPCTSTR lpszDefExt = NULL,
                   LPCTSTR lpszFileName = NULL,
                   CWnd* pParentWnd = NULL);

    static	WNDPROC m_wndProc;

// Attributes
public:
    CStringList m_listDisplayNames;
    // list of actual items selected in listview
    //


// Dialog Data
    //{{AFX_DATA(XPFolderDialog)
    enum { IDD = 130 };
    CListBox	m_wndSelectionsList;
    BOOL	m_bExplorer;
    //}}AFX_DATA


// Operations
public:
    CString GetItemName();
    CString GetItemName(int i);
    void SetCmb13();
    INT_PTR GetItemNumber();
    BOOL SetListViewMode(UINT cmd);
    BOOL IsXP();
    BOOL IsVista();
// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(XPFolderDialog)
protected:
    //}}AFX_VIRTUAL
    virtual BOOL OnFileNameOK();
    virtual void  OnFolderChange();
    virtual void  OnFileNameChange();

    afx_msg LRESULT OnPostInit(WPARAM wp, LPARAM lp); // the REAL
    // initialization



// Implementation
protected:

    BOOL m_bSelectMode; // TRUE when in "Selection" mode

    BOOL ReadListViewNames(CStringList& strlist);	// protected -> not callable without dialog up


    //{{AFX_MSG(XPFolderDialog)
    afx_msg void OnSelectButton();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    virtual BOOL OnInitDialog();
    virtual void  OnInitDone();
    //}}AFX_MSG
    afx_msg void OnHelp();
    DECLARE_MESSAGE_MAP()
};
