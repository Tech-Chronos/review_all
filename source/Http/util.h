#pragma once
#include "../EchoServer.h"
#include <fstream>
#include <unordered_map>
#include <sys/stat.h>

class Util
{
private:    
    static char HexToDecimal(char hex);
public:
    // 字符串分割函数
    static int Split(const std::string& src, const std::string& sep, std::vector<std::string>* arr);

    // 读取文件内容
    static bool ReadFile(const std::string& filename, std::string* buf);

    // 写文件
    static bool WriteFile(const std::string& filename, std::string* buf);

    // url encode
    static std::string UrlEncode(const std::string& url, bool space_to_plus);

    // url decode
    static std::string UrlDecode(const std::string& url, bool plus_to_space);

    // 设置状态码描述
    static std::string GetStatusDesc(int code);

    // 获取文件后缀
    static std::string GetMime(const std::string& filename);

    // 判断是否是目录
    static bool IsDirectory(const std::string& filename);

    // 判断是否是普通文件
    static bool IsRegular(const std::string& filename);

    // 判断是否是合法路径
    static bool IsValidPath(const std::string& path);
};