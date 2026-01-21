#include "Connection.h"

Connection::Connection(EventLoop* loop, MessageCallBack message_cb, 
                       AnyEventCallBack any_cb, ConnectedCallBack conn_cb,
                       CloseCallBack close_cb, CloseCallBack server_close_cb)
    : _loop(loop)
    , _channel(_fd, _loop)
    , _enable_inactive_release(false)
    , _message_cb(message_cb)
    , _any_cb(any_cb)
    , _conn_cb(conn_cb)
    , _close_cb(close_cb)
    , _server_close_cb(server_close_cb)
{
    srand(time(nullptr));
    _conn_id = rand() % 1024;
}

/// @brief 处理读事件
void Connection::HandleRead()
{
    // 1. 读取数据
    char buf[65535] = {0};
    ssize_t ret = _socket.RecvNonBlock(buf, sizeof(buf));
    // 错误读取不是真正的关闭连接
    if (ret < 0)
    {
        ShutDown();
        return;
    }
    // 2. 将接收到的数据放到缓冲区中
    _in_buffer.WriteAndPush(buf, ret);

    // 3. 业务处理
    if (_in_buffer.GetReadableDataNum() > 0)
        _message_cb(shared_from_this(), &_in_buffer); // 获取当前对象的智能指针
}

/// @brief 处理写事件
void Connection::HandleWrite()
{
    // 1. 将发送缓冲区中的数据发送
    ssize_t ret = _socket.SendNonBlock(_out_buffer.GetReadPos(), _out_buffer.GetReadableDataNum());

    // 2. 处理Send返回值
    if (ret < 0)
    {
        // 出错了不能立即返回，要判断接收缓冲区中是否还有数据没进行处理，要先处理
        if (_in_buffer.GetReadableDataNum() > 0)
            _message_cb(shared_from_this(), &_in_buffer);

        // 处理完之后直接退出，这个是真实的退出
        return ReleaseInLoop();
    }
    // 要移动指针
    _out_buffer.MoveReadOffset(ret);

    // 3. 没有数据要发送了，要关闭这个文件描述符的写事件
    if (_out_buffer.GetReadableDataNum() == 0)
    {
        _channel.DisableWrite();

        // 如果缓冲区中没有数据了，并且当前的状态是待关闭状态，则关闭当前的连接
        if (_status == DISCONNECTING)
            return ReleaseInLoop();
    }
    return;
}

/// @brief 处理关闭事件
void Connection::HandleClose()
{
    if (_in_buffer.GetReadableDataNum())
    {
        _message_cb(shared_from_this(), &_in_buffer);
    }
    return ReleaseInLoop();
}

/// @brief 处理错误事件
void Connection::HandleError()
{
    return HandleClose();
}

/// @brief 处理任意事件
void Connection::HandleAnyEvent()
{
    // 1. 刷新定时任务，判断是否开启定时销毁非活跃链接
    if (_enable_inactive_release == true)
    {
        _loop->RefreshTimer(_conn_id);
    }
    // 2. 如果还有其他要执行的，在此执行
    if (_any_cb)
        _any_cb(shared_from_this());
}

/// @brief 获取链接之后，新链接所出的状态要进行设置
void Connection::EstablishedInLoop()
{
    // 必须要是半链接状态
    assert(_status == CONNECTING);
    // 1. 修改链接状态为已链接
    _status = CONNECTED;

    // 2. 启动读事件监控，不能在构造函数的时候启动，因为还没有设置任意事件的回调函数
    //    例如刷新活跃度
    // channel的回调函数在构造函数之前就已经设置好了，所以这里就不用担心了
    _channel.EnableRead();

    // 3. 建立完成之后，要调用 connected_cb，这个函数用来 确定协议 或者 给客户端发送欢迎 或者 启动非活跃事件监测 的函数
    _conn_cb(shared_from_this()); // 常见的就是 addtimer
}

/// @brief 真正关闭链接的函数
void Connection::ReleaseInLoop()
{
    // 1. 将状态设置为关闭
    _status = DISCONNECTED;

    // 2. 底层不再关心这个文件描述符的事件，移除
    _channel.Remove();

    // 3. 关闭掉这个文件描述符
    _socket.Close();

    // 4. 如果eventloop中还有这个链接的定时器任务，就需要cancel
    if (_loop->HasTimer(_conn_id))
        CancelInactiveEventReleaseInLoop();

    // 5. 调用关闭的回调函数
    // 客户端的回调函数是通知用户的，我已经和你断开链接了
    // 避免先移除服务器管理的连接信息导致Connection被释放，
    // 再去处理会出错，因此先调用用户回调
    _close_cb(shared_from_this());

    // 告诉TCP SERVER，把这条链接从链接表中移除
    _server_close_cb(shared_from_this());
}