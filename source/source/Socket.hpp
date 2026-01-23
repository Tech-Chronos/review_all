#pragma once
#include "Common.hpp"

#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

class Socket
{
public:
    Socket() :_sockfd(-1) {}
    Socket(int fd) :_sockfd(fd) {}
    ~Socket() 
    {
        Close();
    }

    // 1. 创建套接字
    bool CreateSocket()
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sockfd < 0)
        {
            ERR_LOG("CreateSocket Error!");
            return false;
        }
        INF_LOG("Create Socket Success, sockfd = %d", _sockfd);
        return true;
    }

    // 2. 绑定ip和端口
    bool Bind(uint16_t port, std::string ip = "0.0.0.0")
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        socklen_t len = sizeof(addr);
        int ret = bind(_sockfd, (sockaddr*)&addr, len);
        if (ret < 0) 
        {
            ERR_LOG("Bind Error!");
            return false;
        }
        return true;
    }

    // 3. 服务器监听套接字
    bool Listen(int backlog = 3)
    {
        int ret = listen(_sockfd, backlog);
        if (ret < 0)
        {
            ERR_LOG("Listen Error!");
            return false;
        }
        INF_LOG("Server Is Listening!");
        return true;
    }

    // 4. 客户端链接服务器
    bool Connection(uint16_t port, std::string ip)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        socklen_t len = sizeof(addr);
        int ret = connect(_sockfd, (sockaddr*)&addr, len);
        if (ret < 0)
        {
            ERR_LOG("Connection Error!");
            return false;
        }
        INF_LOG("Connection Success!");
        return true;
    }

    // 5. 服务器接收客户端
    int Accept()
    {
        int con_fd = accept(_sockfd, nullptr, nullptr);
        if (con_fd < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
            {
                INF_LOG("no accept!");
                return -2;
            }
            ERR_LOG("Accept Error!");
            return -1;
        }
        INF_LOG("Client Connect!");
        SetNonBlock(con_fd);
        return con_fd;
    }

    // 6. 设置文件描述符非阻塞
    void SetNonBlock(int sockfd)
    {
        int ret = fcntl(sockfd, F_GETFL);
        if (ret < 0)
        {
            ERR_LOG("SetNonBlock Error!");
            return;
        }
        fcntl(sockfd, F_SETFL, ret | O_NONBLOCK);
    }

    // 7. 端口复用
    void ReuseAddr()
    {
        int opt = 1;
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    }

    // 8. 服务器初始化
    bool ServerInit(uint16_t port, std::string ip = "0.0.0.0")
    {
        if (!CreateSocket()) return false;
        SetNonBlock(_sockfd);
        ReuseAddr();
        if (!Bind(port, ip)) return false;
        if (!Listen()) return false;
        return true;
    }

    //9. 客户端初始化
    bool ClientInit(uint16_t port, std::string ip)
    {
        if (!CreateSocket()) return false;
        if (!Connection(port, ip)) return false;
        return true;
    }

    // 读取
    ssize_t Recv(char* buffer, size_t size, int flag)
    {
        ssize_t ret = recv(_sockfd, buffer, size - 1, flag);
        if (ret < 0)
        {
            if (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
            {
                return 0;
            }
            return -1;
        }
        else if (ret == 0)
        {
            INF_LOG("client quit!");
            return ret;
        }
        else 
        {
            buffer[ret] = 0;
        }
        return ret;
    }

    ssize_t RecvNonBlock(char* buffer, size_t size)
    {
        ssize_t ret = Recv(buffer, size, MSG_DONTWAIT);
        return ret;
    }

    // 发送
    ssize_t Send(const char* data, size_t size, int flag)
    {
        ssize_t ret = send(_sockfd, data, size, 0);
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                return 0;
            ERR_LOG("send error!");
            return ret;
        }
        else 
        {
            return ret;
        }
    }

    ssize_t SendNonBlock(const char* data, size_t size)
    {
        ssize_t ret = Send(data, size, MSG_DONTWAIT);
        return ret;
    }

    // 10. 关闭文件描述符
    void Close()
    {
        if (_sockfd >= 0)
        {
            close(_sockfd);
            _sockfd = -1;
        }
            
    }

    int Fd() const { return _sockfd; }
private:
    int _sockfd;
};