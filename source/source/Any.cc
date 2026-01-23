#include "Any.h"

// PlaceHolder
template <class T>
Any::PlaceHolder<T>::PlaceHolder(const T &val)
    : _val(val)
{
}

template <class T>
const std::type_info &Any::PlaceHolder<T>::GetType()
{
    return typeid(T);
}

template <class T>
Any::Holder* Any::PlaceHolder<T>::Clone()
{
    return new PlaceHolder<T>(_val);
}

// Any
Any::Any() 
    :_content(nullptr) 
{}

template<class T>
Any::Any(const T& val)
{
    _content = new PlaceHolder<T>(val);
}

Any::Any (const Any& other)
{
    _content = (other._content == nullptr) ? nullptr : other._content->Clone();
}

template<class T>
T* Any::GetValAddr()
{
    if (_content)
    {
        if(typeid(T) == _content->GetType())
            return &((static_cast<PlaceHolder<T>*>(_content))->_val);
    }
    return nullptr;
}

template<class T>
Any& Any::operator=(const T& val)
{
//        if (_content)
//            delete _content;
//        _content = new PlaceHolder<T>(val);
    Any(val).Swap(*this);

    return *this;
}

Any& Any::operator=(const Any& other)
{
    if (&other != this)
    {
        Any(other).Swap(*this);
    }
    return *this;
}

Any::~Any()
{
    delete _content;
}

void Any::Swap(Any& other)
{
    std::swap(_content, other._content);
}