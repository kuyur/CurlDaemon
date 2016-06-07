
#include "stdafx.h"
#include "resource.h"
#include "CurlDeamonView.h"
#include "ResponseDlg.h"
#include "../common/win32helper.h"
#include "../common/winfile.h"

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
    combo.SetCurSel(static_cast<int>(m_config.http_method));

    // initialize header list
    CListViewCtrl list = (CListViewCtrl)GetDlgItem(IDC_LIST_HEADERS);
    list.InsertColumn(0, L"Header Name", LVCFMT_LEFT, 150);
    list.InsertColumn(1, L"Header Value", LVCFMT_LEFT, 250);

    CEdit editContent = (CEdit)GetDlgItem(IDC_EDIT_CONTENT);
    if (m_config.http_method == HTTP_POST || m_config.http_method == HTTP_PUT)
        editContent.EnableWindow(true);
    if (m_config.http_headers.size() > 0)
    {
        std::list<WTL::CString>::iterator iter = m_config.http_headers.begin();
        for (int i = 0; iter != m_config.http_headers.end(); ++iter, ++i)
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

    GetDlgItem(IDC_DATETIMEPICKER_FROM).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_DATETIMEPICKER_TO).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_IGNOREHOLIDAY).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_LOADHOLIDAY).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(m_config.schedule_repeat_randomly && m_config.schedule_load_holiday);

    int hour, minute, second = 0;
    // timer pickers
    time_t t = time(0);
    struct tm *now = localtime(&t);
    SYSTEMTIME from_time = { 0 };
    from_time.wYear = now->tm_year + 1900;
    from_time.wMonth = now->tm_mon + 1;
    from_time.wDay = now->tm_mday;
    swscanf(m_config.schedule_randomly_start, L"%d:%d:%d", &hour, &minute, &second);
    from_time.wHour = hour;
    from_time.wMinute = minute;
    from_time.wSecond = second;
    SYSTEMTIME to_time(from_time);
    swscanf(m_config.schedule_randomly_end, L"%d:%d:%d", &hour, &minute, &second);
    to_time.wHour = hour;
    to_time.wMinute = minute;
    to_time.wSecond = second;
    CDateTimePickerCtrl from_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_FROM);
    CDateTimePickerCtrl to_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_TO);
    from_time_picker.SetSystemTime(GDT_VALID, &from_time);
    to_time_picker.SetSystemTime(GDT_VALID, &to_time);

    // focus on execute button
    CButton button = (CButton)GetDlgItem(IDC_BUTTON_EXCUTE);
    button.SetFocus();

    // create schedule job
    if (_Config.schedule_repeat_cron_like)
        makeCronSchedule();
    else if (_Config.schedule_repeat_randomly)
        makeRandomSchedule();

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
    const int dlgItemCount = 27;
    static int IDs[dlgItemCount] = {
        IDC_STATIC_URL,
        IDC_STATIC_METHOD,
        IDC_STATIC_REQUESTHEADER,
        IDC_STATIC_SENDCONTENT,
        IDC_STATIC_TIMINGGROUP,
        IDC_STATIC_TO,
        IDC_STATIC_NEXTEXECUTION,
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
    _Config = m_config;
}

