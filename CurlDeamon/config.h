#pragma once

// config.h
#ifndef CONFIG_H_
#define CONFIG_H_

#include <list>
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <tinyxml/tinyxml.h>

enum HTTP_METHOD {
    HTTP_GET     = (int) 0x00,
    HTTP_POST    = (int) 0x01,
    HTTP_PUT     = (int) 0x02,
    HTTP_DELETE  = (int) 0x03,
    HTTP_HEAD    = (int) 0x04,
    HTTP_OPTIONS = (int) 0x05
};

static const char * http_method_to_char(HTTP_METHOD method)
{
    static char * HTTP_METHOD_NAMES[6] = { "GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS" };
    return HTTP_METHOD_NAMES[static_cast<int>(method)];
};

static const wchar_t * http_method_to_wchar(HTTP_METHOD method)
{
    static wchar_t * HTTP_METHOD_NAMES_W[6] = { L"GET", L"POST", L"PUT", L"DELETE", L"HEAD", L"OPTIONS" };
    return HTTP_METHOD_NAMES_W[static_cast<int>(method)];
};

typedef struct CConfig_tag
{
    WTL::CString            http_url;
    HTTP_METHOD             http_method;
    std::list<WTL::CString> http_headers;
    WTL::CString            http_sending_content;
    BOOL                    schedule_repeat_cron_like;
    WTL::CString            schedule_cron_config;
    BOOL                    schedule_ignore_holiday;
    BOOL                    schedule_load_holiday;
    WTL::CString            schedule_holiday_url;
    BOOL                    schedule_repeat_randomly; // send randomly every day
    WTL::CString            schedule_randomly_start;
    WTL::CString            schedule_randomly_end;
}CConfig;

BOOL load_config_file(TiXmlDocument *xmlfile, CConfig &config);
BOOL save_config_file(LPCTSTR config_path, const CConfig &config);
void set_default(CConfig &config);

#endif // CONFIG_H_