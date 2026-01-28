#include "util.h"

// "abc,b,cvbf,fda"
int Util::Split(const std::string &src, const std::string &sep, std::vector<std::string> *arr)
{
    int offset = 0;
    while (offset < src.size())
    {
        int pos = src.find(sep, offset);
        if (pos == std::string::npos)
        {
            arr->push_back(src.substr(offset));
            break;
        }
        else
        {
            if (offset == pos)
            {
                offset = pos + sep.size();
                continue;
            }
            arr->push_back(src.substr(offset, pos - offset));
            offset = pos + sep.size();
        }
    }
    return arr->size();
}

bool Util::ReadFile(const std::string &filename, std::string *buf)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs)
    {
        ERR_LOG("OPEN FILE %s ERROR!", filename.c_str());
        return false;
    }
    ifs.seekg(0, ifs.end);
    int filesize = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    buf->resize(filesize);

    ifs.read(&(*buf)[0], filesize);
    if (!ifs)
    {
        ERR_LOG("READ FILE %s ERROR", filename.c_str());
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}

bool Util::WriteFile(const std::string &filename, std::string *buf)
{
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    if (!ofs)
    {
        ERR_LOG("OPEN FILE %s ERROR!", filename.c_str());
        return false;
    }
    ofs.write(buf->c_str(), buf->size());
    if (ofs.good() == false)
    {
        ERR_LOG("WRITE FILE %s ERROR!", filename.c_str());
        ofs.close();
        return false;
    }
    ofs.close();
    return true;
}

std::string Util::UrlEncode(const std::string &url, bool space_to_plus)
{
    std::string res;
    for (auto &e : url)
    {
        // 如果是 . - _ ~，或者是数字、字母就不用转化
        if (e == '.' || e == '-' || e == '_' || e == '~' || isalnum(e))
        {
            res += e;
            continue;
        }
        // 如果需要将空格转化成 + 号
        if (e == ' ' && space_to_plus)
        {
            res += '+';
            continue;
        }

        // 剩下的都要转化
        char tmp[4] = {0};
        snprintf(tmp, 4, "%%%02X", e);
        res += tmp;
    }
    return res;
}

char Util::HexToDecimal(char hex)
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    if (hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;
    if (hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    return -1;
}

std::string Util::UrlDecode(const std::string &url, bool plus_to_space)
{
    std::string ret;
    for (int i = 0; i < url.size(); ++i)
    {
        if (url[i] == '%' && i + 2 < url.size())
        {
            char left = HexToDecimal(url[i + 1]);
            char right = HexToDecimal(url[i + 2]);
            if (left >= 0 && right >= 0)
            {
                char character = (left << 4) + right;
                ret += character;
                i += 2;
            }
        }
        else if (url[i] == '+' && plus_to_space)
        {
            ret += ' ';
        }
        else
        {
            ret += url[i];
        }
    }
    return ret;
}

std::string Util::GetStatusDesc(int code)
{
    std::unordered_map<int, std::string> kHttpStatusText =
    {
        // 1xx
        {100, "Continue"},
        {101, "Switching Protocols"},
        {102, "Processing"},
        {103, "Early Hints"},

        // 2xx
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {207, "Multi-Status"},
        {208, "Already Reported"},
        {226, "IM Used"},

        // 3xx
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},

        // 4xx
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a Teapot"},
        {421, "Misdirected Request"},
        {422, "Unprocessable Entity"},
        {423, "Locked"},
        {424, "Failed Dependency"},
        {425, "Too Early"},
        {426, "Upgrade Required"},
        {428, "Precondition Required"},
        {429, "Too Many Requests"},
        {431, "Request Header Fields Too Large"},
        {451, "Unavailable For Legal Reasons"},

        // 5xx
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"},
        {506, "Variant Also Negotiates"},
        {507, "Insufficient Storage"},
        {508, "Loop Detected"},
        {510, "Not Extended"},
        {511, "Network Authentication Required"}
    };
    auto it = kHttpStatusText.find(code);
    if (it == kHttpStatusText.end()) return "Unknown Code";
    return it->second;
}

std::string Util::GetMime(const std::string& filename)
{
    std::unordered_map<std::string, std::string> kMimeTypes = 
    {
        // text
        {".html", "text/html"},
        {".htm",  "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        {".mjs",  "application/javascript"},
        {".txt",  "text/plain"},
        {".csv",  "text/csv"},
        {".xml",  "application/xml"},
        {".json", "application/json"},
        {".md",   "text/markdown"},

        // images
        {".png",  "image/png"},
        {".jpg",  "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif",  "image/gif"},
        {".bmp",  "image/bmp"},
        {".webp", "image/webp"},
        {".svg",  "image/svg+xml"},
        {".ico",  "image/x-icon"},

        // fonts
        {".woff",  "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf",   "font/ttf"},
        {".otf",   "font/otf"},

        // audio
        {".mp3",  "audio/mpeg"},
        {".wav",  "audio/wav"},
        {".ogg",  "audio/ogg"},
        {".flac", "audio/flac"},
        {".aac",  "audio/aac"},

        // video
        {".mp4",  "video/mp4"},
        {".webm", "video/webm"},
        {".avi",  "video/x-msvideo"},
        {".mov",  "video/quicktime"},
        {".mkv",  "video/x-matroska"},

        // archives
        {".zip",  "application/zip"},
        {".tar",  "application/x-tar"},
        {".gz",   "application/gzip"},
        {".7z",   "application/x-7z-compressed"},
        {".rar",  "application/vnd.rar"},

        // binary / misc
        {".pdf",  "application/pdf"},
        {".wasm", "application/wasm"},
        {".bin",  "application/octet-stream"},
        {".exe",  "application/octet-stream"}
    };
    size_t pos = filename.rfind(".");
    if (pos == std::string::npos) return "Unkwon Suffix";

    std::string suffix = filename.substr(pos);

    auto it = kMimeTypes.find(suffix);
    if (it == kMimeTypes.end()) return "Unkwon Suffix";
    else return it->second;
}


// 判断是否是目录
bool Util::IsDirectory(const std::string& filename)
{
    struct stat st;
    int ret = stat(filename.c_str(), &st);

    if (ret < 0) return false;
    return S_ISDIR(st.st_mode);
}

// 判断是否是普通文件
bool Util::IsRegular(const std::string& filename)
{
    struct stat st;
    int ret = stat(filename.c_str(), &st);

    if (ret < 0) return false;
    return S_ISREG(st.st_mode); // mode 是文件类型 + 权限
}

bool Util::IsValidPath(const std::string& path)
{
    std::vector<std::string> subdir;
    Split(path, "/", &subdir);
    int level = 0;
    for (auto& e : subdir)
    {
        if (e == "..")
        {
            --level;
            if (level < 0) return false;
        }
        else if (e == ".")
        {
            continue;
        }
        else
        {
            ++level;
        }
    }
    return true;
}

