//
// Created by 有趣的中国人 on 2025/11/26.
//
#ifndef REVIEW_HEAP_HEAP_H
#define REVIEW_HEAP_HEAP_H
#include <iostream>
#include <vector>

// 最基础的堆，暂时不需要用容器适配器和仿函数
template<class T>
class Heap
{
private:
    void AdjustUp()
    {
        int child = _size;
        int parent = (child - 1) / 2;

        while (child > 0)
        {
            if (_arr[child] < _arr[parent])
            {
                std::swap(_arr[child], _arr[parent]);
                child = parent;
                parent = (parent - 1) / 2;
            }
            else
            {
                break;
            }
        }
    }

    void AdjustDown()
    {
        int parent = 0;
        int child = (parent * 2) + 1;

        while (child < _size)
        {
            if (child + 1 < _size && _arr[child] > _arr[child + 1])
            {
                child++;
            }

            if (_arr[parent] > _arr[child])
            {
                std::swap(_arr[child], _arr[parent]);
                parent = child;
                child = (child * 2) + 1;
            }
            else
            {
                break;
            }
        }
    }

public:
    Heap(int capacity = 4)
        :_size(0)
        ,_capacity(capacity)
    {
        _arr = new T[_capacity];
    }

    void Insert(const T& val)
    {
        if (_size == _capacity)
        {
            // 方法1
//            int newcapacity = 2 * _capacity;
//            Heap<T> newhp(newcapacity);
//
//            for (int i = 0; i < _size; ++i)
//            {
//                newhp.Insert(_arr[i]);
//            }
//
//            std::swap(_arr, newhp._arr);
//            _capacity = newcapacity;

            // 方法2
            int newcapacity = 2 * _capacity;

            T* tmp = new T[newcapacity];

            for (int i = 0; i < _size; ++i)
            {
                tmp[i] = _arr[i];
            }

            delete[] _arr;
            _arr = tmp;
            _capacity = newcapacity;
        }

        _arr[_size] = val;
        AdjustUp();

        ++_size;
    }

    T erase()
    {
        std::swap(_arr[0], _arr[_size - 1]);
        T ret = _arr[_size - 1];

        --_size;

        AdjustDown();
        return ret;
    }

    int size()
    {
        return _size;
    }

    ~Heap()
    {
        delete[] _arr;
        _size = _capacity = 0;
    }

    void Print()
    {
        for (int i = 0; i < _size; ++i)
        {
            std::cout << _arr[i] << " ";
        }
        std::cout << std::endl;
    }
private:
    T* _arr;
    int _size;
    int _capacity;
};

#endif //REVIEW_HEAP_HEAP_H
