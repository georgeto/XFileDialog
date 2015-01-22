//  Author: Stevpan@gmail.com
//
//
//  Windows CFileDialog has a lot of strange behaviors,
//  please read the code carefully, if you are curious.
//
//  The C code was compiled with Unicode turned on (UTF-16 in Windows)
//  It will return a byteArray containing UTF-16 byte stream
//
//  The Java Code will use UTF-16 to decode and reconstruct the
//  real String object.
//
// XFileDialog.cpp
//
// this is the entry point and the main class
//
// it uses XPFolderDialog.cpp to select folders under XP
// it uses IFileDialog to select folders under Vista/Win7
//
// it uses XPThumbnailDialog.cpp to select images under both XP and Vista/Win7
//
// it uses the old CFileDialog for single-file-selection or multi-file-selection
//

#include <jni.h>
#include <stdio.h>
#include <afxdlgs.h>
#include <comdef.h>
#include <afxpriv.h>

#include "net_tomahawk_XFileDialog.h"
#include "XPFolderDialog.h"
#include "XPThumbnailDialog.h"
#include "VistaThumbnailDialogHandler.h"

// Windows SDK header files in
// e.g. d:\Windows\Microsoft Sdks\Windows\v6.0A\Include
#include "ShFolder.h"
#include "ShObjIdl.h"
#include "KnownFolders.h"

// JNI header
#include <jawt.h>
#include <jawt_md.h>

#define MAX_SELECTION 1000


// In Unicode mode,
// You should use swprintf(), or swprintf_s() not wsprintf();
// In Ansi mode
// You should use sprintf();

// wsprintf() should never be used in my opinion
// MS's function names are too confusing.



// control the debug output
//
int traceLevel=1;
TCHAR tracebuffer[1000];

HWND hWnd=0;  // handle of the JFrame
CWnd* pWnd;   // window object of the JFrame
IShellItem *currentDirItem; // Item in IFileDialog
CString currentDir;   // the current directory
bool bOpenFileDialog=TRUE;  // open or save dialog
CString title;   // the dialog title
CString filters;  // file filters
CString initialFilename; //Initial Filename
CString defaultExtension; //Default Extension

UINT listviewmode=0;
// 0 means standard filedialog
// > 1 means thumbnail view mode filedialog

// debug trace
void trace() {
    // it uses pure tchar array, not CString
    if(traceLevel> 0) wprintf(tracebuffer);

    // else do not show anything

}


BOOL IsXP() {
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osinfo);

    DWORD dwPlatformId   = osinfo.dwPlatformId;
    DWORD dwMajorVersion = osinfo.dwMajorVersion;
    DWORD dwMinorVersion = osinfo.dwMinorVersion;

    return ((dwPlatformId == 2) &&
            (dwMajorVersion == 5) &&
            (dwMinorVersion >= 1));
}

BOOL IsVista() {
    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osinfo);

    DWORD dwPlatformId   = osinfo.dwPlatformId;
    DWORD dwMajorVersion = osinfo.dwMajorVersion;

    return ((dwPlatformId == 2) &&
            (dwMajorVersion >= 6));
}


jbyteArray CString2ByteArray(JNIEnv *env, CString val) {

    // Important
    //
    // Windows Unicode is UTF-16
    //
    // The TCHAR (wide character) in Windows refers
    // unsigned short wchar_t;
    //
    // Unicode uses TCHAR to represent real characters.
    // A character generally will require one TCHAR
    // (multi-TCHAR is used in  rare cases)
    //
    // For characters in the Basic Multilingual Plane (BMP) of Unicode
    // A UTF-16 character = A TCHAR
    //
    // the BMP supports about 90 Scripts (languages) including
    // Latin, CJK
    // http://en.wikipedia.org/wiki/Mapping_of_Unicode_character_planes
    //

    const TCHAR* buffer = NULL;
    DWORD bufferlen = 0;
    bufferlen = val.GetLength();

    swprintf_s(tracebuffer, L"DLL>>: %s\n", val);
    trace();

    swprintf_s(tracebuffer, L"DLL>>: CString2ByteArray bufferlen %d\n", bufferlen);
    trace();


    buffer = val.GetBuffer(bufferlen);

    // one short = two bytes

    // should I add bom as the first two bytes when return?
    // does the last two zeros are added automatically?

    // for big edian  0xFE 0xFF header
    // for little edian 0xFF 0xFE
    //
    // if BOM is missing, big endian is used in default
    //

    jbyteArray result = (*env).NewByteArray( bufferlen*2 + 2 );
    (*env).SetByteArrayRegion(result,0, bufferlen*2, (jbyte*)buffer);



    (*env).SetByteArrayRegion(result,2, bufferlen*2, (jbyte*)buffer);

    // the big endian header for UTF-16 encoding
    BYTE   header[2]   =   {0xff, 0xfe};

    (*env).SetByteArrayRegion(result,0, 2, (jbyte*)header);


    return  result;

}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_refreshFrame
(JNIEnv *env, jobject obj) {
    if(pWnd!=NULL) {
        pWnd->Invalidate(NULL);
        pWnd->UpdateWindow();
    }


}


JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_initWithWindowTitle
(JNIEnv *env, jobject obj, jstring windowtitle) {
    // use the string the get the window

    // convert jstring to jchar (actually short 16bit integer )
    const jchar *str = (*env).GetStringChars(windowtitle, 0);
    // convert the jchar array to CString

    CString tmp((BSTR)str);

    hWnd = FindWindow(NULL, tmp);

    swprintf_s(tracebuffer, L"DLL>>:: hWnd of JFrame(awt) is 0x%x\n", hWnd);
    trace();

    pWnd = CWnd::FromHandle(hWnd);

    bOpenFileDialog=TRUE;
    char nullstr = NULL;
    title = CString(nullstr);
    initialFilename = CString(nullstr);
    defaultExtension = CString(nullstr);

    filters = "All files (*.*)|*.*||";   // default

}

HMODULE _hAWT = 0;

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_initWithJAWT
(JNIEnv *env, jobject obj, jobject comp, jstring javahome) {
    // the implementation is correct and elegant
    // if the hWnd < 0, the error can be easily located from its value
    //
    // a possible issue lies in the LoadLibrary
    // if the path of the jre6 is not set correctly,
    // jawt.dll can not be located and this function will fail.
    //
    //
    swprintf_s(tracebuffer, L"DLL>>: initWithJAWT\n" );
    trace();

    hWnd=0; // init it with zero

    const jchar *str = (*env).GetStringChars(javahome, 0);
    CString tmp((BSTR)str);

    CString jrepath = tmp + _T("\\bin");

    swprintf_s(tracebuffer, L"DLL>>: jrepath: %s\n" ,jrepath ); // a unicode CString

    typedef jboolean (JNICALL *PJAWT_GETAWT)(JNIEnv*, JAWT*);
    JAWT awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_Win32DrawingSurfaceInfo* dsi_win;
    jboolean result;
    jint lock;

    //Load AWT Library
    if(!_hAWT)
        //for Java 1.4
        _hAWT = LoadLibrary(jrepath + _T("\\jawt.dll"));

    if(!_hAWT)
        //for Java 1.3
        _hAWT = LoadLibrary(jrepath + _T("\\awt.dll"));

    if(_hAWT) {
#ifdef OS_ARCH_X86
        PJAWT_GETAWT JAWT_GetAWT = (PJAWT_GETAWT)GetProcAddress(_hAWT, "_JAWT_GetAWT@8");
#endif

#ifdef OS_ARCH_X64
        PJAWT_GETAWT JAWT_GetAWT = (PJAWT_GETAWT)GetProcAddress(_hAWT, "JAWT_GetAWT");
#endif

        if(JAWT_GetAWT) {
            awt.version = JAWT_VERSION_1_4; // Init here with JAWT_VERSION_1_3 or JAWT_VERSION_1_4
            //Get AWT API Interface
            result = JAWT_GetAWT(env, &awt);
            if(result != JNI_FALSE) {
                ds = awt.GetDrawingSurface(env, comp);
                if(ds != NULL) {
                    lock = ds->Lock(ds);
                    if((lock & JAWT_LOCK_ERROR) == 0) {
                        dsi = ds->GetDrawingSurfaceInfo(ds);
                        if(dsi) {
                            dsi_win = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
                            if(dsi_win) {
                                hWnd = dsi_win->hwnd;
                            } else {
                                hWnd = (HWND) -1;
                            }
                            ds->FreeDrawingSurfaceInfo(dsi);
                        } else {
                            hWnd = (HWND) -2;
                        }
                        ds->Unlock(ds);
                    } else {
                        hWnd = (HWND) -3;
                    }
                    awt.FreeDrawingSurface(ds);
                } else {
                    hWnd = (HWND) -4;
                }
            } else {
                hWnd = (HWND) -5;
            }
        } else {
            hWnd = (HWND) -6;
        }
    } else {
        hWnd = (HWND) -7;
    }

    // init other variables
    swprintf_s(tracebuffer, L"DLL>>:: hWnd of JFrame found in JAWT: 0x%x\n", hWnd);
    trace();

    pWnd = CWnd::FromHandle(hWnd);

    bOpenFileDialog=TRUE;
    char nullstr = NULL;
    title = CString(nullstr);
    initialFilename = CString(nullstr);
    defaultExtension = CString(nullstr);
    filters = "All files (*.*)|*.*||";



}



