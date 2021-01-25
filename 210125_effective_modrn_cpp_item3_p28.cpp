// Effective Modern C++ - Item3 (p.28)
// Why forward the first parameter? I got the answer from this SO post:
// https://stackoverflow.com/questions/36242131/c-stdforward-on-the-container-calling-operator
// BTW, google did a very nice search. My search string was: "item3 container without forward"

////////////////////////////////////////////////////////////
// true_type, false_type

template <typename T, T val>
struct integral_constant {
    static constexpr T value = val;

    using value_type = T;
    using type = integral_constant<T, val>;

    constexpr operator T() { return val; }
    constexpr T operator()() { return val; }
};

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

static_assert(true_type::value);
static_assert(!false_type::value);

// conversion operator
static_assert(true_type().operator bool());
static_assert(!false_type().operator bool());
static_assert(bool(true_type()));
static_assert(!bool(false_type()));
static_assert(true_type()); // implicit
static_assert(!false_type()); // implicit

// function call operator
static_assert(true_type().operator()());
static_assert(!false_type().operator()());
static_assert(true_type()());
static_assert(!false_type()());

////////////////////////////////////////////////////////////
// remove_reference

template <typename T>
struct remove_reference { using type = T; };
template <typename T>
struct remove_reference<T&> { using type = T; };
template <typename T>
struct remove_reference<T&&> { using type = T; };
template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

////////////////////////////////////////////////////////////
// forward

template <typename T>
T&& forward(remove_reference_t<T>& arg)
{
    return static_cast<T&&>(arg);
}

////////////////////////////////////////////////////////////
// main

template <typename Container, typename Index>
auto withoutForward(Container&& c, Index i)
-> decltype(c[i])
{
    return c[i];
}

template <typename Container, typename Index>
auto withForward(Container&& c, Index i)
-> decltype(forward<Container>(c)[i])
{
    return forward<Container>(c)[i];
}

#include <cstdio>

template <typename T, int Size>
struct OddContainer {
    using value_type = T;

    OddContainer() { for (int i = 0; i < Size; i++) _data[i] = 3; }
 
    auto operator[](int i) & -> value_type& { puts("overload for &");  return _data[i]; };
    auto operator[](int i) && -> value_type& { puts("overload for &&"); return _data[i]; };

    T _data[Size];
};

int main()
{
    OddContainer<int, 5> c;
    printf("%d\n", c._data[0]);                                 // 3
 
    withoutForward(c, 0) = 1;                                   // overload for &
    printf("%d\n", c._data[0]);                                 // 1
 
    withForward(c, 0) = 2;                                      // overload for &
    printf("%d\n", c._data[0]);                                 // 2
 
    auto val = withoutForward(OddContainer<int, 3>(), 0);       // overload for & (OOPS)
    printf("%d\n", val);                                        // 3

    val = withForward(OddContainer<int, 3>(), 0);               // overload for &&
    printf("%d\n", val);                                        // 3

    OddContainer<int, 3>()[0] = 1;                              // overload for &&
}