void CCurlDeamonView::execute(const CConfig &config, bool scheduled)
{
    // get sending timestamp
    time_t t = time(0);
    struct tm *now = localtime(&t);
    wchar_t ts_buffer[256] = { 0 };
    wcsftime(ts_buffer, sizeof(ts_buffer), L"%F %T %z", now);

    char errorBuffer[CURL_ERROR_SIZE];
    std::string buffer;

    struct curl_slist *list = NULL;
    if (config.http_headers.size() > 0)
    {
        std::list<WTL::CString>::const_iterator iter = config.http_headers.begin();
        for (; iter != config.http_headers.end(); ++iter)
            list = curl_slist_append(list, CC4EncodeUTF16::convert2utf8(*iter, iter->GetLength()).c_str());
    }
    std::string sending_content = CC4EncodeUTF16::convert2utf8(config.http_sending_content, config.http_sending_content.GetLength());

    CURL *conn = curl_easy_init();
    CURLcode code;
    if (initCurlConn(conn, errorBuffer, &buffer, config.http_method, config.http_url, list, sending_content.c_str()))
        code = curl_easy_perform(conn);
    if (list != NULL)
        curl_slist_free_all(list);

    // [Timer][2016-05-30 20:40:28 +0900] "GET http://example.com/" 200
    WTL::CString log = scheduled ? L"[Timer][" : L"[";
    log += ts_buffer;
    log += L"] \"";
    log += http_method_to_wchar(config.http_method);
    log += L" ";
    log += config.http_url;
    log += L"\" ";

    if (code != CURLE_OK)
    {
        curl_easy_cleanup(conn);
        // error happen
        log += curl_easy_strerror(code);
        log += "\n";
    }
    else
    {
        // parse response header
        long response_code = 0;
        curl_easy_getinfo(conn, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_cleanup(conn);

        log.Append(response_code);
        log += L"\n";
    }
    m_logs.push_back(log);
    // append to log file
    WTL::CString log_file_path(GetProcessFolder());
    log_file_path += L"curldeamon.log";
    CWinFile file(log_file_path, CWinFile::modeWrite | CWinFile::shareDenyWrite | CWinFile::openAppend);
    if (file.open())
    {
        std::string utf8_log = CC4EncodeUTF16::convert2utf8(log, log.GetLength());
        file.write(utf8_log.c_str(), utf8_log.length());
        file.close();
    }

    if (!scheduled)
    {
        if (code != CURLE_OK)
        {
            WTL::CString msg = L"Failed to perform ";
            msg += http_method_to_wchar(config.http_method);
            msg += " request for ";
            msg += config.http_url;
            msg += L". ";
            msg += curl_easy_strerror(code);
            msg += L".";
            MessageBox(msg);
        }
        else
        {
            WTL::CString buf(buffer.c_str());
            CResponseDlg dialog(buf);
            dialog.DoModal();
        }
    }
}

static int writer(char *data, size_t size, size_t nmemb, std::string *writerData)
{
    if (writerData == NULL)
        return 0;

    writerData->append(data, size * nmemb);

    return size * nmemb;
}

bool CCurlDeamonView::initCurlConn(CURL *conn, char *errorBuffer, std::string *buffer, HTTP_METHOD http_method, WTL::CString http_url, curl_slist *header_list, const char* sending_content)
{
    std::string url = CC4EncodeUTF16::convert2utf8(http_url, http_url.GetLength());

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
    if (http_method == HTTP_GET)
        curl_easy_setopt(conn, CURLOPT_HTTPGET, 1L);
    else if (http_method == HTTP_PUT)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "PUT");
    else if (http_method == HTTP_DELETE)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "DELETE");
    else if (http_method == HTTP_HEAD)
        curl_easy_setopt(conn, CURLOPT_NOBODY, 1L);
    else if (http_method == HTTP_OPTIONS)
        curl_easy_setopt(conn, CURLOPT_CUSTOMREQUEST, "OPTIONS");

    if (http_method == HTTP_POST)
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

void CCurlDeamonView::makeCronSchedule()
{
    // TODO
}

void CCurlDeamonView::makeRandomSchedule()
{
    time_t current = time(0);
    int hour, minute, second = 0;

    time_t start = current;
    struct tm *ts = localtime(&start);
    swscanf(_Config.schedule_randomly_start, L"%d:%d:%d", &hour, &minute, &second);
    ts->tm_hour = hour;
    ts->tm_min = minute;
    ts->tm_sec = second;
    start = mktime(ts);
    if (start < current)
        start += 24 * 3600;
    // if flag of ignore holiday is true, for time range which is crossing a day, for example
    // 23:30:00 - 00:30:00, schedule will be carried out in time range of:
    // Monday    23:30:00 - Tuesday   00:30:00
    // Tuesday   23:30:00 - Wednesday 00:30:00
    // Wednesday 23:30:00 - Thursday  00:30:00
    // Thursday  23:30:00 - Friday    00:30:00
    // Friday    23:30:00 - Saturday  00:30:00
    if (_Config.schedule_ignore_holiday)
    {
        // get weekday
        struct tm *local_start = localtime(&start);
        // jump to Monday
        if (local_start->tm_wday == 0)
            start += 24 * 3600;
        else if (local_start->tm_wday == 6)
            start += 2 * 24 * 3600;
    }

    time_t end = start;
    ts = localtime(&end);
    swscanf(_Config.schedule_randomly_end, L"%d:%d:%d", &hour, &minute, &second);
    ts->tm_hour = hour;
    ts->tm_min = minute;
    ts->tm_sec = second;
    end = mktime(ts);
    if (end < start)
        end += 24 * 3600;

    // determine next execute time
    m_next_execution = (rand() % (end - start + 1)) + start;

    // update static text
    ts = localtime(&m_next_execution);
    wchar_t ts_buffer[256] = { 0 };
    wcsftime(ts_buffer, sizeof(ts_buffer), L"%F %T %z", ts);
    WTL::CString str = L"Next: ";
    str += ts_buffer;
    GetDlgItem(IDC_STATIC_NEXTEXECUTION).SetWindowText(str);

    // recreate timer (check every one second)
    SetTimer(1, 1000);
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
    moveItem(IDC_STATIC_NEXTEXECUTION, 0, deltaY);
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

    if (_Config.schedule_repeat_cron_like)
        makeCronSchedule();
    else if (_Config.schedule_repeat_randomly)
        makeRandomSchedule();
    else
    {
        KillTimer(1);
        m_next_execution = -1;
        GetDlgItem(IDC_STATIC_NEXTEXECUTION).SetWindowText(L"");
        execute(_Config, false);
    }

    return 0;
}

