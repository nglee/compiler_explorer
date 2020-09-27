// CppCon 2014: Walter E. Brown "Modern Template Metaprogramming: A Compendium, Part I"
// https://youtu.be/Am2is2QCvxY
// CppCon 2014: Walter E. Brown "Modern Template Metaprogramming: A Compendium, Part II"
// https://youtu.be/a0FliKwcwXE

template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;

    using value_type = T;
    using type       = integral_constant<T, v>;
};

template <bool B>
using bool_constant = integral_constant<bool, B>;
using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// STRUCT TEMPLATE conditional - Windows style
template <bool, typename T, typename>
struct conditional { using type = T; }; 

template <typename T, typename F>
struct conditional<false, T, F> { using type = F; };

template <bool Condition, typename T, typename F>
using conditional_t = typename conditional<Condition, T, F>::type;

template <typename, typename>
struct is_same : false_type {};
template <typename T>
struct is_same<T, T> : true_type {};
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template <unsigned M, unsigned N>
struct gcd {
    static_assert(M > 0);
    static_assert(N > 0);
    static constexpr unsigned value = conditional_t<
            M % N == 0,
            integral_constant<unsigned, N>,
            gcd<N, M % N>
        >::value;
};

namespace alternative_wrong {
    // This does not work
    template <unsigned M, unsigned N>
    struct gcd {
        static_assert(M > 0);
        static_assert(N > 0);
        static constexpr unsigned value =
                M % N == 0 ?
                N :
                gcd<N, M % N>::value;
    };
}

namespace alternative {
    template <unsigned M, unsigned N>
    struct gcd {
        static_assert(M > 0);
        static_assert(N > 0);
        static constexpr unsigned value = gcd<N, M % N>::value;
    };

    // Specify the base case with "partial specialization"
    template <unsigned M>
    struct gcd<M, 0> {
        static_assert(M > 0);
        static constexpr unsigned value = M;
    };
}

void check_gcd()
{
    static_assert(1 == gcd<1, 1>::value);
    static_assert(1 == gcd<1, 3>::value);
    static_assert(1 == gcd<17, 3>::value);
    static_assert(1 == gcd<2, 3>::value);
    static_assert(1 == gcd<19, 3>::value);
    static_assert(4 == gcd<4, 12>::value);
    static_assert(4 == gcd<8, 12>::value);
    static_assert(7 == gcd<49, 7>::value);
    static_assert(7 == gcd<49, 63>::value);

    // compiler error
    //static_assert(1 == alternative_wrong::gcd<1, 1>::value);
 
    static_assert(1 == alternative::gcd<1, 1>::value);
    static_assert(1 == alternative::gcd<1, 3>::value);
    static_assert(1 == alternative::gcd<17, 3>::value);
    static_assert(1 == alternative::gcd<2, 3>::value);
    static_assert(1 == alternative::gcd<19, 3>::value);
    static_assert(4 == alternative::gcd<4, 12>::value);
    static_assert(4 == alternative::gcd<8, 12>::value);
    static_assert(7 == alternative::gcd<49, 7>::value);
    static_assert(7 == alternative::gcd<49, 63>::value);

    // compare this case: the second template argument is 0
    //static_assert(50 == gcd<50, 0>::value);               // compiler error
    static_assert(50 == alternative::gcd<50, 0>::value);    // ok
}

// (29:51) rank - obtain the rank of an array in compile time
using size_t = long unsigned int;
template <typename T>
struct rank { static constexpr size_t value = 0UL; };
template <typename T, size_t N>
struct rank<T[N]> { static constexpr size_t value = 1UL + rank<T>::value; };
template <typename T>
struct rank<T[]> { static constexpr size_t value = 1UL + rank<T>::value; };

// (55:51)
namespace alternative {
    template <typename T>
    struct rank : integral_constant<size_t, 0UL> {};
    template <typename T, size_t N>
    struct rank<T[N]> : integral_constant<size_t, 1UL + rank<T>::value> {};
    template <typename T>
    struct rank<T[]> : integral_constant<size_t, 1UL + rank<T>::value> {};
}

