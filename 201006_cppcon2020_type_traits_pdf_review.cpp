//https://github.com/CppCon/CppCon2020/blob/main/Presentations/template_metaprogramming_type_traits/template_metaprogramming_type_traits__jody_hagins__cppcon_2020.pdf

template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;

    using value_type = T;
    using type       = integral_constant<T, v>;
};

template <bool B>
using bool_constant = integral_constant<bool, B>;
using false_type = bool_constant<false>;
using true_type = bool_constant<true>;

// true_type and false_type are called nullary metafunctions
// because they have no parameters

template <typename T>
struct type_identity { using type = T; };   // C++20 introduces std::type_identity

template <typename T>
struct remove_const : type_identity<T> {};
template <typename T>
struct remove_const<T const> : type_identity<T> {};
template <typename T>
using remove_const_t = typename remove_const<T>::type;

template <typename T>
struct remove_volatile : type_identity<T> {};
template <typename T>
struct remove_volatile<T volatile> : type_identity<T> {};
template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T>
using remove_cv = remove_const<remove_volatile_t<T>>;
template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename, typename>
struct is_same : false_type {};
template <typename T>
struct is_same<T, T> : true_type {};
template <typename T, typename U>
inline static constexpr bool is_same_v = is_same<T, U>::value;

// is_one_of
template <typename T, typename... Args>
struct is_one_of;
template <typename T>
struct is_one_of<T> : false_type {};
template <typename T, typename... Args>
struct is_one_of<T, T, Args...> : true_type {};
template <typename T, typename U, typename... Args>
struct is_one_of<T, U, Args...> : is_one_of<T, Args...> {};

// is_void
template <typename T>
using is_void = is_one_of<remove_cv_t<T>, void>;
template <typename T>
inline constexpr bool is_void_v = is_void<T>::value;

namespace alternative {
    template <typename T, typename... Args>
    struct is_one_of : false_type {};
    template <typename T, typename... Args>
    struct is_one_of<T, T, Args...> : true_type {};
    template <typename T, typename U, typename... Args>
    struct is_one_of<T, U, Args...> : is_one_of<T, Args...> {};

    template <typename T>
    using is_void = is_one_of<remove_cv_t<T>, void>;

    template <typename T>
    inline constexpr bool is_void_v = is_void<T>::value;
}

// is_integral
template <typename T>
using is_integral = is_one_of<remove_cv_t<T>,
        char, /*char8_t,*/ char16_t, char32_t, wchar_t, // char8_t is c++20 feature
        bool,
        signed char, short int, int, long int, long long int,
        unsigned char, unsigned short int, unsigned int, unsigned long int, unsigned long long int>;
template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// is_floating_point
template <typename T>
using is_floating_point = is_one_of<remove_cv_t<T>, float, double, long double>;
template <typename T>
inline static constexpr bool is_floating_point_v = is_floating_point<T>::value;

namespace alternative1 {
 
    template <typename T>
    using is_floating_point = bool_constant<
               is_same_v<remove_cv_t<T>, float>
            || is_same_v<remove_cv_t<T>, double>
            || is_same_v<remove_cv_t<T>, long double>>;
    template <typename T>
    inline static constexpr bool is_floating_point_v = is_floating_point<T>::value;
}

namespace alternative2 {
    template <typename T>
    struct is_floating_point : false_type {};
    template <>
    struct is_floating_point<float> : true_type {};
    template <>
    struct is_floating_point<float const> : true_type {};
    template <>
    struct is_floating_point<float volatile> : true_type {};
    template <>
    struct is_floating_point<float const volatile> : true_type {};
    template <>
    struct is_floating_point<double> : true_type {};
    template <>
    struct is_floating_point<double const> : true_type {};
    template <>
    struct is_floating_point<double volatile> : true_type {};
    template <>
    struct is_floating_point<double const volatile> : true_type {};
    template <>
    struct is_floating_point<long double> : true_type {};
    template <>
    struct is_floating_point<long double const> : true_type {};
    template <>
    struct is_floating_point<long double volatile> : true_type {};
    template <>
    struct is_floating_point<long double const volatile> : true_type {};
    template <typename T>
    inline static constexpr bool is_floating_point_v = is_floating_point<T>::value;
}


// Convenience Calling Conventions
// : Type metafunctions use alias templates ending with "_t"
// : Value metafunctions use variable templates ending with "_v"

