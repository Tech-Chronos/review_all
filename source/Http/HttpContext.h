#pragma once
#include "HttpRequest.h"
#include "../Common.hpp"
#include "../Buffer.hpp"
#include "../Any.h"
#include "util.h"
#include <regex>
#include <algorithm>
#include <ctype.h>

#define MaxLine 8192

typedef enum
{
    RECV_REQ_LINE,
    RECV_REQ_HEAD,
    RECV_REQ_BODY,
    RECV_REQ_OVER,
    RECV_REQ_ERROR
} RecvStatus;

class HttpContext
{
private:
    bool RecvReqLine(Buffer* buf);
    bool ParseReqLine(const std::string&);

    bool RecvHttpHead(Buffer* buf);
    bool ParseHttpHead(std::string&);

    bool RecvHttpBody(Buffer* buf);
public:
    HttpContext();
    int GetRespCode();
    RecvStatus GetStatus();
    HttpRequest& GetRequest();
    void RecvHttpRequest(Buffer*);
    void Reset();
private:
    int _resp_code; //响应状态码
    RecvStatus _status; // 接收到哪个部分了
    HttpRequest _req; 
};