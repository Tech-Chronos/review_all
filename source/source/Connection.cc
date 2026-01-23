#include "Connection.h"

////////////// public /////////////////
Connection::Connection(uint64_t conn_id, int fd, EventLoop *loop)
    : _conn_id(conn_id), _fd(fd), _socket(_fd), _loop(loop), _channel(_fd, _loop), _enable_inactive_release(false), _status(CONNECTING)
{
    // 设置channel回调函数
    _channel.SetReadCb(std::bind(&Connection::HandleRead, this));
    _channel.SetWriteCb(std::bind(&Connection::HandleWrite, this));
    _channel.SetErrorCb(std::bind(&Connection::HandleError, this));
    _channel.SetCloseCb(std::bind(&Connection::HandleClose, this));
    _channel.SetAnyCb(std::bind(&Connection::HandleAnyEvent, this));
}

Connection::~Connection()
{
    DBG_LOG("Release Connection : %p", this);
}

int Connection::GetFd()
{
    return _fd;
}

int Connection::GetConnId()
{
    return _conn_id;
}

/// @brief 设置协议类型
void Connection::SetContext(const Any &context)
{
    _context = context;
}

Any *Connection::GetContext()
{
    return &_context;
}

/// @brief 组件使用者设置的4个回调函数
void Connection::SetConnectedCallback(const ConnectedCallBack &conn_cb)
{
    _conn_cb = conn_cb;
}

void Connection::SetMessageDealCallback(const MessageCallBack &message_cb)
{
    _message_cb = message_cb;
}

void Connection::SetCloseCallback(const CloseCallBack &close_cb)
{
    _close_cb = close_cb;
}

void Connection::SetServerCloseCallback(const CloseCallBack &server_close_cb)
{
    _server_close_cb = server_close_cb;
}

void Connection::SetAnyEventCallback(const AnyEventCallBack &any_cb)
{
    _any_cb = any_cb;
}

////////////// private /////////////////
/// @brief 处理读事件
void Connection::HandleRead()
{
    // 1. 读取数据
    char buf[65535] = {0};
    ssize_t ret = _socket.RecvNonBlock(buf, sizeof(buf));
    // 错误读取不是真正的关闭连接
    if (ret < 0)
    {
        ShutDownInLoop();
        return;
    }
    // 2. 将接收到的数据放到缓冲区中
    _in_buffer.WriteAndPush(buf, ret);

    // 3. 业务处理
    if (_in_buffer.GetReadableDataNum() > 0)
        if (_message_cb)
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
    // 2. 如果还有其他要执行的，在此执行，由用户给定
    if (_any_cb)
        _any_cb(shared_from_this());
}

void Connection::Established()
{
    _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop, this));
}

/// @brief 这个只是把Send函数放到任务队列，可能还没等到执行，就释放了data
void Connection::Send(char *data, size_t len)
{
    Buffer buf;
    buf.WriteAndPush(data, len);
    _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, buf));
}

void Connection::ShutDown()
{
    _loop->RunInLoop(std::bind(&Connection::ShutDownInLoop, this));
}

void Connection::Release()
{
    _loop->RunInLoop(std::bind(&Connection::ReleaseInLoop, this));
}

void Connection::AddInactiveEventRelease(int sec)
{
    _loop->RunInLoop(std::bind(&Connection::AddInactiveEventReleaseInLoop, this, sec));
}

void Connection::CancelInactiveEventRelease()
{
    _loop->RunInLoop(std::bind(&Connection::CancelInactiveEventReleaseInLoop, this));
}

// 必须保证这个任务在自己的eventloop中立即执行，因为加入业务线程执行，把这个任务放到任务队列中，但是eventloop还没来得及处理
// 此时又突然来了一个事件，那就会造成这个新来的事件会拿着旧的协议进行处理，就会出错
// 所以必须在自己的eventloop中立即处理这个函数，不能放到业务线程处理
void Connection::Upgrade(const Any &context, const MessageCallBack &message_cb,
                         const AnyEventCallBack &any_cb, const ConnectedCallBack &connect_cb,
                         const CloseCallBack &close_cb)
{
    // 这里直接端严即可
    _loop->AssertInLoop();
    _loop->RunInLoop(std::bind(&Connection::UpgradeContext, this, context, message_cb,
                               any_cb, connect_cb, close_cb));
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
    if (_close_cb)
        _close_cb(shared_from_this());

    // 告诉TCP SERVER，把这条链接从链接表中移除
    if (_server_close_cb)
        _server_close_cb(shared_from_this());
}

/// @brief 并不是真正的发送数据，只是把数据放到outbuffer中：业务完成后的数据data
void Connection::SendInLoop(Buffer buf)
{
    if (_status == DISCONNECTED)
        return;
    // 1. 插入数据
    _out_buffer.WriteBufferAndPush(buf);
    // 2. 启动读事件
    if (!_channel.Writable())
        _channel.EnableWrite();
}

/// @brief 并不是真正的关闭，得先处理两个缓冲区中的数据
void Connection::ShutDownInLoop()
{
    // 1. 设置状态
    _status = DISCONNECTING;

    // 2. 处理读缓冲区中的数据
    // 可能缓冲区中的数据并不是完整的，但是也没关系，处理一次不管成功与否都不再继续了
    if (_in_buffer.GetReadableDataNum() > 0)
    {
        if (_message_cb)
            _message_cb(shared_from_this(), &_in_buffer);
    }

    // 3. 处理写缓冲区中的数据
    if (_out_buffer.GetReadableDataNum() > 0)
    {
        if (!_channel.Writable())
            _channel.EnableWrite();
    }

    // 4. 判断输出缓冲区是否是空了，并且关闭链接
    if (_out_buffer.GetReadableDataNum() == 0)
    {
        ReleaseInLoop();
    }
}

/// @brief 在eventloop中新增非活跃事件的销毁任务
void Connection::AddInactiveEventReleaseInLoop(int sec)
{
    _enable_inactive_release = true;
    // 如果已经存在，就更新定时任务，没有就插入定时任务
    if (_loop->HasTimer(_conn_id))
        _loop->RefreshTimer(_conn_id);
    else
        _loop->AddTimer(_conn_id, sec, std::bind(&Connection::ReleaseInLoop, this));
}

/// @brief 在eventloop中取消非活跃事件的释放
void Connection::CancelInactiveEventReleaseInLoop()
{
    _enable_inactive_release = false;
    // 定时任务必须存在
    if (_loop->HasTimer(_conn_id))
        _loop->CancelTimer(_conn_id);
}

/// @brief 更新上层协议
void Connection::UpgradeContext(const Any &context, const MessageCallBack &message_cb,
                                const AnyEventCallBack &any_cb, const ConnectedCallBack &connect_cb,
                                const CloseCallBack &close_cb)
{
    _context = context;
    _message_cb = message_cb;
    _any_cb = any_cb;
    _conn_cb = connect_cb;
    _close_cb = close_cb;
}
