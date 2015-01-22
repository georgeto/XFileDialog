//  predefined symbols
//
// XPThumbnailDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// XPThumbnailDialog
// a CFileDialg with default thumbnail mode


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


// The following 3 strings can used as filters,
// It can help CFileDialog to display only folders


class XPThumbnailDialog : public CFileDialog {
    DECLARE_DYNAMIC(XPThumbnailDialog)
public:
    XPThumbnailDialog(BOOL bOpenFileDialog = TRUE, // TRUE for FileOpen, FALSE for FileSaveAs
                      LPCTSTR lpszDefExt = NULL,
                      LPCTSTR lpszFileName = NULL,
                      DWORD dwFlags = OFN_EXPLORER|OFN_HIDEREADONLY,
                      LPCTSTR lpszFilter = NULL,
                      CWnd* pParentWnd = NULL);

// Attributes
public:


// Dialog Data
    //{{AFX_DATA(XPThumbnailDialog)
    CListBox	m_wndSelectionsList;
    BOOL	m_bExplorer;
    //}}AFX_DATA


// Operations
public:
    BOOL SetListViewMode(UINT cmd);
    BOOL IsXP();
    BOOL IsVista();
// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(XPThumbnailDialog)
protected:
    //}}AFX_VIRTUAL

    virtual void  OnFolderChange();
    virtual void  OnFileNameChange();

    afx_msg LRESULT OnPostInit(WPARAM wp, LPARAM lp); // the REAL
    // initialization

// Implementation
protected:

    BOOL m_bSelectMode; // TRUE when in "Selection" mode


    //{{AFX_MSG(XPThumbnailDialog)
    afx_msg void OnSelectButton();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    virtual BOOL OnInitDialog();
    virtual void  OnInitDone();
    //}}AFX_MSG
    afx_msg void OnHelp();
    DECLARE_MESSAGE_MAP()
};
