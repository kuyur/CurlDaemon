// CurlDeamonView.h : interface of the CCurlDeamonView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CURLDEAMONVIEW_H_
#define CURLDEAMONVIEW_H_

#include <unordered_map>
#include "config.h"

extern CConfig _Config;

class CCurlDeamonView : public CDialogImpl<CCurlDeamonView>, public CUpdateUI<CCurlDeamonView>,
        public CMessageFilter, public CIdleHandler, public CWinDataExchange<CCurlDeamonView>
{
private:
    std::unordered_map<int, RECT> m_itemRects; // Orignal size of dialog items
    RECT m_dlgRect;                            // Orinal rect of dialog

public:
    enum { IDD = IDD_MAIN_DIALOG };
    CCurlDeamonView() {};
    ~CCurlDeamonView() {};

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();

    BEGIN_UPDATE_UI_MAP(CCurlDeamonView)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CCurlDeamonView)
        CHAIN_MSG_MAP(CUpdateUI<CCurlDeamonView>)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SIZE, onDialogResize)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_HANDLER(IDC_BUTTON_EXCUTE, BN_CLICKED, OnBnClickedButtonExcute)
        COMMAND_HANDLER(IDC_COMBO_METHOD, CBN_SELCHANGE, OnCbnSelchangeComboMethod)
        COMMAND_HANDLER(IDC_CHECK_SENDRANDOMLY, BN_CLICKED, OnBnClickedCheckSendrandomly)
        COMMAND_HANDLER(IDC_CHECK_LOADHOLIDAY, BN_CLICKED, OnBnClickedCheckLoadholiday)
    END_MSG_MAP()

    // DDX
    BEGIN_DDX_MAP(CCurlDeamonView)
        DDX_TEXT(IDC_EDIT_URL, _Config.http_url)
        DDX_TEXT(IDC_EDIT_CONTENT, _Config.http_sending_content)
        DDX_CHECK(IDC_CHECK_SENDRANDOMLY, _Config.schedule_repeat_randomly)
        DDX_CHECK(IDC_CHECK_IGNOREHOLIDAY, _Config.schedule_ignore_holiday)
        DDX_CHECK(IDC_CHECK_LOADHOLIDAY, _Config.schedule_load_holiday)
        DDX_TEXT(IDC_EDIT_HOLIDAYURL, _Config.schedule_holiday_url)
    END_DDX_MAP()

// Handler prototypes (uncomment arguments if needed):
//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onDialogResize(UINT, WPARAM, LPARAM, BOOL&);

private:
    void getDlgItemsRelativePosition();
    void moveItem(int itemId, int deltaX, int deltaY);
    void resizeItem(int itemId, int deltaX, int deltaY);
    void updateConfig();
    bool initCurlConn(CURL *conn, char *errorBuffer, std::string *buffer, curl_slist *header_list, const char* sending_content);
public:
    LRESULT OnBnClickedButtonExcute(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCbnSelchangeComboMethod(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckSendrandomly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckLoadholiday(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif