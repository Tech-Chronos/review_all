#include "Common.hpp"
#include "Buffer.hpp"
#include "Socket.hpp"
#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"
#include "TimerWheel.h"
#include "Connection.h"
#include "Acceptor.h"
#include <unordered_map>

std::unordered_map<uint64_t, std::shared_ptr<Connection>> conns;
std::vector<ThreadLoop> threads(2);
EventLoop base_thread;
int next_loop = 1;

void ConnCb(std::shared_ptr<Connection> conn)
{
    conns.insert(make_pair(conn->GetConnId(), conn));
}

void ServerCloseCb(std::shared_ptr<Connection> conn)
{
    conns.erase(conn->GetConnId());
}

void MessageDealCb(std::shared_ptr<Connection> conn, Buffer* buf)
{
    INF_LOG("%s", buf->GetReadPos());
    buf->MoveReadOffset(buf->GetReadableDataNum());
}

void AcceptHelper(EventLoop* loop, int io_fd)
{
    static int con_id = 1;
    EventLoop* next = threads[next_loop].GetEventLoopPtr();
    std::shared_ptr<Connection> new_con = std::make_shared<Connection>(con_id, io_fd, next);
    new_con->SetConnectedCallback(std::bind(ConnCb, std::placeholders::_1));
    new_con->SetServerCloseCallback(std::bind(ServerCloseCb, std::placeholders::_1));
    new_con->SetMessageDealCallback(std::bind(MessageDealCb, std::placeholders::_1, std::placeholders::_2));
    new_con->Established();

    new_con->AddInactiveEventRelease(5);
    ++con_id;
    next_loop = (next_loop + 1 ) % 2;
}

int main()
{
    //EventLoop loop;
    Acceptor acc(8888, &base_thread);
    acc.SetAcceptCallback(std::bind(AcceptHelper, &base_thread, std::placeholders::_1));
    acc.Listen();

    while (true)
        base_thread.Start();
}
