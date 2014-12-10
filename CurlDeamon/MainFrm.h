// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainFrame : 
    public CFrameWindowImpl<CMainFrame>, 
    public CUpdateUI<CMainFrame>,
    public CMessageFilter, public CIdleHandler
{
private:
    int m_minWidth;
    int m_minHeight;

public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

    CCurlDeamonView m_view;

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
            return TRUE;

        return m_view.PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        return FALSE;
    }

    BEGIN_UPDATE_UI_MAP(CMainFrame)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnResized)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        // center the main window on the screen
        CenterWindow();

        m_hWndClient = m_view.Create(m_hWnd);
        // get required size of dialog
        RECT dlgRect;
        m_view.GetWindowRect(&dlgRect);
        // get size of current client rect for dialog
        RECT clientRect;
        GetClientRect(&clientRect);
        // get size of main frame
        RECT mainRect;
        GetWindowRect(&mainRect);
        m_minWidth = mainRect.right - mainRect.left + (dlgRect.right - dlgRect.left) - (clientRect.right - clientRect.left);
        m_minHeight = mainRect.bottom - mainRect.top + (dlgRect.bottom - dlgRect.top) - (clientRect.bottom - clientRect.top);
        // resize main frame
        SetWindowPos(NULL, 0, 0, m_minWidth, m_minHeight, SWP_NOMOVE | SWP_NOACTIVATE);

        // register object for message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);

        return 0;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        // unregister message filtering and idle updates
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);

        bHandled = FALSE;
        return 1;
    }

    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        PostMessage(WM_CLOSE);
        return 0;
    }

    LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        // TODO: add code to initialize document

        return 0;
    }

    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CAboutDlg dlg;
        dlg.DoModal();
        return 0;
    }

    LRESULT OnResized(UINT, WPARAM, LPARAM lParam, BOOL&)
    {
        RECT rc;
        GetWindowRect(&rc);

        RECT clientRc;
        GetClientRect(&clientRc);

        // set size of main dialog
        m_view.SetWindowPos(NULL, &clientRc, SWP_NOMOVE);
        return 0;
    }

    LRESULT CMainFrame::OnGetMinMaxInfo(UINT, WPARAM, LPARAM lParam, BOOL&)
    {
        // load size structure with lParam values
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

        // change the values in the size structure to desired values
        lpMMI->ptMinTrackSize.x = m_minWidth;  // min width
        lpMMI->ptMinTrackSize.y = m_minHeight; // min height

        return 0;
    }
};
