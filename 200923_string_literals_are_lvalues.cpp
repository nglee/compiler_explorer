#include <cstdio>

// is_same

template <typename, typename>
struct is_same {
    constexpr static bool value = false;
};

template <typename T>
struct is_same<T, T> {
    constexpr static bool value = true;
};

// remove_reference

template <typename T>
struct remove_reference {
    typedef T type;
};

template <typename T>
struct remove_reference<T&> {
    typedef T type;
};

template <typename T>
struct remove_reference<T&&> {
    typedef T type;
};

// is_reference

template <typename T>
struct is_reference {
    constexpr static bool value = !is_same<
            typename remove_reference<T>::type,
            T>::value;
};

// move

template <typename T>
typename remove_reference<T>::type&&
move (T&& param)
{
    return static_cast<typename remove_reference<T>::type&&>(param);
}

template <typename T>
void test(T&& value)
{
    if (is_reference<T>::value)
        puts("lvalue");
    else
        puts("rvalue");
}

int main()
{
    test("hi"); // prints "lvalue", a string literal in C++ is of type "const char [N]"
    test(move("hi")); // prints "rvalue"
}