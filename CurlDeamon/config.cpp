#include "config.h"
#include <c4lib/c4encode.h>
#include "../common/winfile.h"

BOOL load_config_file(TiXmlDocument * xmlfile, CConfig & config)
{
    TiXmlHandle hRoot(xmlfile);
    TiXmlElement *pElem;
    TiXmlHandle hXmlHandler(0);

    // config node
    pElem = hRoot.FirstChildElement().Element();
    if (!pElem) return FALSE;
    if (strcmp(pElem->Value(), "config") != 0)
        return FALSE;

    hXmlHandler = TiXmlHandle(pElem);
    // http-url node
    pElem = hXmlHandler.FirstChild("http-url").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.http_url = CC4EncodeUTF8::convert2unicode(pElem->GetText(), strlen(pElem->GetText())).c_str();

    // http-method node
    pElem = hXmlHandler.FirstChild("http-method").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
    {
        if (_stricmp(pElem->GetText(), "GET") == 0)
            config.http_method = HTTP_GET;
        else if (_stricmp(pElem->GetText(), "POST") == 0)
            config.http_method = HTTP_POST;
        else if (_stricmp(pElem->GetText(), "PUT") == 0)
            config.http_method = HTTP_PUT;
        else if (_stricmp(pElem->GetText(), "DELETE") == 0)
            config.http_method = HTTP_DELETE;
        else if (_stricmp(pElem->GetText(), "HEAD") == 0)
            config.http_method = HTTP_HEAD;
        else if (_stricmp(pElem->GetText(), "OPTIONS") == 0)
            config.http_method = HTTP_OPTIONS;
        else
            config.http_method = HTTP_GET;
    }

    // http-sending-content node
    pElem = hXmlHandler.FirstChild("http-sending-content").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.http_sending_content = CC4EncodeUTF8::convert2unicode(pElem->GetText(), strlen(pElem->GetText())).c_str();

    // schedule-repeat-type node
    pElem = hXmlHandler.FirstChild("schedule-repeat-type").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
    {
        config.schedule_repeat_cron_like = _stricmp(pElem->GetText(), "cron-like") == 0;
        config.schedule_repeat_randomly = _stricmp(pElem->GetText(), "randomly") == 0;
    }

    // schedule-ignore-holiday node
    pElem = hXmlHandler.FirstChild("schedule-ignore-holiday").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.schedule_ignore_holiday = _stricmp(pElem->GetText(), "true") == 0;

    // schedule-load-holiday node
    pElem = hXmlHandler.FirstChild("schedule-load-holiday").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.schedule_load_holiday = _stricmp(pElem->GetText(), "true") == 0;

    // schedule-holiday-url node
    pElem = hXmlHandler.FirstChild("schedule-holiday-url").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.schedule_holiday_url = CC4EncodeUTF8::convert2unicode(pElem->GetText(), strlen(pElem->GetText())).c_str();

    // schedule-randomly-start node
    pElem = hXmlHandler.FirstChild("schedule-randomly-start").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.schedule_randomly_start = CC4EncodeUTF8::convert2unicode(pElem->GetText(), strlen(pElem->GetText())).c_str();

    // schedule-randomly-end node
    pElem = hXmlHandler.FirstChild("schedule-randomly-end").Element();
    if (!pElem) return FALSE;
    if (pElem->GetText())
        config.schedule_randomly_end = CC4EncodeUTF8::convert2unicode(pElem->GetText(), strlen(pElem->GetText())).c_str();

    return TRUE;
}

