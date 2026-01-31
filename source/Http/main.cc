#include "util.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"

// int main()
// {
//     // std::string str = "abc,,,b,.,,.cvbf,fda,";
//     // std::string sep = ",";
//     // std::vector<std::string> arr;
//     // Util tool;
//     // tool.Split(str, sep , &arr);
//     // //Util::Split(str, sep , &arr);
//     // for (auto& e : arr)
//     // {
//     //     std::cout << e << std::endl;
//     // }

//     // std::string str;
//     // std::string filename = "../Connection.cc";
//     // Util::ReadFile(filename, &str);
//     // std::cout << str << std::endl;

//     // Util::WriteFile("test.txt", &str);
//     std::string str = Util::UrlEncode("password=C  ", false);
//     std::cout << str << std::endl;
//     std::string str2 = Util::UrlDecode(str, false);
//     std::cout << str2 << std::endl;
// 

// int main()
// {
//     std::string desc(Util::GetStatusDesc(500));
    
//     std::cout << desc << std::endl;

//     std::string suffix(Util::GetMime("a.b.wasm"));
//     std::cout << suffix << std::endl;

//     std::cout << Util::IsValidPath("/html/hello/../../../Channel.cc") << std::endl;
// }
std::string RequestStr(const HttpRequest& req)
{
    std::stringstream req_str;
    req_str << req._method << " " << req._uri << " " << req._version << "\r\n";

    for (auto& it : req._headers)
    {
        req_str << it.first << ": " << it.second << "\r\n";
    }

    for (auto& it : req._params)
    {
        req_str << it.first << ": " << it.second << "\r\n";
    }
    req_str << "\r\n";
    req_str << req._body;
    return req_str.str();
}

void RegisterGetMethod(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

void RegisterPutMethod(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

void RegisterPostMethod(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

void RegisterDeleteMethod(const HttpRequest& req, HttpResponse* resp)
{
    resp->SetContent(RequestStr(req), "text/plain");
}

int main()
{
    HttpServer server(8888);
    server.SetWebRoot("./wwwroot");

    server.SetThreadCount(3);

    server.Get("/hello", RegisterGetMethod);

    server.Delete("/123.txt", RegisterDeleteMethod);

    server.Post("/login", RegisterPostMethod);

    server.Put("/890.txt", RegisterPutMethod);

    server.Listen();
    return 0;
}