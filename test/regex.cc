#include <iostream>
#include <string>
#include <regex>

int main()
{
    // GET /index.html?user=zhangsan&passwd=123456 HTTP/1.1\r\n;
    std::string req_header = "GET /login.html/a/b/c?user=zhangsan&passwd=123456 HTTP/1.1\r\n";

    // GET /login.html HTTP/1.0\r\n
    //std::string req_header = "GET /login.html HTTP/1.0\r\n";
    std::smatch matches;

    // std::regex pattern("(GET|POST|DELETE|PUT|OPTIONS|PATCH|HEAD)\\s(?:[\\?])?([^\\s]+)?\\s(HTTP/1\\.[01])(?:\n|\r\n)");
    // std::regex pattern("(GET|POST|PUT|DELETE|HEAD) ([^\\?]+)(?:\\?)?([^\\&]+)?(?:\\&)?([^\\s]+)? (HTTP/1\\.[01])(?:\n|\r\n)");
    //std::regex pattern("(GET|POST|PUT|DELETE|OPTIONS|PATCH|HEAD) ([^\\?]+)(?:[\\?])?([^\\s]+) (HTTP/1\\.[01])(?:\n|\r\n)");
    //std::regex pattern("(GET|PUT|POST|OPTIONS|DELETE|PATCH|HEAD) ([^\\?]+)(?:[\\?])?([^\\s]+) (HTTP/1\\.[01])(?:\n|\r\n))", std::regex::icase);
    std::regex pattern("(GET|PUT|POST|OPTIONS|DELETE|PATCH|HEAD)\\s+([^\\s\\?]+)(?:\\?([^\\s]*))?\\s+(HTTP/1\\.[01])\\r?\\n", std::regex::icase);
    
    bool ret = std::regex_match(req_header, matches, pattern);

    if (ret == false)
    {
        std::cout << "regex_match false"<< std::endl;
        return -1;
    }

    for (int i = 0; i < matches.size(); ++i)
    {
        std::cout << i << ": " << matches[i] << std::endl;
    }

    return 0;
}

//std::regex pattern("(GET|POST|PUT|DELETE|HEAD) ([^\\?]+)(?:\\?)?([^\\&]+)?(?:\\&)?([^\\s]+)? (HTTP/1\\.[01])(?:\n|\r\n)");
