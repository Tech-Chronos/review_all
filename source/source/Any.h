#include <iostream>
#include <string>
#include <assert.h>

class Any
{
private:
    class Holder
    {
    public:
        virtual ~Holder() {}
        virtual const std::type_info& GetType() = 0;
        virtual Holder* Clone() = 0;
    };

    template<class T>
    class PlaceHolder:public Holder
    {
    public:
        PlaceHolder(const T& val);

        const std::type_info& GetType() override;

        Holder* Clone() override;

    public:
        T _val;
    };
public:
    // 构造
    Any();

    // 构造
    template<class T>
    Any(const T& val);

    // 拷贝构造
    Any (const Any& other);

    template<class T>
    T* GetValAddr();

    template<class T>
    Any& operator=(const T& val);

    Any& operator=(const Any& other);

    ~Any();

private:
    void Swap(Any& other);

private:
    Holder* _content;
};