LRESULT CCurlDeamonView::OnCbnSelchangeComboMethod(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CComboBox combo = (CComboBox)GetDlgItem(IDC_COMBO_METHOD);
    m_config.http_method = static_cast<HTTP_METHOD>(combo.GetCurSel());

    GetDlgItem(IDC_EDIT_CONTENT).EnableWindow(m_config.http_method == HTTP_POST || m_config.http_method == HTTP_PUT);

    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedCheckSendrandomly(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CButton checkbox = (CButton)GetDlgItem(IDC_CHECK_SENDRANDOMLY);
    m_config.schedule_repeat_randomly = checkbox.GetCheck() != 0;
    GetDlgItem(IDC_DATETIMEPICKER_FROM).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_DATETIMEPICKER_TO).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_IGNOREHOLIDAY).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_CHECK_LOADHOLIDAY).EnableWindow(m_config.schedule_repeat_randomly);
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(m_config.schedule_repeat_randomly && m_config.schedule_load_holiday);

    return 0;
}

LRESULT CCurlDeamonView::OnBnClickedCheckLoadholiday(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CButton checkbox = (CButton)GetDlgItem(IDC_CHECK_LOADHOLIDAY);
    m_config.schedule_load_holiday = checkbox.GetCheck() != 0;
    GetDlgItem(IDC_EDIT_HOLIDAYURL).EnableWindow(m_config.schedule_repeat_randomly && m_config.schedule_load_holiday);

    return 0;
}

LRESULT CCurlDeamonView::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (m_next_execution <= 0)
        return 0;

    // check hitting next execution time or not
    time_t now = time(0);
    if (now < m_next_execution)
        return 0;

    KillTimer(1);
    execute(_Config, true);

    if (_Config.schedule_repeat_cron_like)
        makeCronSchedule();
    else if (_Config.schedule_repeat_randomly)
        makeRandomSchedule();

    return 0;
}

LRESULT CCurlDeamonView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    KillTimer(1);

    return 0;
}


LRESULT CCurlDeamonView::OnDtnDatetimechangeDatetimepickerFrom(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    CDateTimePickerCtrl from_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_FROM);
    SYSTEMTIME from_time = { 0 };
    from_time_picker.GetSystemTime(&from_time);
    struct tm t = { 0 };
    t.tm_year = from_time.wYear - 1900;
    t.tm_mon = from_time.wMonth - 1;
    t.tm_mday = from_time.wDay;
    t.tm_hour = from_time.wHour;
    t.tm_min = from_time.wMinute;
    t.tm_sec = from_time.wSecond;
    wchar_t buffer[10] = { 0 };
    wcsftime(buffer, 10, L"%H:%M:%S", &t);
    m_config.schedule_randomly_start = buffer;

    return 0;
}


LRESULT CCurlDeamonView::OnDtnDatetimechangeDatetimepickerTo(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    CDateTimePickerCtrl to_time_picker = (CDateTimePickerCtrl)GetDlgItem(IDC_DATETIMEPICKER_TO);
    SYSTEMTIME to_time = { 0 };
    to_time_picker.GetSystemTime(&to_time);
    struct tm t = { 0 };
    t.tm_year = to_time.wYear - 1900;
    t.tm_mon = to_time.wMonth - 1;
    t.tm_mday = to_time.wDay;
    t.tm_hour = to_time.wHour;
    t.tm_min = to_time.wMinute;
    t.tm_sec = to_time.wSecond;
    wchar_t buffer[10] = { 0 };
    wcsftime(buffer, 10, L"%H:%M:%S", &t);
    m_config.schedule_randomly_end = buffer;

    return 0;
}
