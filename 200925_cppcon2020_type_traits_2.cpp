// CppCon 2020
// Template Metaprogramming: Type Traits (part 2 of 2) - Jody Hagins
// https://youtu.be/dLZcocFOb5Q

// char8_t is introduced in c++20, so -std=c++20 flag is required
// https://en.cppreference.com/w/cpp/language/types

// (0:38)

// Primary type categories

// STRUCT TEMPLATE integral_constant
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;

    using value_type = T;
    using type       = integral_constant<T, v>;

    constexpr operator value_type() const noexcept {
        return value;
    }
    constexpr value_type operator()() const noexcept {
        return value;
    }
};

// ALIAS TEMPLATE bool_constant
template <bool B>
using bool_constant = integral_constant<bool, B>;
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// TypeIdentity
template <typename T>
struct TypeIdentity { using type = T; };

// remove_const
template <typename T>
struct remove_const : TypeIdentity<T> {};
template <typename T>
struct remove_const<T const> : TypeIdentity<T> {};
template <typename T>
using remove_const_t = typename remove_const<T>::type;

// remove_volatile
template <typename T>
struct remove_volatile : TypeIdentity<T> {};
template <typename T>
struct remove_volatile<T volatile> : TypeIdentity<T> {};
template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

// (13:31) remove_cv

template <typename T>
using remove_cv = remove_const<remove_volatile_t<T>>;
// note that we have remove_const without "_t"
// we are defining "remove_cv" as a metafunction here
// we would have invoked the metafunction if we had done ::type
// compare with the inherited form:

//template <typename T>
//struct remove_cv : TypeIdentity<remove_const_t<remove_volatile_t<T>>> {};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

// (20:44) is_same

// Primary template - two types are never the same
template <typename, typename>
struct is_same : false_type {};

// Partial specialization - when they are both the same
template <typename T> // you can have as many things in here as you want
struct is_same<T, T> : true_type {}; // number of template parameters must match the primary template

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// (21:49) is_same_raw (not a standard type trait)
template <typename T, typename U>
using is_same_raw = is_same<remove_cv_t<T>, remove_cv_t<U>>;
template <typename T, typename U>
inline constexpr bool is_same_raw_v = is_same_raw<T, U>::value;

// is_void
template <typename T>
using is_void = is_same_raw<void, T>;
template <typename T>
inline constexpr bool is_void_v = is_void<T>::value;

// is_null_pointer
using nullptr_t = decltype(nullptr); // from c++11, defined in <cstddef>

template <typename T>
using is_null_pointer = is_same_raw<nullptr_t, T>;
template <typename T>
inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

// (23:00) is_floating_point
template <typename T>
using is_floating_point = bool_constant<
       is_same_raw_v<float, T>
    || is_same_raw_v<double, T>
    || is_same_raw_v<long double, T>>;

template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

// (23:38) is_integral
template <typename T>
using is_integral = bool_constant<
       is_same_raw_v<bool, T>
    || is_same_raw_v<char, T>
    || is_same_raw_v<char8_t, T>
    || is_same_raw_v<char16_t, T>
    || is_same_raw_v<char32_t, T>
    || is_same_raw_v<wchar_t, T>
    || is_same_raw_v<signed char, T>
    || is_same_raw_v<short int, T>
    || is_same_raw_v<int, T>
    || is_same_raw_v<long int, T>
    || is_same_raw_v<long long int, T>
    || is_same_raw_v<unsigned char, T>
    || is_same_raw_v<unsigned short int, T>
    || is_same_raw_v<unsigned int, T>
    || is_same_raw_v<unsigned long int, T>
    || is_same_raw_v<unsigned long long int, T>>;
template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// (31:37) is_array
using size_t = unsigned long long;

template <typename T>
struct is_array : false_type {}; // matches any type
template <typename T, size_t N>
struct is_array<T[N]> : true_type {}; // bounded array
template <typename T>
struct is_array<T[]> : true_type {}; // unbounded array
template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

// (33:40) is_pointer
namespace detail {
    template <typename T>
    struct is_pointer_impl : false_type {};
    template <typename T>
    struct is_pointer_impl<T *> : true_type {};
}
template <typename T>
using is_pointer = detail::is_pointer_impl<remove_cv_t<T>>;
template <typename T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;