BOOL save_config_file(LPCTSTR config_path, const CConfig & config)
{
    TiXmlDocument config_file;
    TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "utf-8", "");
    TiXmlElement *configure = new TiXmlElement("config");

    TiXmlElement *http_url = new TiXmlElement("http-url");
    TiXmlText *http_url_value = new TiXmlText(CC4EncodeUTF16::convert2utf8(config.http_url, config.http_url.GetLength()).c_str());
    http_url->LinkEndChild(http_url_value);
    configure->LinkEndChild(http_url);

    TiXmlElement *http_method = new TiXmlElement("http-method");
    TiXmlText *http_method_value;
    switch (config.http_method)
    {
    case HTTP_GET:
        http_method_value = new TiXmlText("GET");
        break;
    case HTTP_POST:
        http_method_value = new TiXmlText("POST");
        break;
    case HTTP_PUT:
        http_method_value = new TiXmlText("PUT");
        break;
    case HTTP_DELETE:
        http_method_value = new TiXmlText("DELETE");
        break;
    case HTTP_HEAD:
        http_method_value = new TiXmlText("HEAD");
        break;
    case HTTP_OPTIONS:
        http_method_value = new TiXmlText("OPTIONS");
        break;
    default:
        http_method_value = new TiXmlText("GET");
    }
    http_method->LinkEndChild(http_method_value);
    configure->LinkEndChild(http_method);

    TiXmlElement *http_sending_content = new TiXmlElement("http-sending-content");
    TiXmlText *http_sending_content_value = new TiXmlText(CC4EncodeUTF16::convert2utf8(config.http_sending_content, config.http_sending_content.GetLength()).c_str());
    http_sending_content->LinkEndChild(http_sending_content_value);
    configure->LinkEndChild(http_sending_content);

    TiXmlElement *schedule_repeat_type = new TiXmlElement("schedule-repeat-type");
    TiXmlText *schedule_repeat_type_value = new TiXmlText(config.schedule_repeat_cron_like ? "cron-like" : config.schedule_repeat_randomly ? "randomly" : "none");
    schedule_repeat_type->LinkEndChild(schedule_repeat_type_value);
    configure->LinkEndChild(schedule_repeat_type);

    TiXmlElement *schedule_ignore_holiday = new TiXmlElement("schedule-ignore-holiday");
    TiXmlText *schedule_ignore_holiday_value = new TiXmlText(config.schedule_ignore_holiday ? "true" : "false");
    schedule_ignore_holiday->LinkEndChild(schedule_ignore_holiday_value);
    configure->LinkEndChild(schedule_ignore_holiday);

    TiXmlElement *schedule_load_holiday = new TiXmlElement("schedule-load-holiday");
    TiXmlText *schedule_load_holiday_value = new TiXmlText(config.schedule_load_holiday ? "true" : "false");
    schedule_load_holiday->LinkEndChild(schedule_load_holiday_value);
    configure->LinkEndChild(schedule_load_holiday);

    TiXmlElement *schedule_holiday_url = new TiXmlElement("schedule-holiday-url");
    TiXmlText *schedule_holiday_url_value = new TiXmlText(CC4EncodeUTF16::convert2utf8(config.schedule_holiday_url, config.schedule_holiday_url.GetLength()).c_str());
    schedule_holiday_url->LinkEndChild(schedule_holiday_url_value);
    configure->LinkEndChild(schedule_holiday_url);

    TiXmlElement *schedule_randomly_start = new TiXmlElement("schedule-randomly-start");
    TiXmlText *schedule_randomly_start_value = new TiXmlText(CC4EncodeUTF16::convert2utf8(config.schedule_randomly_start, config.schedule_randomly_start.GetLength()).c_str());
    schedule_randomly_start->LinkEndChild(schedule_randomly_start_value);
    configure->LinkEndChild(schedule_randomly_start);

    TiXmlElement *schedule_randomly_end = new TiXmlElement("schedule-randomly-end");
    TiXmlText *schedule_randomly_end_value = new TiXmlText(CC4EncodeUTF16::convert2utf8(config.schedule_randomly_end, config.schedule_randomly_end.GetLength()).c_str());
    schedule_randomly_end->LinkEndChild(schedule_randomly_end_value);
    configure->LinkEndChild(schedule_randomly_end);

    config_file.LinkEndChild(dec);
    config_file.LinkEndChild(configure);

    TiXmlPrinter printer;
    config_file.Accept(&printer);
    CWinFile file(config_path, CWinFile::modeWrite | CWinFile::openCreateAlways | CWinFile::shareExclusive);
    if (file.open())
    {
        file.write(CC4Encode::UTF_8_BOM, 3);
        file.write(printer.CStr(), strlen(printer.CStr()));
        file.close();
    }

    return TRUE;
}

void set_default(CConfig & config)
{
    config.http_url = L"";
    config.http_method = HTTP_GET;
    config.http_headers.clear();
    config.http_sending_content = L"";
    config.schedule_repeat_cron_like = FALSE;
    config.schedule_repeat_randomly = FALSE;
    config.schedule_cron_config = L"";
    config.schedule_ignore_holiday = FALSE;
    config.schedule_load_holiday = FALSE;
    config.schedule_holiday_url = L"";
    config.schedule_randomly_start = L"11:00:00";
    config.schedule_randomly_end = L"11:30:00";
}
