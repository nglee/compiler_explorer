#include <type_traits>
#include <string>
#include <cstdio>

template <typename T>
typename std::remove_reference<T>::type&&
move(T&& param)
{
    typedef typename std::remove_reference<T>::type&& ReturnType;
    return static_cast<ReturnType>(param);
}

template <typename T>
T&&
forward(typename std::remove_reference<T>::type& param)
{
    return static_cast<T&&>(param);
}

class A
{
public:
    A() {}
    A(const A& rhs) { puts("A(const A&)"); }
    A(A&& rhs) { puts("A(A&&)"); }
    A& operator=(const A& rhs) { puts("A& operator=(const A&)"); return *this; }
    A& operator=(A&& rhs) { puts("A& operator=(A&&)"); return *this; }

    void swapCopy(A& rhs)
    {
        A tmp = *this;
        *this = rhs;
        rhs = tmp;
    }

    void swapMove(A& rhs)
    {
        A tmp = move(*this);
        *this = move(rhs);
        rhs = move(tmp);
    }
};

class Widget
{
public:
    template <typename T>
    void
    setA(T&& newA)
    {
        a = forward<T>(newA);
    }

private:
    A a;
};

int main()
{
    Widget w;
    A a;

    // perfect forwarding
    w.setA(a);
    w.setA(A());
    // A& operator=(const A&)
    // A& operator=(A&&)

    // move vs. copy
    A a1, a2;
    a1.swapCopy(a2);
    a1.swapMove(a2);
    // A(const A&)
    // A& operator=(const A&)
    // A& operator=(const A&)
    // A(A&&)
    // A& operator=(A&&)
    // A& operator=(A&&)
}