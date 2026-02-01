#include "HttpServer.h"

void HttpServer::MakeResponse(const std::shared_ptr<Connection>& con, const HttpRequest &req, HttpResponse *resp)
{
    // 1. 设置头部字段
    // 长 or 短 链接
    if (resp->IsKeepAlive())
    {
        resp->SetHeaders("Connection", "keep-alive");
    }
    else
    {
        resp->SetHeaders("Connection", "close");
    }
    // 正文长度
    if (!resp->_body.empty() && !resp->HasHeaders("Content-Length"))
    {
        resp->SetHeaders("Content-Length", std::to_string(resp->_body.size()));
    }
    // 正文类型
    if (!resp->_body.empty() && !resp->HasHeaders("Content-Type"))
    {
        resp->SetHeaders("Content-Type", "application/octet-stream");
    }
    // 重定向
    if (resp->_redirect_flag)
    {
        resp->SetHeaders("Location", resp->_redirect_uri);
    }
    // 2. 构建响应
    std::stringstream resp_str;

    resp_str << req._version << " " << resp->_code << " " << Util::GetStatusDesc(resp->_code) << "\r\n";

    for (auto &e : resp->_headers)
    {
        resp_str << e.first << ": " << e.second << "\r\n";
    }
    resp_str << "\r\n";

    resp_str << resp->_body;

    //INF_LOG("%s",resp_str.str().c_str());

    // 在conn中发送数据
    con->Send(resp_str.str().c_str(), resp_str.str().size());
}

// 返回请求错误的页面（正文部分）
void HttpServer::ErrorHandler(const HttpRequest &req, HttpResponse *resp)
{
    std::string body;
    body += "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'> \
            <title>Document</title></head><body>";
    body += std::to_string(resp->_code);
    body += ": ";
    body += Util::GetStatusDesc(resp->_code);
    body += "</body></html>";

    resp->SetContent(body, "text/html");
}

void HttpServer::Dispatcher(HttpRequest &req, HttpResponse *resp, const std::vector<std::pair<std::regex, HttpReqHandle>> &route)
{
    for (auto &handle : route)
    {
        std::regex pattern = handle.first;
        bool ret = std::regex_match(req._uri, req._matches, pattern);
        if (ret == false)
            continue;
        if (ret == true)
        {
            handle.second(req, resp);
            return;
        }
    }
    resp->_code = 404;
}

bool HttpServer::IsFileHandler(HttpRequest &req)
{
    // 1. 首先判断方法是否是 head or get
    if (req._method != "GET" && req._method != "HEAD")
    {
        INF_LOG("req.method error!");
        return false;
    }
        
    // 2. 是否设置了网络根目录
    if (_www_root.empty())
    {
        INF_LOG("_www_root.empty() error!");
        return false;
    }
        
    // 3. 判断路径是否是合法的
    if (!Util::IsValidPath(req._uri))
    {
        INF_LOG("Util::IsValidPath(req._uri) error");
        return false;
    }
        
    // 4. 判断请求的是目录还是普通文件，如果是目录，要判断这个目录加上index.html，如果是普通文件，直接判断是否存在
    std::string req_uri = _www_root + req._uri;
    if (req_uri.back() == '/')
    {
        req_uri += "index.html";
    }

    // 5. 判断处理好的文件名是否是普通文件
    // 如果不是普通文件，或者此文件不存在就返回，执行动态处理函数
    bool ret = Util::IsRegular(req_uri);
    if (!ret)
    {
        INF_LOG("Util::IsRegular(req_uri) error");
        INF_LOG("filename : %s", req_uri.c_str());
        return false;
    }
    // 如果是普通文件，返回true，并且把我这个文件的uri重新设置一下
    req._uri = req_uri;
    //INF_LOG("uri:%s",req._uri.c_str());
    return true;
}

// 判断完成之后如果确实是静态资源请求，则可以进行文件的读取操作，放到resp的body中
void HttpServer::FileHandler(HttpRequest &req, HttpResponse *resp)
{
    bool ret = Util::ReadFile(req._uri, &resp->_body);
    if (ret == false)
    {
        ERR_LOG("READ FILE ERROR!");
        return;
    }
    std::string mime = Util::GetMime(req._uri);
    resp->SetHeaders("Content-Type", mime);
}