void check_rank()
{
    static_assert(0 == rank<int>::value);
    static_assert(1 == rank<int[10]>::value);
    static_assert(2 == rank<int[10][20]>::value);
    static_assert(3 == rank<int[10][20][30]>::value);

    static_assert(0 == alternative::rank<int>::value);
    static_assert(1 == alternative::rank<int[10]>::value);
    static_assert(2 == alternative::rank<int[10][20]>::value);
    static_assert(3 == alternative::rank<int[10][20][30]>::value);
}

template <bool, typename = void> // why is it better to have the default argument void for the second parameter?
struct enable_if {};
template <typename T>
struct enable_if<true, T> { using type = T; };
template <bool Condition, typename T = void>
using enable_if_t = typename enable_if<Condition, T>::type;

void check_enable_if()
{
    static_assert(is_same_v<void, enable_if_t<true, void>>);
    static_assert(is_same_v<int, enable_if_t<true, int>>);
    //static_assert(is_same_v<int, enable_if_t<false, int>>); // compiler error: no type named 'type'
    static_assert(is_same_v<void, enable_if_t<true>>);        // compiler error on this line if no default argument were given
}

// Part II

// (9:35) is_one_of
template <typename T, typename... Args> // A template parameter pack is a template parameter that accepts zero or more template arguments
struct is_one_of : false_type {};

template <typename T, typename... Args>
struct is_one_of<T, T, Args...> : true_type {};

template <typename T, typename U, typename... Args>
struct is_one_of<T, U, Args...> : is_one_of<T, Args...> {};

template <typename T>
using is_void = is_one_of<T, void, void const, void volatile, void const volatile>;

void check_is_void()
{
    static_assert(is_void<void>::value);
    static_assert(is_void<const void volatile>::value);
    static_assert(not is_void<int>::value);
}

template <typename T>
T&& declval();

// this is not complete, it needs to check whether the return value of copy assignment is T&
// does it work when T has cv-qualifiers, or when T is an lvalue or an rvalue reference type?
template <typename T>
struct is_copy_assignable {
private:
    template <typename U, typename = decltype(declval<U&>() = declval<U const&>())>
    static constexpr true_type try_assignment(U&&);  // SFINAE may apply, another way of applying SFINAE, by giving a default value to an un-named template parameter
    static constexpr false_type try_assignment(...); // a langage feature we rarely use, catch-all overload, the worst possible match
public:
    using type = decltype(try_assignment(declval<T>())); // decltype of a function "call" is the type of its return value
};

template <typename T>
struct is_move_assignable {
private:
    template <typename U, typename = decltype(declval<U&>() = declval<U&&>())>
    static constexpr true_type try_assignment(U&&);
    static constexpr false_type try_assignment(...);
public:
    static constexpr bool value = decltype(try_assignment(declval<T>()))::value;
};

template <typename T>
struct is_copy_constructible {
private:
    template <typename U, typename = decltype(U(declval<U const&>()))>
    static constexpr true_type try_construction(U&&);
    static constexpr false_type try_construction(...);
public:
    static constexpr bool value = decltype(try_construction(declval<T>()))::value;
};

template <typename T>
struct is_move_constructible {
private:
    template <typename U, typename = decltype(U(declval<U&&>()))>
    static constexpr true_type try_construction(U&&);
    static constexpr false_type try_construction(...);
public:
    static constexpr bool value = decltype(try_construction(declval<T>()))::value;
};

