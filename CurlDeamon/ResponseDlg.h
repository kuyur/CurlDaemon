
#pragma once
#ifndef RESPONSEDIG_H_
#define RESPONSEDIG_H_

class CResponseDlg : public CDialogImpl<CResponseDlg>
{
protected: 
    WTL::CString m_response;
public:
    CResponseDlg(WTL::CString response) : m_response(response)
    {
        m_response.Replace(L"\n", L"\r\n");
    };
    ~CResponseDlg() {};
    enum { IDD = IDD_RESPONSE_DIALOG };

    BEGIN_MSG_MAP(CResponseDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_EDIT_RESPONSE, EN_SETFOCUS, OnEnSetfocusEditResponse)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        CEdit edit = (CEdit)GetDlgItem(IDC_EDIT_RESPONSE);
        edit.SetWindowText(m_response);

        return TRUE;
    };
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return 0;
    };
    LRESULT OnEnSetfocusEditResponse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CEdit edit = (CEdit)GetDlgItem(IDC_EDIT_RESPONSE);
        edit.SetSel(0, 0);
        return 0;
    };
};

#endif