JNIEXPORT jbyteArray JNICALL Java_net_tomahawk_XFileDialog_getDirectory2
(JNIEnv *env, jobject obj) {
    return CString2ByteArray(env, currentDir);
}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setDirectory2
(JNIEnv *env, jobject obj, jstring dirstr) {
    if(dirstr==NULL) return;
    const jchar *str = (*env).GetStringChars(dirstr, 0);

    // save it into the String currentDir
    // this variable will be used in CFileDialog creation
    // to force it to the correct init directory
    CString tmp((BSTR)str);
    currentDir=tmp;


    swprintf_s(tracebuffer, L"DLL>>: Set the current directory %s\n",   tmp);
    trace();


    SetCurrentDirectory(tmp);
    // this only set the current working directory,
    // it does not affect the filedialog's init location
}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setMode2
(JNIEnv *env, jobject obj, jint mode) {
    if(mode==0) {
        swprintf_s(tracebuffer, L"DLL>>: Set bOpenFileDialog to be True\n");
        trace();
        bOpenFileDialog =TRUE;
    } else {
        swprintf_s(tracebuffer, L"DLL>>: Set bOpenFileDialog to be False\n");
        trace();
        bOpenFileDialog =FALSE;
    }


}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setThumbnail2
(JNIEnv *env, jobject obj, jint mode) {
    if(mode>0) {
        if( IsXP() )  listviewmode= XLVM_XP_THUMBNAILS;

        if(IsVista() ) listviewmode= XLVM_VISTA_LARGE_ICONS;

    } else {
        listviewmode=0;
    }


}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setTraceLevel2
(JNIEnv *env, jclass cls, jint val) {
    traceLevel= val;
    swprintf_s(tracebuffer, L"DLL>>: TraceLevel: %d\n", traceLevel);
    trace();

}



JNIEXPORT jint JNICALL Java_net_tomahawk_XFileDialog_getMode2
(JNIEnv *env, jobject obj) {
    if(bOpenFileDialog ==TRUE ) return 0;
    else return 1;
}

