
#include "stdafx.h"
#include "resource.h"
#include "CurlDeamonView.h"

BOOL CCurlDeamonView::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

BOOL CCurlDeamonView::OnIdle()
{
    return FALSE;
}

LRESULT CCurlDeamonView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    UIAddChildWindowContainer(m_hWnd);
    // DDX
    DoDataExchange(FALSE);

    // Get orignal size of dialog
    GetWindowRect(&m_dlgRect);
    // Get orignal size of dialog items
    getDlgItemsRelativePosition();

    return 1;
}

LRESULT CCurlDeamonView::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // Get parent frame
    CWindow* parent = &GetParent();
    parent->PostMessage(WM_CLOSE);

    return 0;
}

void CCurlDeamonView::getDlgItemsRelativePosition()
{
    const int dlgItemCount = 26;
    static int IDs[dlgItemCount] = {
        IDC_STATIC_URL,
        IDC_STATIC_METHOD,
        IDC_STATIC_REQUESTHEADER,
        IDC_STATIC_SENDCONTENT,
        IDC_STATIC_TIMINGGROUP,
        IDC_STATIC_TO,
        IDC_BUTTON_EDIT_COOKIE,
        IDC_BUTTON_EDIT_CONTENTTYPE,
        IDC_BUTTON_EDIT_FORM,
        IDC_BUTTON_EDIT_QUERYPARAM,
        IDC_BUTTON_EXCUTE,
        IDC_BUTTON_RESET,
        IDC_BUTTON_CLEAR,
        IDC_BUTTON_SHOWLOG,
        IDC_CHECK_REPEAT,
        IDC_CHECK_IGNOREHOLIDAY,
        IDC_CHECK_LOADHOLIDAY,
        IDC_CHECK_SENDRANDOMLY,
        IDC_COMBO_METHOD,
        IDC_COMBO_INTERVAL,
        IDC_EDIT_URL,
        IDC_EDIT_HEADERS,
        IDC_EDIT_CONTENT,
        IDC_EDIT_HOLIDAYURL,
        IDC_DATETIMEPICKER_FROM,
        IDC_DATETIMEPICKER_TO
    };
    if (m_itemRects.empty())
    {
        for (int i = 0; i <= dlgItemCount - 1; ++i)
        {
            RECT rc;
            ATL::CWindow item = GetDlgItem(IDs[i]);
            item.GetWindowRect(&rc);
            rc.right = rc.right - rc.left;      // size x
            rc.bottom = rc.bottom - rc.top;     // size y
            rc.left = rc.left - m_dlgRect.left; // relative x-position
            rc.top = rc.top - m_dlgRect.top;    // relative y-position
            m_itemRects[IDs[i]] = rc;
        }
    }
}

void CCurlDeamonView::moveItem(int itemId, int deltaX, int deltaY)
{
    int relativeX = m_itemRects[itemId].left, relativeY = m_itemRects[itemId].top;
    GetDlgItem(itemId).SetWindowPos(NULL, relativeX + deltaX, relativeY + deltaY, NULL, NULL, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void CCurlDeamonView::resizeItem(int itemId, int deltaX, int deltaY)
{
    RECT rect = m_itemRects[itemId];
    int x = rect.right + deltaX;
    int y = rect.bottom + deltaY;
    GetDlgItem(itemId).SetWindowPos(NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

LRESULT CCurlDeamonView::onDialogResize(UINT, WPARAM, LPARAM, BOOL&)
{
    RECT rc;
    GetWindowRect(&rc);

    LONG deltaX = rc.right - rc.left - (m_dlgRect.right - m_dlgRect.left);
    LONG deltaY = rc.bottom - rc.top - (m_dlgRect.bottom - m_dlgRect.top);
    LONG leftDeltaX = deltaX / 2;
    LONG rightDeltaX = deltaX - leftDeltaX;
    LONG halfDeltaY = deltaY / 2;

    // move controls
    moveItem(IDC_BUTTON_EDIT_QUERYPARAM, deltaX, 0);
    moveItem(IDC_BUTTON_EDIT_COOKIE, deltaX, 0);
    moveItem(IDC_STATIC_SENDCONTENT, 0, halfDeltaY);
    moveItem(IDC_BUTTON_EDIT_CONTENTTYPE, deltaX, 0);
    moveItem(IDC_EDIT_CONTENT, 0, halfDeltaY);
    moveItem(IDC_BUTTON_EDIT_FORM, deltaX, halfDeltaY);
    moveItem(IDC_STATIC_TIMINGGROUP, 0, deltaY);
    moveItem(IDC_CHECK_REPEAT, 0, deltaY);
    moveItem(IDC_COMBO_INTERVAL, 0, deltaY);
    moveItem(IDC_CHECK_IGNOREHOLIDAY, 0, deltaY);
    moveItem(IDC_CHECK_LOADHOLIDAY, 0, deltaY);
    moveItem(IDC_EDIT_HOLIDAYURL, 0, deltaY);
    moveItem(IDC_CHECK_SENDRANDOMLY, 0, deltaY);
    moveItem(IDC_DATETIMEPICKER_FROM, 0, deltaY);
    moveItem(IDC_STATIC_TO, 0, deltaY);
    moveItem(IDC_DATETIMEPICKER_TO, 0, deltaY);
    moveItem(IDC_BUTTON_EXCUTE, deltaX, deltaY);
    moveItem(IDC_BUTTON_RESET, deltaX, deltaY);
    moveItem(IDC_BUTTON_CLEAR, deltaX, deltaY);
    moveItem(IDC_BUTTON_SHOWLOG, deltaX, deltaY);

    // resize edit controls
    resizeItem(IDC_EDIT_URL, deltaX, 0);
    resizeItem(IDC_EDIT_HEADERS, deltaX, halfDeltaY);
    resizeItem(IDC_EDIT_CONTENT, deltaX, halfDeltaY);
    resizeItem(IDC_STATIC_TIMINGGROUP, deltaX, 0);
    resizeItem(IDC_EDIT_HOLIDAYURL, deltaX, 0);

    // force repaint dialog items
    Invalidate(true);
    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedButtonExcute(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // TODO: Add your control notification handler code here

    return 0;
}
