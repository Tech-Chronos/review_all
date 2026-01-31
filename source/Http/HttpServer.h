#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "../TcpServer.h"

#include <vector>
#include <regex>

#define DEFAULT_TIME 30

using HttpReqHandle = std::function<void(const HttpRequest&, HttpResponse*)>;

class HttpServer
{
private:
    void MakeResponse(const std::shared_ptr<Connection>& con, const HttpRequest&, HttpResponse*);
    void ErrorHandler(const HttpRequest& req, HttpResponse* resp);
    void Dispatcher(HttpRequest&, HttpResponse*, const std::vector<std::pair<std::regex, HttpReqHandle>>& route);

    bool IsFileHandler(HttpRequest &req);
    void FileHandler(HttpRequest &req, HttpResponse* resp);

    void Route(HttpRequest& req, HttpResponse* resp);
    void OnConnected(const std::shared_ptr<Connection>& con);
    void OnMessage(const std::shared_ptr<Connection>& con, Buffer* buf);

public:
    HttpServer(int port, int timeout = DEFAULT_TIME);
    
    void SetWebRoot(const std::string& wwwroot);
    void SetThreadCount(int thread_count);

    void Get(const std::string& pattern, const HttpReqHandle& handle);
    void Put(const std::string& pattern, const HttpReqHandle& handle);
    void Post(const std::string& pattern, const HttpReqHandle& handle);
    void Delete(const std::string& pattern, const HttpReqHandle& handle);

    void Listen();

private:
    std::vector<std::pair<std::regex, HttpReqHandle>> _get_route;
    std::vector<std::pair<std::regex, HttpReqHandle>> _put_route;
    std::vector<std::pair<std::regex, HttpReqHandle>> _post_route;
    std::vector<std::pair<std::regex, HttpReqHandle>> _delete_route;
    TcpServer _tcp_server;
    std::string _www_root; // 网络资源根目录
};  