void check_assignable()
{
    struct copy_a {
        copy_a& operator=(copy_a const&) { return *this; };
        copy_a& operator=(copy_a&&) = delete;
        //copy_a(copy_a const&) = delete;
        //copy_a(copy_a&&) = delete;
    };
    struct move_a {
        //move_a& operator=(move_a const&) = delete;
        move_a& operator=(move_a&&) { return *this; };
        //move_a(move_a const&) = delete;
        //move_a(move_a&&) = delete;
    };
    struct copy_c {
        //copy_c& operator=(copy_c const&) = delete;
        //copy_c& operator=(copy_c&&) = delete;
        copy_c(copy_c const&) {};
        copy_c(copy_c&&) = delete;
    };
    struct move_c {
        //move_c& operator=(move_c const&) = delete;
        //move_c& operator=(move_c&&) = delete;
        //move_c(move_c const&) = delete;
        move_c(move_c&&) {};
    };

    static_assert(is_copy_assignable<copy_a>::type::value);
    static_assert(not is_move_assignable<copy_a>::value);
    static_assert(not is_copy_constructible<copy_a>::value);
    static_assert(not is_move_constructible<copy_a>::value);

    static_assert(not is_copy_assignable<move_a>::type::value);
    static_assert(is_move_assignable<move_a>::value);
    static_assert(not is_copy_constructible<move_a>::value);
    static_assert(not is_move_constructible<move_a>::value);

    static_assert(not is_copy_assignable<copy_c>::type::value);
    static_assert(not is_move_assignable<copy_c>::value);
    static_assert(is_copy_constructible<copy_c>::value);
    static_assert(not is_move_constructible<copy_c>::value);

    static_assert(not is_copy_assignable<move_c>::type::value);
    static_assert(not is_move_assignable<move_c>::value);
    static_assert(not is_copy_constructible<move_c>::value);
    static_assert(is_move_constructible<move_c>::value);
}

template <typename... Args>
using void_t = void;

// it is essential that the second template parameter must have a default argument AND
// it must be the same type as the type of void_t
template <typename, typename = void>
struct has_type_member : false_type {};

template <typename T>
struct has_type_member<T, void_t<typename T::type>> : true_type {};

void check_type_member()
{
    static_assert(has_type_member<true_type>::value);
    static_assert(has_type_member<false_type>::value);
    static_assert(has_type_member<conditional<true, void, void>>::value);
    static_assert(has_type_member<has_type_member<void>>::value);
    static_assert(not has_type_member<void>::value);
}

// (50:16) also check for the return type
namespace advanced {
    // helper alias for the result type of a valid copy assignment:
    template <typename T>
    using copy_assignment_t = decltype(declval<T&>() = declval<T const &>());
    // note that "T&" has nothing to do with the return type of operator=
    // we can replace "T&" with "T" and get identical result

    // primary template handles all non-copy-assignable types:
    template <typename T, typename = void>
    struct is_copy_assignable : false_type {};
    // specialization recognizes and validates only copy-assignable types:
    template <typename T>
    struct is_copy_assignable<T, void_t<copy_assignment_t<T>>> : is_same<copy_assignment_t<T>, T&> {};
}

// "advanced" version is not the only solution
namespace ugly {
    template <typename T>
    using copy_assignment_t = decltype(declval<T&>() = declval<T const &>());

    template <typename T>
    struct is_copy_assignable {
    private:
        template <typename U, typename = copy_assignment_t<T>>
        static constexpr true_type try_assignment(U&&); 
        static constexpr false_type try_assignment(...);
    public:
        static constexpr bool value = decltype(try_assignment(declval<T>()))::value
                && is_same<copy_assignment_t<T>, T&>::value;
    };
}

void check_assignable_advanced()
{
    struct copy_a {
        copy_a& operator=(copy_a const&) { return *this; };
    };
 
    struct copy_a_wrong {
        copy_a_wrong operator=(copy_a_wrong const&) { return *this; };
    };

    static_assert(advanced::is_copy_assignable<copy_a>::value);
    static_assert(not advanced::is_copy_assignable<copy_a_wrong>::value);
 
    static_assert(ugly::is_copy_assignable<copy_a>::value);
    static_assert(not ugly::is_copy_assignable<copy_a_wrong>::value);

    // compare with the following result, the un-advanced version does not check the return type
    static_assert(is_copy_assignable<copy_a_wrong>::type::value);
 
    static_assert(is_same<advanced::copy_assignment_t<copy_a_wrong>, copy_a_wrong>::value);
    static_assert(not is_same<advanced::copy_assignment_t<copy_a_wrong>, copy_a_wrong&>::value);
}

int main()
{
    check_gcd();

    check_rank();

    check_is_void();

    check_enable_if();
 
    check_assignable();

    check_type_member();

    check_assignable_advanced();
}