JNIEXPORT jbyteArray JNICALL Java_net_tomahawk_XFileDialog_getFile2
(JNIEnv *env, jobject obj) {
    CString pathName;
    bool bCanceled=true;


    // the default cfiledialog
    if(listviewmode==0) {
        CFileDialog dlg2(bOpenFileDialog/*Open=TRUE Save=False*/,(LPCTSTR)defaultExtension/*Filename Extension*/,(LPCTSTR)initialFilename/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
        swprintf_s(tracebuffer, L"DLL>>:: CFileDialog was chosen\n");
        trace();

        (dlg2).m_ofn.lpstrTitle = title;
        (dlg2).m_ofn.lpstrInitialDir= currentDir;
        if ( (dlg2).DoModal() == IDOK) {
//         fileName= (dlg2).GetFileName();

            pathName=dlg2.GetPathName();
            bCanceled = false;
        }


    }

    if(listviewmode!=0 ) {

        if(IsXP() || IsVista() ) {
            XPThumbnailDialog dlg1(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
            swprintf_s(tracebuffer, L"DLL>>: XPThumbnailDialog was chosen\n");
            trace();

            (dlg1).m_ofn.lpstrTitle = title;
            (dlg1).m_ofn.lpstrInitialDir= currentDir;
            if ( (dlg1).DoModal() == IDOK) {
//         fileName= (dlg1).GetFileName();
                pathName=dlg1.GetPathName();
                bCanceled = false;
            }



        }

        if(IsVista() && false) {

            // preparation
            // Vista requires such a line, otherwise, Jni can
            // not launch IFileDialog

            swprintf_s(tracebuffer, L"DLL>>:: IFileDialog in Vista was chosen\n");
            trace();

            CFileDialog dlg2(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
            IFileDialog *pfd;

            // CoCreate the dialog object.
            HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(&pfd));

            if (SUCCEEDED(hr)) {
                // set Title for pfd

                hr = pfd->SetTitle (title);

                // Set up our event listener.
                CComObjectStackEx<VistaThumbnailDialogHandler> cbk;
                CComQIPtr<IFileDialogEvents> pEvents = cbk.GetUnknown();
                DWORD dwCookie;
                bool bAdvised;

                hr = pfd->Advise ( pEvents, &dwCookie );
                bAdvised = SUCCEEDED(hr);


                // Show the dialog
                hr = pfd->Show(hWnd);


                if (SUCCEEDED(hr)) {
                    // Obtain the result of the user's interaction with the dialog.
                    IShellItem *pItem;
                    hr = pfd->GetResult(&pItem);

                    if (SUCCEEDED(hr)) {
                        // Do something with the result.

                        LPOLESTR pwsz = NULL;

                        hr = pItem->GetDisplayName ( SIGDN_FILESYSPATH, &pwsz );

                        if ( SUCCEEDED(hr) ) {
                            // convert LPOLESTR => CString
                            USES_CONVERSION;
                            CString itemname(OLE2T(pwsz));

                            pathName= itemname;
                            bCanceled = false;

                            CoTaskMemFree ( pwsz );
                        }

                        pItem->Release();
                    }
                }

                if ( bAdvised ) pfd->Unadvise ( dwCookie );

                pfd->Release();
            }


        }


    } // end with thumbnail dialog

    // calculate the filename and parent path from absolute path name
    if(!bCanceled) {
        int charPosition=pathName.ReverseFind( _T('\\') );
        CString filePath=pathName.Left((charPosition+1));
        currentDir = filePath;

        swprintf_s(tracebuffer, L"DLL>>: charPosition of Seperator: %d \n", charPosition);
        trace();

        CString fileName = pathName.Mid(charPosition+1);

        swprintf_s(tracebuffer, L"DLL>>:: pathName %s\n", pathName);
        trace();

        swprintf_s(tracebuffer, L"DLL>>:: fileName %s\n", fileName);
        trace();

        // reset listviewmode to default case
        listviewmode=0;

        swprintf_s(tracebuffer, L"DLL>>: currentDir: %s\n", currentDir);
        trace();

        jbyteArray bArray = CString2ByteArray(env, fileName);
        return bArray;
    } else
        return 0;

}


JNIEXPORT jbyteArray JNICALL Java_net_tomahawk_XFileDialog_getFolder2
(JNIEnv *env, jobject obj) {

    CString folderName;

    INT_PTR nCounter=0;
    bool UsingDefaultFolder=false;

    // testing the X64 XPFolderDialog for Windows XP 64bit

    if( IsXP() ) {

        XPFolderDialog dlg(bOpenFileDialog, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST, ONLYFOLDERS, NULL, NULL, pWnd/*Flags*/);

        // folder selection should not have an initial filename
        dlg.m_ofn.lpstrInitialDir= currentDir;
        dlg.m_ofn.lpstrTitle = title;

        // folderdialog in XP relies on a Windows Hook
        // feedback must be processed
        INT_PTR feedback = dlg.DoModal();
        if ( feedback == IDOK) {
            INT_PTR num = dlg.GetItemNumber();

            if(num> 0) {
                folderName = dlg.GetItemName();
                nCounter= num;
            } else {
                // return the currentDir when you select nothing but click OK
                // use default selection
                UsingDefaultFolder=true;
                folderName=currentDir;
                nCounter=1;
            }

        }

    }

    if(IsVista() ) {

        // preparation
        // Vista requires such a line, otherwise, Jni can
        // not launch IFileDialog

        swprintf_s(tracebuffer, L"DLL>>:: IFileDialog was chosen\n");
        trace();

        CFileDialog dlg2(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);

        // we have to set the initial Directory
        // otherwise, its behavior will be different from XP.
        //

        IFileDialog *pfd;
        DWORD dwOptions;

        // CoCreate the dialog object.
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&pfd));

        if (SUCCEEDED(hr)) {
            // set Title for pfd

            hr = pfd->SetTitle (title);

            // Specify folder selection
            hr = pfd->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) {
                hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FILEMUSTEXIST);
            }

            // remember last-visit location and set the folder location
            if(currentDirItem!=NULL) {

                hr = pfd->SetFolder(currentDirItem);

            }

            // Show the dialog
            hr = pfd->Show(hWnd);

            if (SUCCEEDED(hr)) {
                // Obtain the result of the user's interaction with the dialog.
                IShellItem *pItem;
                hr = pfd->GetResult(&pItem);

                if (SUCCEEDED(hr)) {
                    LPOLESTR pwsz = NULL;


                    // To deal with the case when you select nothing but click OK
                    // use CurrentDir instead of currentDirItem
                    // the later will crash the JVM
                    //

                    if(!currentDir.IsEmpty()) {

                        hr=pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                        CString tmp1(pwsz);

                        if(currentDir.Compare(tmp1)==0) {
                            UsingDefaultFolder=true;
                        }
                    } else {
                        // init for the first time with the pItem
                        printf("DLL>>: Init Default Folder first time\n");
                        hr=pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                        CString tmp(pwsz);
                        currentDir = tmp.Left(tmp.GetLength());
                        UsingDefaultFolder=true;
                        printf("DLL>>: End of Init Default Folder\n");
                    }

                    if(!UsingDefaultFolder) {
                        // An unusual situation:
                        //
                        // you select nothing but click OK,
                        // In this case, the default current folder will be used,
                        // the currentDirItem will not be set in this case

                        hr= pItem->GetParent(&currentDirItem);
                        if(SUCCEEDED(hr)) {
                            hr= currentDirItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                            if(SUCCEEDED(hr) ) {
                                CString tmp(pwsz);
                                swprintf_s(tracebuffer, L"DLL>>: parent of the current folder: %s\n", tmp);
                                trace();
                            }
                        }
                    }

                    // Do something with the result.

                    hr = pItem->GetDisplayName ( SIGDN_FILESYSPATH, &pwsz );

                    if ( SUCCEEDED(hr) ) {

                        CString itemname(pwsz);

                        folderName= itemname;
                        nCounter=1;
                        swprintf_s(tracebuffer, L"DLL>>: a folder was picked: %s\n", itemname);
                        trace();

                        CoTaskMemFree ( pwsz );
                    }

                    pItem->Release();
                }
            }
            pfd->Release();
        }


    }


    // select something
    if(nCounter > 0) {
        swprintf_s(tracebuffer, L"DLL>>: folder picked in XPFolderDialog : %s\n", folderName );
        trace();

        if(!UsingDefaultFolder) {
            currentDir = folderName.Left(folderName.ReverseFind(_T('\\')) +1 );
            swprintf_s(tracebuffer, L"DLL>>: currentDir: %s\n", currentDir );
            trace();
        }


        jbyteArray bArray = CString2ByteArray(env, folderName);
        return bArray;
    } else {

        return 0;
    }

}


JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setFilters2
(JNIEnv *env, jobject obj, jstring filterstr) {
    const jchar *str = (*env).GetStringChars(filterstr, 0);


    CString tmp((BSTR)str);

    filters= tmp;
}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setTitle2
(JNIEnv *env, jobject obj, jstring dlgtitle) {
    const jchar *str = (*env).GetStringChars(dlgtitle, 0);

    CString tmp((BSTR)str);
    title= tmp;
}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setFilename2
(JNIEnv *env, jobject obj, jstring dlgfilename) {
    const jchar *str = (*env).GetStringChars(dlgfilename, 0);

    CString tmp((BSTR)str);
    initialFilename= tmp;
}

JNIEXPORT void JNICALL Java_net_tomahawk_XFileDialog_setDefaultExtension2
(JNIEnv *env, jobject obj, jstring dext) {
    const jchar *str = (*env).GetStringChars(dext, 0);

    CString tmp((BSTR)str);
    defaultExtension= tmp;
}

JNIEXPORT jbyteArray JNICALL Java_net_tomahawk_XFileDialog_getTitle2
(JNIEnv *env, jobject obj) {
    return CString2ByteArray(env, title);
}

// 	Readme
// 	GetPathName()  // full path
// 	GetFileName() // only filename
// 	GetNextPathName();
// 	GetStartPosition();
//
//
JNIEXPORT jobjectArray JNICALL Java_net_tomahawk_XFileDialog_getFiles2
(JNIEnv *env, jobject obj) {
    // MAX_SELECTION = 500
    // multi-selection at most 500 files one time
    //
    // Change it if you wish
    //

    CString absoluteFileNames[MAX_SELECTION];
    CString pathName;
    LPTSTR lpstrFile= new TCHAR[MAX_PATH*MAX_SELECTION];
    *lpstrFile=0; // set the string to be empty at first
    int nCounter=0;

    // for thumbnail dialog
    if(listviewmode!=0) {

        if(IsXP() || IsVista() ) {

            XPThumbnailDialog dlg1(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
            dlg1.m_ofn.lpstrFile = lpstrFile;
            dlg1.m_ofn.nMaxFile = MAX_PATH*MAX_SELECTION;
            dlg1.m_ofn.lpstrTitle = title;
            dlg1.m_ofn.lpstrInitialDir= currentDir;

            if (dlg1.DoModal() == IDOK) {
                POSITION pos = dlg1.GetStartPosition();

                while(pos) {

                    if(nCounter>=MAX_SELECTION) break;

                    absoluteFileNames[nCounter] = dlg1.GetNextPathName(pos);
                    swprintf_s(tracebuffer, L"DLL>>: filename: %s\n", absoluteFileNames[nCounter]);
                    trace();

                    nCounter++;

                }
                // refresh currentDir
                currentDir = dlg1.GetPathName();
            }

        }

    } else { // for normal files
        CFileDialog dlg2(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
        dlg2.m_ofn.lpstrFile = lpstrFile;
        dlg2.m_ofn.nMaxFile = MAX_PATH*MAX_SELECTION;
        dlg2.m_ofn.lpstrTitle = title;
        dlg2.m_ofn.lpstrInitialDir= currentDir;

        if (dlg2.DoModal() == IDOK) {
            POSITION pos = dlg2.GetStartPosition();

            while(pos) {

                if(nCounter>=MAX_SELECTION) break;

                absoluteFileNames[nCounter] = dlg2.GetNextPathName(pos);
                swprintf_s(tracebuffer, L"DLL>>: Abstract Filename: %s\n", absoluteFileNames[nCounter]);
                trace();

                nCounter++;


            }
            // refresh currentDir
            currentDir = dlg2.GetPathName();
        }


    }

    if(nCounter==1) {
        // selecting only one file with multi-selection may cause
        // incorrect path, it must be fixed here.

        // reverse find and trim last several chars
        int charPosition=currentDir.ReverseFind('\\');
        currentDir=currentDir.Left((charPosition+1));
        swprintf_s(tracebuffer, L"charPosition of seperator : %d \n", charPosition);
        trace();


    }
    if(nCounter> 1) currentDir= currentDir + _T("\\"); // add a last seperator

    // reset listviewmode to default case
    listviewmode=0;

    swprintf_s(tracebuffer, L"DLL>> currentDir: %s\n", currentDir);
    trace();

    delete[] lpstrFile;

    if(nCounter > 0) {
        int i;

        jbyteArray temprow = (*env).NewByteArray(1);
        jclass objCls = (*env).GetObjectClass(temprow);

//	jclass objCls = (*env).FindClass("Ljava/lang/String;");
        jobjectArray objarray = (*env).NewObjectArray(nCounter,objCls,NULL);


        if( IsVista() ) {
            swprintf_s(tracebuffer, L"DLL>>:: return multi-selection from Vista\n");
            trace();

            for(i=0; i<nCounter; i++) {
                jbyteArray row = CString2ByteArray(env, absoluteFileNames[i]);
                (*env).SetObjectArrayElement(objarray,i, row);
                (*env).DeleteLocalRef(row);
            }

        } else {
            swprintf_s(tracebuffer, L"DLL>>:: the array order need to be changed for multi-selection from XP.\n");
            trace();


            for(i=0; i<nCounter-1; i++) {
                jbyteArray row = CString2ByteArray(env, absoluteFileNames[i+1]);
                (*env).SetObjectArrayElement(objarray,i, row);
                (*env).DeleteLocalRef(row);
            }
            jbyteArray row = CString2ByteArray(env, absoluteFileNames[0]);
            (*env).SetObjectArrayElement(objarray,nCounter-1, row);
            (*env).DeleteLocalRef(row);
        }

        return objarray;
    } else {
        return 0;
    }

}

JNIEXPORT jobjectArray JNICALL Java_net_tomahawk_XFileDialog_getFolders2
(JNIEnv *env, jobject obj) {
    INT_PTR nCounter=0;
    bool UsingDefaultFolder=false;
    // at most MAX_SELECTION folders
    CString folderNames[MAX_SELECTION];

    if(IsXP() ) {

        XPFolderDialog dlg(bOpenFileDialog, OFN_EXPLORER|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT, ONLYFOLDERS, NULL, NULL, pWnd/*Flags*/);
        // multi selection should not have an initial filename
        dlg.m_ofn.lpstrInitialDir= currentDir;
        dlg.m_ofn.lpstrTitle = title;


        if ( dlg.DoModal() == IDOK ) {

            INT_PTR num = dlg.GetItemNumber();
            if(num> 0) {
                nCounter= num;
                for(int i=0; i<num; i++) {
                    folderNames[i] = dlg.GetItemName(i);
                }
            } else {
                // return the currentDir when you select nothing but click OK
                folderNames[0]=currentDir;
                nCounter=1;
            }


        }

    }

    if(IsVista() ) {

        // preparation
        // Vista requires such a line, otherwise, Jni can
        // not launch IFileDialog

        swprintf_s(tracebuffer, L"DLL>>:: IFileDialog was chosen\n");

        CFileDialog dlg2(bOpenFileDialog/*Open=TRUE Save=False*/,NULL/*Filename Extension*/,NULL/*Initial Filename*/, OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_FILEMUSTEXIST/*Flags*/,filters/*Filetype Filter*/,pWnd/*parent Window*/);
        IFileOpenDialog *pfd;
        DWORD dwOptions;

        // CoCreate the dialog object.
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&pfd));

        if (SUCCEEDED(hr)) {
            // set Title for pfd

            hr = pfd->SetTitle (title);

            // Specify folder selection
            hr = pfd->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) {
                hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT);
            }

            // set the current folder myself
            // in the JFileChooser's way
            if(!currentDir.IsEmpty()) {
                hr = pfd->SetFolder(currentDirItem);
            }


            // Show the dialog
            hr = pfd->Show(hWnd);

            if (SUCCEEDED(hr)) {


                // Obtain the result of the user's interaction with the dialog.
                IShellItemArray *pItemArray;
                hr = pfd->GetResults(&pItemArray);

                if (SUCCEEDED(hr)) {

                    DWORD cSelItems;
                    // Get the number of selected files.
                    hr = pItemArray->GetCount ( &cSelItems );
                    if ( SUCCEEDED(hr) ) {
                        for ( DWORD j = 0; j < cSelItems; j++ ) {
                            CComPtr<IShellItem> pItem;
                            // Get an IShellItem interface on the next file.
                            hr = pItemArray->GetItemAt ( j, &pItem );
                            if ( SUCCEEDED(hr) ) {
                                LPOLESTR pwsz = NULL;

                                // keep the parent when j==0 (the first file)


                                if(j==0) {

                                    // To deal with the case when you select nothing but click OK
                                    if(!currentDir.IsEmpty()) {

                                        hr=pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                                        CString tmp1(pwsz);

                                        if(currentDir.Compare(tmp1)==0) {
                                            UsingDefaultFolder=true;
                                        }
                                    } else {
                                        // init for the first time with the pItem
                                        printf("DLL>>: Init Default Folder first time\n");
                                        hr=pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                                        CString tmp(pwsz);
                                        currentDir = tmp.Left(tmp.GetLength());
                                        UsingDefaultFolder=true;
                                        printf("DLL>>: End of Init Default Folder\n");
                                    }


                                    if(!UsingDefaultFolder) {
                                        hr= pItem->GetParent(&currentDirItem);
                                        if(SUCCEEDED(hr)) {
                                            hr= currentDirItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz );
                                            if(SUCCEEDED(hr) ) {
                                                CString tmp(pwsz);
                                                swprintf_s(tracebuffer, L"DLL>>: parnent of current folders: %s\n", tmp);
                                                trace();
                                            }



                                        }
                                    }
                                } // end of j=0

                                // Get its file system path.
                                //
                                hr = pItem->GetDisplayName ( SIGDN_FILESYSPATH, &pwsz );
                                // convert it to Cstring
                                if ( SUCCEEDED(hr) ) {
                                    USES_CONVERSION;
                                    CString itemname(OLE2T(pwsz));
                                    folderNames[j]= itemname;
                                    nCounter++;
                                    CoTaskMemFree ( pwsz );
                                }
                            }
                        } // end for loop
                    } // the array was get successfully

                    pItemArray->Release();
                }
            }

            pfd->Release();
        }


    }
    if(nCounter >0) {

        if(!UsingDefaultFolder) {
            currentDir = folderNames[0].Left(folderNames[0].ReverseFind(_T('\\')) +1 );
            swprintf_s(tracebuffer, L"DLL>>: currentDir : %s\n", currentDir);
        }

        // return an object array
        jbyteArray temprow = (*env).NewByteArray(1);
        jclass objCls = (*env).GetObjectClass(temprow);

        //	jclass objCls = (*env).FindClass("Ljava/lang/String;");
        jobjectArray objarray = (*env).NewObjectArray((jsize)nCounter,objCls,NULL);

        for(int i=0; i<nCounter; i++) {
            jbyteArray row = CString2ByteArray(env, folderNames[i]);
            (*env).SetObjectArrayElement(objarray,i, row);
            (*env).DeleteLocalRef(row);
        }

        return objarray;
    } else
        return 0;  // 0 is NULL

}


