#include <cstdio>
#include <algorithm>

// part 1 of 2
// Back to Basics: Templates (part 1 of 2) - Andreas Fertig - CppCon 2020

template <typename T, size_t N>
class Array
{
public:
    T* begin() { return m_data; };
    T* end() { return m_data + N; };
    const T* cbegin() const { return m_data; };
    const T* cend() const { return m_data + N; };
 
    T& operator[](size_t idx)
    {
        if (idx >= N)
            printf("index value(%llu) is out of bounds of this array of %llu elements\n", idx, N);
        return m_data[idx];
    }

    T m_data[N]; // same as vc++ std::array, note that it is public so we can construct the class with std::initializer_list
};

void array_test()
{
    Array<int, 5> ai{0, 1, 2, 3, 4}; // same as {{0, 1, 2, 3, 4}};

    for (const auto i : ai)
        printf("%d", i);
    printf("\n");

    ai[0] = 4;
    ai[1] = 3;

    for (const auto i : ai)
        printf("%d", i);
    printf("\n");
}

template <typename T>
class Span
{
public:
    template <size_t N>
    explicit Span(T (&_caar)[N])
        : m_data(new T[N]), m_size(N) { std::copy(_caar, _caar + N, m_data); }

    template <size_t N>
    explicit Span(Array<T, N> _arr)
        : m_data(new T[N]), m_size(N) { std::copy(_arr.cbegin(), _arr.cend(), m_data); }

    ~Span() { if (m_data) delete[] m_data; }

    T* begin() { return m_data; };
    T* end() { return m_data + m_size; };
    T& operator[] (size_t idx)
    {
        return m_data[idx];
    }

private:
    T* m_data;
    size_t m_size;
};

void span_test()
{
    int arr1[5]{1, 2, 3, 4, 1};
    Array<int, 5> arr2{1, 2, 3, 4, 2};

    Span<int> si1{arr1};
    Span<int> si2{arr2};

    for (const auto i : si1)
        printf("%d", i);
    printf("\n");
    for (const auto i : si2)
        printf("%d", i);
    printf("\n");

    si1[0] = si2[0] = 5;
    si1[1] = si2[1] = 4;
 
    for (const auto i : si1)
        printf("%d", i);
    printf("\n");
    for (const auto i : si2)
        printf("%d", i);
    printf("\n");
}

// part 2 of 2
// Back to Basics: Templates (part 2 of 2) - Andreas Fertig - CppCon 2020

// (p.2) variadic templates
template <typename T,
          typename... Ts> // template parameter pack with optional name
constexpr auto
min(const T& a,
    const T& b,
    const Ts&... ts) // function argument parameter pack with optional name
{
    // if constexpr is a c++17 feature
    const auto m = a <= b ? a : b;
    if constexpr (sizeof...(ts) > 0) // determine the number of arguments passed
        return min(m, ts...); // unpack the arguments in the body of a function
    else
        return m;
}

namespace old {
    // when if constexpr is not allowed
    template <typename T>
    constexpr auto min(const T& a, const T& b)
    {
        return a <= b ? a : b;
    }
    template <typename T, typename... Ts>
    constexpr auto min(const T& a, const T&b, const Ts&... ts)
    {
        return min(a <= b ? a : b, ts...);
    }

    static_assert(min(2, 3, 4, 5) == 2);
    static_assert(min(3, 2, 3, 4, 5) == 2);
}

void min_test()
{
    static_assert(min(2, 3, 4, 5) == 2);
    static_assert(min(3, 2, 3, 4, 5) == 2);
}

// (p.4) fold expressions (c++17)

#include <iostream>

// Print
template <typename T,
          typename... Ts>
void Print(const T& arg, const Ts&... args)
{
    std::cout << arg;
    auto coutSpaceAndArg = [](const auto& arg)
    {
        std::cout << ' ' << arg;
    };

    (..., coutSpaceAndArg(args)); // note the comma operator
 
    std::cout << std::endl;
}

// Normalize
auto                       Normalize(const std::string& s) { return s; }
auto                       Normalize(const char* c_str) { return std::string(c_str); }
template <typename T> auto Normalize(const T& arg) { return std::to_string(arg); }

// Print in CSV format
template <typename T, typename... Ts>
auto PrintCSV(const T& t, const Ts&... ts)
{
    std::string ret = Normalize(t);
    auto coutCommaAndArg = [&ret](const auto& arg)
    {
        ret += ',';
        ret += Normalize(arg);
    };
 
    (..., coutCommaAndArg(ts)); // a unary left fold

    return ret;
}

void foldexpr_test()
{
    Print("Hello", "C++", 20);
    Print(PrintCSV("Hello", "C++", 20, std::string("@cpp2020")));
}

// (p.13) SFINAE and (p.14) Tag dispatch
namespace alternative {
    template <typename T>
    constexpr
    std::enable_if_t<not std::is_floating_point_v<T>, bool>
    equal(const T& a, const T& b)
    {
        return a == b;
    }

    template <typename T>
    constexpr
    std::enable_if_t<std::is_floating_point_v<T>, bool>
    equal(const T& a, const T& b)
    {
        return std::abs(a - b) < 0.00001;
    }
}

namespace internal {
    struct notFloatingPoint {};
    struct floatingPoint {};
 
    template <typename T>
    constexpr bool equal(const T& a, const T& b, notFloatingPoint)
    {
        return a == b;
    }
 
    template <typename T>
    constexpr bool equal(const T& a, const T& b, floatingPoint)
    {
        return std::abs(a - b) < 0.00001;
    }
}

template <typename T>
constexpr bool equal(const T& a, const T& b)
{
    using namespace internal;
 
    if constexpr(std::is_floating_point_v<T>)
        return equal(a, b, floatingPoint{});
    else
        return equal(a, b, notFloatingPoint{});
}

// (p.17) requires (c++20)
namespace future {
    template <typename T>
    requires(not std::is_floating_point_v<T>)
    constexpr bool equal(const T& a, const T& b)
    {
        return a == b;
    }
    
    template <typename T>
    requires(std::is_floating_point_v<T>)
    constexpr bool equal(const T& a, const T& b)
    {
        return std::abs(a - b) < 0.00001;
    }
}

void tagdispatch_test()
{
    static_assert(equal(1, 1));
    static_assert(!equal(1, 2));
    static_assert(equal(1.0f, 1.00000001f));
    static_assert(!equal(1.0f, 1.001f));
 
    static_assert(alternative::equal(1, 1));
    static_assert(!alternative::equal(1, 2));
    static_assert(alternative::equal(1.0f, 1.00000001f));
    static_assert(!alternative::equal(1.0f, 1.001f));
 
    static_assert(future::equal(1, 1));
    static_assert(!future::equal(1, 2));
    static_assert(future::equal(1.0f, 1.00000001f));
    static_assert(!future::equal(1.0f, 1.001f));
}

// (p.18) Template template parameters
template <
    template <class, class> class Container,    // a template template parameter
    class T,                                    // parameter for Container
    class Allocator = std::allocator<T>>        // parameter for Container
void Fun(const Container<T, Allocator>& c)
{
    for (const auto& e : c)
    {
        printf("%d ", e);
    }
    printf("\n");
}

#include <list>

void ttp_test()
{
    std::vector<int> v = { 2, 4, 6, 8, 10 };
    Fun(v);

    std::list<char> l = { 'x', 'y', 'z', 'w' };
    Fun(l);
}

int main()
{
    array_test();
    span_test();
    min_test();
    foldexpr_test();
    tagdispatch_test();
    ttp_test();
}