// (34:17) is_union
// This metafunction is actually impossible to implement without
// support from the compiler. Both clang and gcc provide this
// particular compiler intrinsic to determine if a type is a union.
// "Metaprogramming in C++ is not a first order language feature."
template <typename T>
using is_union = bool_constant<__is_union(T)>;
template <typename T>
inline constexpr bool is_union_v = is_union<T>::value;

// (36:30) is_class
// Almost always implemented as compiler intrinsic
// Without compiler assistance, impossible to distinguish between
// union and nonunion class type
// We have is_union (with help from the compiler)
// Can we tell if something in the "union or class" category?
// Each type must be in exactly one of the 14 categories

// (37:57) is_class_or_union
// What do we know about unions and classes that is unique to those two types?
// They can have members(struct is considered as a class)
// Devise a way to detect if a type can have a member
// How can you tell if a class has a member?
// The syntax for a pointer-to-member is valid for any class, even without any members

// (39:15)
// int* is a valid pointer type - does not have to point to anything
// int Foo::* is a member pointer type - does not have to point to anything

// Function overload resolution

template <typename T>
T&& declval();

namespace alternative {
    namespace detail {
        true_type is_nullptr(nullptr_t);
        false_type is_nullptr(...);
    }
    template <typename T>
    using is_null_pointer = decltype(detail::is_nullptr(declval<T>()));
}

// SFINAE [sf/inae]

template <typename T>
true_type can_have_pointer_to_member(int T::*);
template <typename T>
false_type can_have_pointer_to_member(...);

// is_class
namespace detail {
    template <typename T>
    bool_constant<not is_union_v<T>>
    is_class_or_union(int T::*);

    template <typename T>
    false_type is_class_or_union(...);
}

template <typename T>
using is_class = decltype(detail::is_class_or_union<T>(nullptr));
template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

// (52:30)
namespace alternative {
    namespace detail {
        template <typename T> constexpr bool is_class_or_union(int T::*) {
            return not is_union<T>::value;
        }
        template <typename T> constexpr bool is_class_or_union(...) {
            return false;
        }
    }
    template <typename T>
    using is_class = bool_constant<detail::is_class_or_union<T>(nullptr)>;
}

int main()
{
    // is_void
    static_assert(is_void_v<void const volatile>);
    static_assert(not is_void_v<int*>);

    // is_null_pointer
    static_assert(is_null_pointer<nullptr_t>::value);
    static_assert(not is_null_pointer_v<int*>);

    // is_floating_point
    static_assert(is_floating_point<const float>::value);
    static_assert(is_floating_point<long double>::value);
    static_assert(is_floating_point_v<const double volatile>);
    static_assert(not is_floating_point_v<volatile int>);

    // is_integral
    static_assert(is_integral_v<int>);
    static_assert(is_integral_v<int const>);
    static_assert(is_integral_v<const int>);
    static_assert(is_integral_v<int volatile>);
    static_assert(is_integral_v<volatile int>);
    static_assert(is_integral_v<const int volatile>);
    static_assert(is_integral_v<const volatile int>);
    static_assert(is_integral_v<int const volatile>);
    static_assert(not is_integral_v<float>);

    // is_array
    static_assert(is_array_v<int[5]>);
    static_assert(is_array_v<int[]>);
    static_assert(is_array_v<int[][5]>);
    static_assert(not is_array_v<int*>);

    // is_pointer
    static_assert(is_pointer_v<int*>);
    static_assert(is_pointer_v<int* const>);
    static_assert(is_pointer_v<int* volatile>);
    static_assert(is_pointer_v<const int* volatile>);
    static_assert(not is_pointer_v<int[]>);

    // is_union

    // function overload resolution
    static_assert(alternative::is_null_pointer<nullptr_t>::value);
    static_assert(not alternative::is_null_pointer<int>::value);
 
    // SFINAE
    struct S;
    static_assert(decltype(can_have_pointer_to_member<S>(nullptr))::value);
    static_assert(not decltype(can_have_pointer_to_member<int>(nullptr))::value);

    // is_class
    static_assert(is_class_v<S>);
    static_assert(not is_class_v<int>);
    static_assert(alternative::is_class<S>::value);
    static_assert(not alternative::is_class<int>::value);
}