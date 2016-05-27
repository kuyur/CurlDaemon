// CurlDeamon.cpp : main source file for CurlDeamon.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include "resource.h"
#include "../common/winfile.h"
#include "../common/win32helper.h"
#include "config.h"
#include "CurlDeamonView.h"
#include "aboutdlg.h"
#include "MainFrm.h"

CAppModule _Module;
CConfig _Config; // The global instance of configuration

void load_config(const WTL::CString &config_path)
{
    set_default(_Config);
    // Load config file...
    // Because TiXml does not support wchar_t file name,
    // use Win32 File Api to load xml file.
    CWinFile file(config_path, CWinFile::modeRead | CWinFile::shareDenyWrite);
    if (!file.open())
        save_config_file(config_path, _Config);
    else
    {
        UINT file_length = file.length();
        char *file_buffer = new char[file_length + 1];
        memset((void*)file_buffer, 0, file_length + 1);
        file.seek(0, CWinFile::begin);
        file.read(file_buffer, file_length);
        file.close();

        TiXmlDocument *doc = new TiXmlDocument;
        doc->Parse(file_buffer, NULL, TIXML_ENCODING_UTF8);
        if (doc->Error() || !load_config_file(doc, _Config))
        {
            ::DeleteFile(config_path);
            set_default(_Config);
            save_config_file(config_path, _Config);
        }

        delete[]file_buffer;
        delete doc;
    }
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;

    if(wndMain.CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
    HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//    HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_BAR_CLASSES);    // add flags to support other controls

    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    // load config
    WTL::CString config_path(GetProcessFolder());
    config_path += L"config-curl-deamon.xml";
    load_config(config_path);

    int nRet = Run(lpstrCmdLine, nCmdShow);
    save_config_file(config_path, _Config);

    _Module.Term();
    ::CoUninitialize();

    return nRet;
}
