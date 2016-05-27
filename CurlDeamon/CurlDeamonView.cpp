
#include "stdafx.h"
#include "resource.h"
#include "CurlDeamonView.h"
#include "ResponseDlg.h"

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

    // add method options
    CComboBox combo = (CComboBox)GetDlgItem(IDC_COMBO_METHOD);
    combo.InsertString(-1, L"GET");
    combo.InsertString(-1, L"POST");
    combo.InsertString(-1, L"PUT");
    combo.InsertString(-1, L"DELETE");
    combo.InsertString(-1, L"HEAD");
    combo.InsertString(-1, L"OPTIONS");
    combo.SetCurSel(static_cast<int>(_Config.http_method));

    // initialize header list
    CListViewCtrl list = (CListViewCtrl)GetDlgItem(IDC_LIST_HEADERS);
    list.InsertColumn(0, L"Header Name", LVCFMT_LEFT, 150);
    list.InsertColumn(1, L"Header Value", LVCFMT_LEFT, 250);

    CEdit editContent = (CEdit)GetDlgItem(IDC_EDIT_CONTENT);
    if (_Config.http_method == HTTP_POST || _Config.http_method == HTTP_PUT)
        editContent.EnableWindow(true);
    if (_Config.http_headers.size() > 0)
    {
        std::list<WTL::CString>::iterator iter = _Config.http_headers.begin();
        for (int i = 0; iter != _Config.http_headers.end(); ++iter, ++i)
        {
            int index = iter->Find(L':');
            WTL::CString header_name = iter->Left(index);
            header_name.TrimLeft();
            header_name.TrimRight();
            WTL::CString header_value = iter->Right(iter->GetLength() - index - 1);
            header_value.TrimLeft();
            header_value.TrimRight();
            int row = list.InsertItem(i, L"");
            list.SetItemText(row, 0, header_name);
            list.SetItemText(row, 1, header_value);
        }
    }

    GetDlgItem(IDC_DATETIMEPICKER_FROM).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_DATETIMEPICKER_TO).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_IGNOREHOLIDAY).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_LOADHOLIDAY).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(_Config.schedule_repeat_randomly && _Config.schedule_load_holiday);

    // timer pickers

    time_t t = time(0);
    struct tm *now = localtime(&t);
    SYSTEMTIME from_time = { 0 };
    from_time.wYear = now->tm_year + 1900;
    from_time.wMonth = now->tm_mon + 1;
    from_time.wDay = now->tm_mday;
    from_time.wHour = 11;
    from_time.wMinute = 0;
    from_time.wSecond = 0;
    SYSTEMTIME to_time(from_time);
    to_time.wHour = 11;
    to_time.wMinute = 30;
    to_time.wSecond = 0;
    CDateTimePickerCtrl from_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_FROM);
    CDateTimePickerCtrl to_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_TO);
    from_time_picker.SetSystemTime(GDT_VALID, &from_time);
    to_time_picker.SetSystemTime(GDT_VALID, &to_time);

    // focus on excute button
    CButton button = (CButton)GetDlgItem(IDC_BUTTON_EXCUTE);
    button.SetFocus();

    return FALSE;
}

LRESULT CCurlDeamonView::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    updateConfig();

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
        IDC_EDIT_CRONCONFIG,
        IDC_EDIT_URL,
        IDC_LIST_HEADERS,
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

void CCurlDeamonView::updateConfig()
{
    DoDataExchange(true);
}

static int writer(char *data, size_t size, size_t nmemb, std::string *writerData)
{
    if (writerData == NULL)
        return 0;

    writerData->append(data, size * nmemb);

    return size * nmemb;
}