void test_is_one_of()
{
    static_assert(not is_void_v<int>);
    static_assert(is_void_v<void>);
    static_assert(is_void_v<volatile void>);
    static_assert(is_void_v<void const>);
    static_assert(is_void_v<volatile void const>);
 
    static_assert(not alternative::is_void_v<int>);
    static_assert(alternative::is_void_v<void>);
    static_assert(alternative::is_void_v<volatile void>);
    static_assert(alternative::is_void_v<void const>);
    static_assert(alternative::is_void_v<volatile void const>);

    static_assert(not is_integral_v<void>);
    static_assert(not is_integral_v<const float>);
    static_assert(not is_integral_v<const double volatile>);
    static_assert(not is_integral_v<const long double volatile>);
    static_assert(is_integral_v<const bool volatile>);
    static_assert(is_integral_v<unsigned long int volatile>);
    static_assert(is_integral_v<long long int const volatile>);

    static_assert(not is_floating_point_v<void>);
    static_assert(not is_floating_point_v<int>);
    static_assert(not is_floating_point_v<long const volatile>);
    static_assert(is_floating_point_v<double>);
    static_assert(is_floating_point_v<long double volatile>);
    static_assert(is_floating_point_v<const float volatile>);
    static_assert(alternative1::is_floating_point_v<double>);
    static_assert(alternative1::is_floating_point_v<long double volatile>);
    static_assert(alternative1::is_floating_point_v<const float volatile>);
    static_assert(alternative2::is_floating_point_v<double>);
    static_assert(alternative2::is_floating_point_v<long double volatile>);
    static_assert(alternative2::is_floating_point_v<const float volatile>);
}

// Reference
// N4436: Proposing Standard Library Support for the C++ Detection Idiom
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4436.pdf
// (p. 3) "Note particulary the role of std::declval in forming an archetypal expression
//         to be detected, and the use of decltype to inspect this expression in an
//         unevaluated context."

// Reference
// How does 'void_t' work
// https://stackoverflow.com/questions/27687389/how-does-void-t-work

// void_t
template <typename...>
using void_t = void;

// declval
template <typename T>
T&& declval();

// has_type_member
template <typename, typename = void_t<>>
struct has_type_member : false_type {};
template <typename T>
struct has_type_member<T, void_t<typename T::type>> : true_type {};

template <typename T>
inline static constexpr bool has_type_member_v = has_type_member<T>::value;

void test_has_type_member()
{
    static_assert(has_type_member_v<integral_constant<int, 42>>);
    static_assert(not has_type_member_v<int>);
}

// has_preincrement_operator
template <typename T>
using helper_pre_increment = decltype(++declval<T&>());
template <typename, typename = void_t<>>
struct has_pre_increment_operator : false_type {};
template <typename T>
struct has_pre_increment_operator<T, void_t<helper_pre_increment<T>>> : bool_constant<is_same_v<T&, helper_pre_increment<T>>> {};

template <typename T>
inline static constexpr bool has_pre_increment_operator_v = has_pre_increment_operator<T>::value;

void test_has_pre_increment_operator()
{
    struct yes {
        yes& operator++() { return *this; }
    };
    struct no {
    };
    struct yes_wrong {
        yes_wrong operator++() { return *this; }
    };

    static_assert(has_pre_increment_operator_v<int>);
    static_assert(has_pre_increment_operator_v<yes>);
    static_assert(not has_pre_increment_operator_v<no>);
    static_assert(not has_pre_increment_operator_v<yes_wrong>);
}

// has_postincrement_operator
template <typename T>
using helper_post_increment = decltype(declval<T&>()++);
template <typename, typename = void_t<>>
struct has_post_increment_operator : false_type {};
template <typename T>
struct has_post_increment_operator<T, void_t<helper_post_increment<T>>> : bool_constant<is_same_v<T, helper_post_increment<T>>> {};

template <typename T>
inline static constexpr bool has_post_increment_operator_v = has_post_increment_operator<T>::value;

void test_has_post_increment_operator()
{
    struct yes {
        yes operator++(int) { return *this; }
    };
    struct no{
    };
    struct yes_wrong {
        yes_wrong& operator++(int) { return *this; }
    };

    static_assert(has_post_increment_operator_v<int>);
    static_assert(has_post_increment_operator_v<yes>);
    static_assert(not has_post_increment_operator_v<no>);
    static_assert(not has_post_increment_operator_v<yes_wrong>);
}

// is_move_assignable
template <typename T>
using helper_move_assignable = decltype(declval<T&>() = declval<T&&>());
template <typename T, typename = void_t<>>
struct is_move_assignable : false_type {};
template <typename T>
struct is_move_assignable<T, void_t<helper_move_assignable<T>>> : bool_constant<is_same_v<T&, helper_move_assignable<T>>> {};

template <typename T>
inline static constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

void test_is_move_assignable()
{
    struct yes {
        yes& operator=(yes&&) = default;
    };
    struct no {
        no& operator=(no&&) = delete;
    };
    static_assert(is_move_assignable_v<int>);
    static_assert(is_move_assignable_v<yes>);
    static_assert(not is_move_assignable_v<no>);
}