void HttpServer::Route(HttpRequest &req, HttpResponse *resp)
{
    // 1. 先判断是否是静态资源请求
    //INF_LOG("IsFileHandler(req): %d, req.uri = %s", IsFileHandler(req), req._uri.c_str());
    if (IsFileHandler(req))
    {
        //INF_LOG("uri: %s, Is File Handler.", req._uri.c_str());
        return FileHandler(req, resp);
    }
    // 2.如果不是静态资源请求，就要看是不是动态的请求
    if (req._method == "GET" || req._method == "HEAD")
    {
        Dispatcher(req, resp, _get_route);
    }
    else if (req._method == "POST")
    {
        Dispatcher(req, resp, _post_route);
    }
    else if (req._method == "PUT")
    {
        Dispatcher(req, resp, _put_route);
    }
    else if (req._method == "DELETE")
    {
        Dispatcher(req, resp, _delete_route);
    }
    else
    {
        resp->_code = 405;
    }
}

void HttpServer::OnConnected(const std::shared_ptr<Connection>& con)
{
    con->SetContext(HttpContext());
}

void HttpServer::OnMessage(const std::shared_ptr<Connection>& con, Buffer *buf)
{
    while (true)
    {
        // 1. 获取上下文
        HttpContext *context = con->GetContext()->GetValAddr<HttpContext>();
        if (context == nullptr) 
        {
            ERR_LOG("context get error");
            return ;
        }
        // 2. 处理buf中的数据放到HttpRequest
        context->RecvHttpRequest(buf);

        // 构造函数的时候要给响应码
        HttpResponse resp(context->GetRespCode());

        // 获取req
        HttpRequest req = context->GetRequest();

        // 2.1 如果处理的时候 RespCode 大于 400，说明发生错误，要给客户端发送错误响应
        if (context->GetRespCode() >= 400)
        {
            ERR_LOG("Client Request Error");
            ErrorHandler(req, &resp);
            MakeResponse(con, req, &resp);
            con->ShutDown();
            return;
        }

        // 2.2 当上下文没有处理完成的时候，要继续接受，不能立刻构造resp
        if (context->GetStatus() != RECV_REQ_OVER)
        {
            // ]
            DBG_LOG("Server Need To Go On Receiving ...");
            return;
        }

        // 3. 将上下文处理完成的req发送给路由表，进行处理请求
        Route(req, &resp);

        // 4. 构造响应
        MakeResponse(con, req, &resp);

        // 5. 此次请求处理完毕，清空上下文
        context->Reset();

        // 6. 判断是否是长链接
        if (!req.IsKeepAlive())
            con->ShutDown();
    }
}


/// 公有方法
HttpServer::HttpServer(int port, int timeout)
    : _tcp_server(port)
{
    _tcp_server.EnableIncativeEventRelease(timeout);
    _tcp_server.SetConnectionCallback(std::bind(&HttpServer::OnConnected, this, std::placeholders::_1));
    _tcp_server.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::SetWebRoot(const std::string &wwwroot)
{
    _www_root = wwwroot;
}

void HttpServer::SetThreadCount(int thread_count)
{
    _tcp_server.SetThreadCountAndCreate(thread_count);
}

void HttpServer::Get(const std::string &pattern, const HttpReqHandle &handle)
{
    _get_route.emplace_back(std::make_pair(std::regex(pattern), handle));
}

void HttpServer::Put(const std::string &pattern, const HttpReqHandle &handle)
{
    _put_route.emplace_back(std::make_pair(std::regex(pattern), handle));
}

void HttpServer::Post(const std::string &pattern, const HttpReqHandle &handle)
{
    _post_route.emplace_back(std::make_pair(std::regex(pattern), handle));
}

void HttpServer::Delete(const std::string &pattern, const HttpReqHandle &handle)
{
    _delete_route.emplace_back(std::make_pair(std::regex(pattern), handle));
}

void HttpServer::Listen()
{
    _tcp_server.AcceptorStart();
}