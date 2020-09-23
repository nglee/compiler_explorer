// CppCon 2020
// Template Metaprogramming: Type Traits (part 1 of 2) - Jody Hagins
// https://youtu.be/tiAVWcjIF6o

// compile with -std=c++17

template <int T>
struct IntIdentity {
    static constexpr int value = T;
};

// Generic Identity Metafunction
template <typename T, T val>
struct ValueIdentity {
    static constexpr T value = val;
};

// Generic Identity Metafunction (C++17)
template <auto X>
struct ValueIdentity17 {
    static constexpr auto value = X;
};

// Value Metafunctions: Sum
template <int X, int Y>
struct IntSum {
    static constexpr int value = X + Y;
};

// Generic version is only possible with C++17
template <auto X, auto Y>
struct Sum17 {
    static constexpr auto value = X + Y;
};

// (18:47) Type Metafunctions example : Type Identity
template <typename T>
struct TypeIdentity {
    using type = T;
};

// (21:42) Convenience Calling Conventions
template <typename T, T val>
inline constexpr T ValueIdentity_v = ValueIdentity<T, val>::value;

template <auto X>
inline constexpr auto ValueIdentity17_v = ValueIdentity17<X>::value;

// (22:46) Convenience Calling Conventions for type metafunctions
template <typename T>
using TypeIdentity_t = typename TypeIdentity<T>::type;

// (25:34) std::integral_constant
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;
 
    using value_type = T;
    using type       = integral_constant<T, v>;

    constexpr operator value_type() const noexcept {
        return value;
    } // integral cast, instead ::value, cast to value_type
    constexpr value_type operator()() const noexcept {
        return value;
    } // allows to be created as a functor, it has a call operator
};

template <auto X>
struct integral_constant17 {
    static constexpr auto value = X;
    using value_type = decltype(value);
    using type       = integral_constant17<X>;
    constexpr operator value_type() const noexcept {
        return value;
    }
    constexpr value_type operator()() const noexcept {
        return value;
    }
};

// (27:42)
template <bool v>
using bool_constant = integral_constant<bool, v>;
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;
inline constexpr bool true_type_v = true_type::value;
inline constexpr bool false_type_v = false_type::value;

// (36:08)

// primary template: general case
template <typename T>
struct is_void : false_type {};

// explicit specialization: special case
// empty angle bracket: full or explicit specialization
// whenever you see a special type, in this case, void,
// don't use the general one
template <> struct is_void<void> : true_type {};
template <> struct is_void<void const> : true_type {};
template <> struct is_void<void volatile> : true_type {};
template <> struct is_void<void const volatile> : true_type {};

// standard mandated convenience alias
template <typename T>
inline constexpr bool is_void_v = is_void<T>::value;

// (47:59)

// remove_const (Transformation Trait)

// Primary template, do nothing if no const
template <typename T>
struct remove_const : TypeIdentity<T> {};

// Partial specialization, when detect const
// angle brackets are not empty: partial specialization
// it's not giving the full exact type(T const)
// it has to be deduced
// compiler will chose the one that has the best match
template <typename T>
struct remove_const<T const> : TypeIdentity<T> {};

// standard mandated convenience alias
template <typename T>
using remove_const_t = typename remove_const<T>::type;

// (56:10)

// conditional : a way of returning a type based on a value

// Primary template
template <bool Condition, typename T, typename F>
struct conditional : TypeIdentity<T> {};

// Partial specialization
template <typename T, typename F>
struct conditional<false, T, F> : TypeIdentity<F> {};

template <bool condition, typename T, typename F>
using conditional_t = typename conditional<condition, T, F>::type;


template <typename, typename>
struct is_same { static constexpr bool value = false_type_v; };
template <typename T>
struct is_same<T, T> { static constexpr bool value = true_type_v; };
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

int main()
{
    static_assert(239 == IntIdentity<239>::value);
    static_assert(239 == ValueIdentity<int, 239>::value);
    static_assert(239 == ValueIdentity17<239ULL>::value);
    static_assert(239ULL == ValueIdentity17<239ULL>::value);
    static_assert(42 == IntSum<30, 12>::value);
    static_assert(42 == Sum17<30, 12ULL>::value);
    static_assert(42ULL == Sum17<30, 12ULL>::value);

    static_assert(42 == ValueIdentity_v<int, 42>);
    static_assert(42 == ValueIdentity17_v<42>);
    static_assert(is_same<int, TypeIdentity_t<int>>::value);
    static_assert(is_same_v<unsigned long long, TypeIdentity_t<unsigned long long>>);
    static_assert(is_same<int, TypeIdentity<int>::type>::value);

    static_assert(is_void<void> {});    // implicit conversion
    static_assert(not is_void<int> {}); // implicit conversion
    static_assert(is_void_v<void const>);
    static_assert(is_void_v<volatile void>);

    static_assert(is_same_v<int volatile, remove_const_t<int volatile const>>);
    static_assert(is_same<int volatile, remove_const<int const volatile>::type>::value);

    static_assert(is_same_v<int, conditional_t<is_void_v<void>, int, char>>);
    static_assert(is_same<char, conditional<is_void_v<long>, int, char>::type>::value);
}