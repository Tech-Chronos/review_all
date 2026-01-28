#include "HttpRequest.h"

void HttpRequest::SetHeaders(const std::string& key, const std::string& val)
{
    _headers.insert(std::make_pair(key, val));
}

bool HttpRequest::HasHeaders(const std::string& key)
{
    auto it = _headers.find(key);
    if (it == _headers.end()) return false;
    return true;
}

std::string HttpRequest::GetHeaders(const std::string& key)
{
    if (HasHeaders(key))
    {
        return _headers[key];
    }
    return "";
}

void HttpRequest::SetParams(const std::string& key, const std::string& val)
{
    _params.insert(std::make_pair(key, val));
}

bool HttpRequest::HasParams(const std::string& key)
{
    auto it = _params.find(key);
    if (it == _params.end()) return false;
    return true;
}

std::string HttpRequest::GetParams(const std::string& key)
{
    if (HasParams(key))
    {
        return _params[key];
    }
    return "";
}

int HttpRequest::ContentLength()
{
    if (HasHeaders("Content-Length"))
    {
        std::string ret = GetHeaders("Content-Length");
        return std::stoi(ret);
    }
    return 0;
}

bool HttpRequest::IsKeepAlive()
{
    if (HasHeaders("Connection") && GetHeaders("Connection") == "keep-alive")
        return true;
    return false;
}

void HttpRequest::Reset()
{
    _method.clear();
    _version.clear();
    _uri.clear();
    _headers.clear();
    _params.clear();

    std::smatch match;
    _matches.swap(match);
}

