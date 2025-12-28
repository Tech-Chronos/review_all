#ifndef __BUFFER_HPP__

#define __BUFFER_HPP__
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>

#define BUFFER_SIZE 1024

class Buffer
{
public:
    Buffer()
        : _buffer(BUFFER_SIZE), _writer_idx(0), _reader_idx(0)
    {
    }

    // 1. 获取写位置偏移量
    char *GetWritePos()
    {
        return &_buffer[0] + _writer_idx;
    }

    // 2. 获取读位置偏移量
    char *GetReadPos()
    {
        return &_buffer[0] + _reader_idx;
    }

    // 3. 获取前沿空间大小
    uint64_t GetFrontSpaceNum()
    {
        return _buffer.size() - _writer_idx;
    }

    // 4. 获取后沿空间大小
    uint64_t GetBackSpaceNum()
    {
        return _reader_idx;
    }

    // 5. 获取可读数据区域大小
    size_t GetReadableDataNum()
    {
        return _writer_idx - _reader_idx;
    }

    // 6. 将读偏移向后移动
    void MoveReadOffset(size_t len)
    {
        // 必须保证可读区域要大于len
        assert(len <= GetReadableDataNum());
        _reader_idx += len;
    }

    // 7. 确保有充足的写空间
    void EnsureAmpleSpace(int len)
    {
        // 前沿空间直接够
        if (len <= GetFrontSpaceNum())
        {
            return;
        }
        // 剩余空间够，前移
        else if (len <= GetFrontSpaceNum() + GetBackSpaceNum())
        {
            // 首先获取读的起始位置
            std::vector<char>::iterator it_begin = _buffer.begin() + _reader_idx;
            // 将整部分的可读空间copy到开头
            std::copy(it_begin, it_begin + GetReadableDataNum(), _buffer.begin());
            // 更新读写偏移量
            _reader_idx = 0;
            _writer_idx = GetReadableDataNum();
            return;
        }
        // 剩余空间不够，扩容
        else
        {
            _buffer.resize(len + _writer_idx);
            return;
        }
    }

    // 8. 将写偏移向后移动
    void MoveWriteOffset(size_t len)
    {
        // 只要保证前沿的空间够，就可以向后移动了，这里肯定可以保证，因为已经ensure了
        assert(len <= GetFrontSpaceNum());
        _writer_idx += len;
    }

    // 9. 写入
    void Write(const void* data, size_t len)
    {
        EnsureAmpleSpace(len);
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, GetWritePos());
    }

    void WriteString(const std::string& data)
    {
        Write(data.c_str(), data.size());
    }

    void WriteBuffer(Buffer& data)
    {
        Write(data.GetReadPos(), data.GetReadableDataNum());
    }

    // 10. 读取
    void Read(void* buf, size_t len)
    {
        assert(len < GetReadableDataNum());
        std::copy(GetReadPos(), GetReadPos() + GetReadableDataNum(), (char*)buf);
    }

    std::string ReadString(size_t len)
    {
        std::string str;
        str.resize(GetReadableDataNum());
        Read(&str[0], len);
        return str;
    }

    std::string ReadStringAndPop(size_t len)
    {
        ReadString(len);
        MoveReadOffset(len);
    }

    void Clear()
    {
        _writer_idx = _reader_idx = 0;
    }
private:
    std::vector<char> _buffer;
    uint64_t _writer_idx;
    uint64_t _reader_idx;
};

#endif