// is_lvalue_reference
template <typename T>
struct is_lvalue_reference : false_type {};
template <typename T>
struct is_lvalue_reference<T&> : true_type {};

template <typename T>
inline static constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

void test_is_lvalue_reference()
{
    static_assert(is_lvalue_reference_v<int&>);
    static_assert(is_lvalue_reference_v<int const&>);
    static_assert(is_lvalue_reference_v<int volatile&>);
    static_assert(is_lvalue_reference_v<int const volatile&>);
    static_assert(is_lvalue_reference_v<int*&>);
    static_assert(is_lvalue_reference_v<int const*&>);
    static_assert(is_lvalue_reference_v<int *const&>);
    static_assert(is_lvalue_reference_v<int volatile*&>);
    static_assert(is_lvalue_reference_v<int const volatile*&>);
    static_assert(not is_lvalue_reference_v<int>);
    static_assert(not is_lvalue_reference_v<int const>);
    static_assert(not is_lvalue_reference_v<int volatile>);
    static_assert(not is_lvalue_reference_v<int const volatile>);
    static_assert(not is_lvalue_reference_v<int*>);
    static_assert(not is_lvalue_reference_v<int const*>);
    static_assert(not is_lvalue_reference_v<int *const>);
    static_assert(not is_lvalue_reference_v<int volatile*>);
    static_assert(not is_lvalue_reference_v<int const volatile*>);
    static_assert(not is_lvalue_reference_v<int&&>);
    static_assert(not is_lvalue_reference_v<int const&&>);
    static_assert(not is_lvalue_reference_v<int volatile&&>);
    static_assert(not is_lvalue_reference_v<int const volatile&&>);
    static_assert(not is_lvalue_reference_v<int*&&>);
    static_assert(not is_lvalue_reference_v<int const*&&>);
    static_assert(not is_lvalue_reference_v<int *const&&>);
    static_assert(not is_lvalue_reference_v<int volatile*&&>);
    static_assert(not is_lvalue_reference_v<int const volatile*&&>);
}

// is_function

// primary template
template <typename T>
struct is_function : false_type {};

// specialization for regular functions
template <typename Ret, typename... Args>
struct is_function<Ret(Args...)> : true_type {};

// specialization for functions that have a C-variadic part, such as std::printf
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...)> : true_type {};

// specialization for function types that have cv-qualifiers
// "The effect of a cv-qualifier-seq in a function declarator is not the same as adding cv-qualification on top
//  of the function type. In the latter case, the cv-qualifiers are ignored. [Note: A function type that has a
//  cv-qualifier-seq is not a cv-qualified type; there are no cv-qualified function types. -- end note] [Example:
//    typedef void F();
//    struct S {
//      const F f;        // OK: equivalent to: void f();
//    };
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile> : true_type {};

// specialization for functions types that have ref-qualifiers
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile &> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile &&> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile &&> : true_type {};

// specialization for noexcept
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile & noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) volatile && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args...) const volatile && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) volatile && noexcept> : true_type {};
template <typename Ret, typename... Args>
struct is_function<Ret(Args..., ...) const volatile && noexcept> : true_type {};

template <typename T>
inline static constexpr bool is_function_v = is_function<T>::value;

void test_is_function()
{
    static_assert(not is_function_v<int>);
    static_assert(is_function_v<void()>);
    static_assert(is_function_v<int(double, char)>);

    static_assert(is_function_v<void(const char*, ...)>); // printf

    using F = int(double, char) const volatile;
    static_assert(is_function_v<F>);
    static_assert(is_function_v<const F>);

    static_assert(is_function_v<int()&>);
    static_assert(is_function_v<void(int, double) volatile &&>);

    static_assert(is_function_v<double(char, ...) noexcept>);
}

// template-template parameter (type parameter, non-type parameter)
template <template<typename T, typename U> typename IsSameMetaFunc, typename T, typename U>
struct generic_is_same : IsSameMetaFunc<T, U> {};

template <template<typename T, typename U> typename IsSameMetaFunc, typename T, typename U>
inline static constexpr bool generic_is_same_v = generic_is_same<IsSameMetaFunc, T, U>::value;

void test_generic_is_same()
{
    static_assert(generic_is_same_v<is_same, remove_cv_t<int const>, remove_const_t<remove_volatile_t<int volatile>>>);
}

int main()
{
    test_is_one_of();

    test_void_t();

    test_has_type_member();

    test_has_pre_increment_operator();

    test_has_post_increment_operator();

    test_is_move_assignable();

    test_is_lvalue_reference();

    test_is_function();

    test_generic_is_same();
}