bool CCurlDeamonView::initCurlConn(CURL *conn, char *errorBuffer, std::string *buffer, curl_slist *header_list, const char* sending_content)
{
    std::string url = CC4EncodeUTF16::convert2utf8(_Config.http_url, _Config.http_url.GetLength());

    CURLcode code;

    if (conn == NULL)
    {
        MessageBox(L"Failed to create CURL connection.");
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
    if (code != CURLE_OK)
    {
        MessageBox(L"Failed to set error buffer.");
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_URL, url.c_str());
    if (code != CURLE_OK)
    {
        MessageBox(L"Failed to set URL.");
        return false;
    }

    if (strncmp(url.c_str(), "https://", 8) == 0)
    {
        code = curl_easy_setopt(conn, CURLOPT_SSL_VERIFYPEER, 0L);
        if (code != CURLE_OK)
        {
            MessageBox(L"Failed to set SSL.");
            return false;
        }
    }

    // http header
    if (header_list != NULL)
    {
        curl_easy_setopt(conn, CURLOPT_HTTPHEADER, header_list);
    }

    // http method
    if (_Config.http_method == HTTP_GET)
        curl_easy_setopt(conn, CURLOPT_HTTPGET, 1L);
    else if (_Config.http_method == HTTP_PUT)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "PUT");
    else if (_Config.http_method == HTTP_DELETE)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "DELETE");
    else if (_Config.http_method == HTTP_HEAD)
        curl_easy_setopt(conn, CURLOPT_NOBODY, 1L);
    else if (_Config.http_method == HTTP_OPTIONS)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    if (_Config.http_method == HTTP_POST)
    {
        if (sending_content != NULL && strlen(sending_content) > 0)
        {
            curl_easy_setopt(conn, CURLOPT_POSTFIELDS, sending_content);
            curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, (long)strlen(sending_content));
        }
        else
        {
            // post without sending content
            curl_easy_setopt(conn, CURLOPT_POST, 1L);
        }
    }

    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if (code != CURLE_OK)
    {
        MessageBox(L"Failed to set redirect option.");
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
    if (code != CURLE_OK)
    {
        MessageBox(L"Failed to set writer.");
        return false;
    }

    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, buffer);
    if (code != CURLE_OK)
    {
        MessageBox(L"Failed to set write data.");
        return false;
    }

    return true;
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
    moveItem(IDC_EDIT_CRONCONFIG, 0, deltaY);
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
    resizeItem(IDC_LIST_HEADERS, deltaX, halfDeltaY);
    resizeItem(IDC_EDIT_CONTENT, deltaX, halfDeltaY);
    resizeItem(IDC_STATIC_TIMINGGROUP, deltaX, 0);
    resizeItem(IDC_EDIT_HOLIDAYURL, deltaX, 0);

    // force repaint dialog items
    Invalidate(true);
    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedButtonExcute(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    updateConfig();

    char errorBuffer[CURL_ERROR_SIZE];
    std::string buffer;

    struct curl_slist *list = NULL;
    if (_Config.http_headers.size() > 0)
    {
        std::list<WTL::CString>::iterator iter = _Config.http_headers.begin();
        for (; iter != _Config.http_headers.end(); ++iter)
            list = curl_slist_append(list, CC4EncodeUTF16::convert2utf8(*iter, iter->GetLength()).c_str());
    }
    std::string sending_content = CC4EncodeUTF16::convert2utf8(_Config.http_sending_content, _Config.http_sending_content.GetLength());

    CURL *conn = curl_easy_init();
    CURLcode code;
    if (initCurlConn(conn, errorBuffer, &buffer, list, sending_content.c_str()))
    {
        code = curl_easy_perform(conn);
    }
    if (conn != NULL)
    {
        curl_easy_cleanup(conn);
    }
    if (list != NULL)
    {
        curl_slist_free_all(list);
    }
    WTL::CString buf(buffer.c_str());
    CResponseDlg dialog(buf);
    dialog.DoModal();

    return 0;
}

LRESULT CCurlDeamonView::OnCbnSelchangeComboMethod(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CComboBox combo = (CComboBox)GetDlgItem(IDC_COMBO_METHOD);
    _Config.http_method = static_cast<HTTP_METHOD>(combo.GetCurSel());

    GetDlgItem(IDC_EDIT_CONTENT).EnableWindow(_Config.http_method == HTTP_POST || _Config.http_method == HTTP_PUT);

    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedCheckSendrandomly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CButton checkbox = (CButton)GetDlgItem(IDC_CHECK_SENDRANDOMLY);
    _Config.schedule_repeat_randomly = checkbox.GetCheck() != 0;
    GetDlgItem(IDC_DATETIMEPICKER_FROM).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_DATETIMEPICKER_TO).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_IGNOREHOLIDAY).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_LOADHOLIDAY).EnableWindow(_Config.schedule_repeat_randomly);
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(_Config.schedule_repeat_randomly && _Config.schedule_load_holiday);

    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedCheckLoadholiday(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CButton checkbox = (CButton)GetDlgItem(IDC_CHECK_LOADHOLIDAY);
    _Config.schedule_load_holiday = checkbox.GetCheck() != 0;
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(_Config.schedule_repeat_randomly && _Config.schedule_load_holiday);

    return 0;
}
