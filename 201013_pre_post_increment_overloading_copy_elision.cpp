#include <cstdio>

template <typename T>
struct type_identity { using type = T; };

template <typename T>
struct remove_reference : type_identity<T> {};
template <typename T>
struct remove_reference<T&> : type_identity<T> {};
template <typename T>
struct remove_reference<T&&> : type_identity<T> {};

template <typename T>
using remove_reference_t = remove_reference<T>::type;

template <typename T>
remove_reference_t<T>&& move(T&& param)
{
    return static_cast<remove_reference_t<T>&&>(param);
}

struct S {
    explicit S(int _data) : data(_data) { puts("explicit S(int)"); }

    ~S() { puts("~S()"); }
 
    S(const S&  rhs) : data(rhs.data) { puts("S(const S& rhs)"); }
    S(      S&& rhs) : data(rhs.data) { puts("S(S&&)"); }

    S& operator++()    { ++data; return *this; }
    S  operator++(int) { S ret(data); ++data; return ret; }
 
    int data;
};

void testS()
{
    {
        S s(0);
        S s1(s);
        S s2(move(s1));

        // explicit S(int) // S s(0)
        // S(const S& rhs) // S s1(s)
        // S(S&&)          // S s2(move(s1))
        // ~S()
        // ~S()
        // ~S()
    }
 
    S s(0);
    S s1(++s);
    S s2(move(++s1));
    S s3(s2++);
    S s4(move(s3++));
    printf("s : %d\n", s.data);
    printf("s1: %d\n", s1.data);
    printf("s2: %d\n", s2.data);
    printf("s3: %d\n", s3.data);
    printf("s4: %d\n", s4.data);

    // explicit S(int) // S s(0)
    // S(const S& rhs) // S s1(++s)
    // S(S&&)          // S s2(move(++s1))
    // explicit S(int) // S s3(s2++)       => NRVO
    // explicit S(int) // S s4(move(s3++)) => worst!
    // S(S&&)          //                     do not use it this way
    // ~S()            //                     destroys temporary
    // s : 1
    // s1: 2
    // s2: 3
    // s3: 3
    // s4: 2 
    // ~S()
    // ~S()
    // ~S()
    // ~S()
    // ~S()
}

struct T {
    explicit T(int value)
        : data(new int)
    {
        *data = value;
        puts("explicit T(int)");
    }

    ~T()
    {
        if (data)
        {
            delete data;
        }
        puts("~T()");
    }
 
    T(const T& rhs)
    {
        data = new int;
        *data = *(rhs.data);
        puts("T(const T& rhs)");
    }
 
    T(T&& rhs)
    {
        data = rhs.data;
        rhs.data = nullptr;
        puts("T(T&&)");
    }

    T& operator++()
    {
        ++*data;
        return *this;
    }
 
    T  operator++(int)
    {
        T ret(*data);
        ++*data;
        return ret;
    }

    int* data;
};

void testT()
{
    {
        T t(0);
        T t1(t);
        T t2(move(t));

        // explicit T(int) // T t(0)
        // T(const T& rhs) // T t1(t)
        // T(T&&)          // T t2(move(t))
        // ~T()
        // ~T()
        // ~T()
    }
 
    T t(0);
    T t1(++t);
    T t2(move(++t1));
    T t3(t2++);
    T t4(move(t3++));
    printf("t : %d\n", *(t.data));
    // printf("t1: %d\n", *(t1.data)); // we cannot access t1
    printf("t2: %d\n", *(t2.data));
    printf("t3: %d\n", *(t3.data));    // but note that we CAN access t3
    printf("t4: %d\n", *(t4.data));

    // explicit T(int)
    // T(const T& rhs)
    // T(T&&)
    // explicit T(int)
    // explicit T(int)
    // T(T&&)
    // ~T()
    // t : 1
    // t2: 3
    // t3: 3
    // t4: 2
    // ~T()
    // ~T()
    // ~T()
    // ~T()
    // ~T()
}

int main()
{
    testS();
    testT();
}