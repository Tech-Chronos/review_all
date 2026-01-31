#include "HttpContext.h"

HttpContext::HttpContext()
    : _resp_code(200), _status(RECV_REQ_LINE)
{
}

// 拿到请求行（首行）
bool HttpContext::RecvReqLine(Buffer *buf)
{
    if (_status != RECV_REQ_LINE)
        return false;
    // 没有找到 \r\n 一整行数据
    std::string req_line = buf->GetLineAndPop();
    //DBG_LOG("req_line=[%s]", req_line.c_str());
    if (req_line.empty())
    {
        if (buf->GetReadableDataNum() > MaxLine)
        {
            _resp_code = 414;
            _status = RECV_REQ_ERROR;
            return false;
        }
        return true;
    }
    // 有一整行数据，但是太长了
    if (req_line.size() > MaxLine)
    {
        _resp_code = 414;
        _status = RECV_REQ_ERROR;
        return false;
    }
    bool ret = ParseReqLine(req_line);
    if (ret == false)
        return false;

    _status = RECV_REQ_HEAD;
    return true;
}

bool HttpContext::ParseReqLine(const std::string& req_line)
{
    // 用正则表达式分割请求行：请求方法、请求资源、查询字符串、http版本
    std::smatch matches;
    std::regex pattern("(GET|PUT|POST|OPTIONS|DELETE|PATCH|HEAD)\\s+([^\\s\\?]+)(?:\\?([^\\s]*))?\\s+(HTTP/1\\.[01])\\r?\\n", std::regex::icase);
    //std::regex pattern("(GET|PUT|POST|OPTIONS|DELETE|PATCH|HEAD) ([^\\s\\?]+)(?:\\?([^\\s]*)? (HTTP/1\\.[01])(?:\n|\r\n)", std::regex::icase);
    //(GET|PUT|POST|OPTIONS|DELETE|PATCH|HEAD)\s+([^\s\?]+)(?:\?([^\s]*))?\s+(HTTP/1\.[01])\r?\n
    bool ret = std::regex_match(req_line, matches, pattern);
    if (ret == false)
    {
        ERR_LOG("regex_match error!");
        _resp_code = 400;
        _status = RECV_REQ_ERROR;
        return false;
    }

    _req._method = matches[1];
    transform(_req._method.begin(), _req._method.end(), _req._method.begin(), ::toupper);
    // 不需要将 "+ -> 空格"
    _req._uri = Util::UrlDecode(matches[2], false);
    _req._version = matches[4];

    // 设置http的查询字符串
    std::string params = matches[3];
    std::vector<std::string> params_arr;
    Util::Split(params, "&", &params_arr);
    for (auto &e : params_arr)
    {
        auto pos = e.find("=");
        if (pos == std::string::npos)
        {
            _resp_code = 400;
            _status = RECV_REQ_ERROR;
            return false;
        }
        std::string key = Util::UrlDecode(e.substr(0, pos), true);
        std::string val = Util::UrlDecode(e.substr(pos + 1), true);

        // 查询字符串解码的时候要将 "空格 -> + "
        _req.SetParams(key, val);
    }
    return true;
}

bool HttpContext::RecvHttpHead(Buffer *buf)
{
    if (_status != RECV_REQ_HEAD)
        return false;
    while (true)
    {
        std::string line = buf->GetLineAndPop();
        if (line.empty())
        {
            if (buf->GetReadableDataNum() > MaxLine)
            {
                _status = RECV_REQ_ERROR;
                _resp_code = 414;
                return false;
            }
            return true;
        }
        if (line.size() > MaxLine)
        {
            _status = RECV_REQ_ERROR;
            _resp_code = 414;
            return false;
        }

        // 遇到换行，则退出
        if (line == "\r\n" || line == "\n")
        {
            _status = RECV_REQ_BODY;
            return true;
        }

        bool ret = ParseHttpHead(line);
        if (ret == false)
        {
            _status = RECV_REQ_ERROR;
            _resp_code = 400;
            return false;
        }
        // _status = RECV_REQ_BODY;
        // return true;
    }
    return true;
}

bool HttpContext::ParseHttpHead(const std::string& head)
{
    auto pos = head.find(": ");
    if (pos == std::string::npos)
    {
        _status = RECV_REQ_ERROR;
        return false;
    }
    std::string key = head.substr(0, pos);
    std::string val = head.substr(pos + 2);
    _req.SetHeaders(key, val);

    // for (auto& e : _req._headers)
    // {
    //     std::cout << e.first << ": " << e.second << std::endl;
    // }
    return true;
}

bool HttpContext::RecvHttpBody(Buffer* buf)
{
    if (_status != RECV_REQ_BODY) return false; 

    int content_length = _req.ContentLength();
    if (content_length == 0)
    {
        INF_LOG("RECV_REQ_OVER");
        _status = RECV_REQ_OVER;
        return true;
    }
    int cur_length = _req._body.size();
    int need_length = content_length - cur_length;

    int buf_size = buf->GetReadableDataNum();

    if (need_length <= buf_size)
    {
        _req._body.append(buf->GetReadPos(), need_length);
        buf->MoveReadOffset(need_length);
        _status = RECV_REQ_OVER;
    }
    else
    {
        _req._body.append(buf->GetReadPos(), buf_size);
        buf->MoveReadOffset(buf_size);
    }
    return true;
}


int HttpContext::GetRespCode()
{
    return _resp_code;
}

RecvStatus HttpContext::GetStatus()
{
    return _status;
}

HttpRequest& HttpContext::GetRequest()
{
    return _req;
}

void HttpContext::RecvHttpRequest(Buffer* buf)
{
    // 不用break
    switch(_status)
    {
        case RECV_REQ_LINE : RecvReqLine(buf);
        case RECV_REQ_HEAD : RecvHttpHead(buf);
        case RECV_REQ_BODY : RecvHttpBody(buf);
    }
    return;
}

void HttpContext::Reset()
{
    _resp_code = 200;
    _status = RECV_REQ_LINE;
    _req.Reset();
}

