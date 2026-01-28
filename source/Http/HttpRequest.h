#pragma once
#include <string>
#include <unordered_map>
#include <regex>

class HttpRequest
{
public: 
    void SetHeaders(const std::string& key, const std::string& val);
    bool HasHeaders(const std::string& key);
    std::string GetHeaders(const std::string& key);

    void SetParams(const std::string& key, const std::string& val);
    bool HasParams(const std::string& key);
    std::string GetParams(const std::string& key);

    int ContentLength();
    bool IsKeepAlive();

    void Reset();

public:
    std::string _method;  // 请求方法
    std::string _uri;     // 请求额的uri
    std::string _version; // http 版本

    std::smatch _matches; 

    std::unordered_map<std::string, std::string> _headers; // 请求头部
    std::unordered_map<std::string, std::string> _params; // 查询字符串
};