// https://www.youtube.com/watch?v=sQCSX7vmmKY
// Back to Basics: Smart Pointers - Rainer Grimm - CppCon 2020

#include <cstdio>

// integral_constant, bool_constant, true_type, false_type
template <typename T, T val>
struct integral_constant {
    static constexpr T value = val;

    using type       = integral_constant<T, val>;
    using value_type = T;
};

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

// is_same
template <typename, typename>
struct is_same : false_type {};
template <typename U>
struct is_same<U, U> : true_type {};
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// void_t
template <typename... T>
using void_t = void;

// declval
template <typename T>
T&& declval();

// is_copy_constructible
template <typename T, typename = void_t<>>
struct is_copy_constructible : false_type {};
template <typename T>
struct is_copy_constructible<T, void_t<decltype(T(declval<const T&>()))>> : true_type {};
template <typename T>
inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

namespace test_is_copy_constructible {
    struct yes {};
    struct no { no(const no&) = delete; };

    static_assert(is_copy_constructible_v<yes>);
    static_assert(not is_copy_constructible_v<no>);
}

// is_move_constructible
template <typename T>
struct is_move_constructible
{
private:
    template <typename U, typename = decltype(U(declval<U&&>()))>
    static constexpr true_type try_construction(U&&);
    static constexpr false_type try_construction(...);
public:
    static constexpr bool value = decltype(try_construction(declval<T>()))::value;
};
template <typename T>
inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

namespace test_is_move_constructible {
    struct yes {};
    struct no { no(no&&) = delete; };

    static_assert(is_move_constructible_v<yes>);
    static_assert(not is_move_constructible_v<no>);
}

// enable_if
template <bool, typename = void>
struct enable_if {};
template <typename T>
struct enable_if<true, T> { using type = T; };
template <bool B, typename T>
using enable_if_t = typename enable_if<B, T>::type;

// remove_reference
template <typename T>
struct remove_reference { using type = T; };
template <typename T>
struct remove_reference<T&> { using type = T; };
template <typename T>
struct remove_reference<T&&> { using type = T; };
template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// move
template <typename T>
remove_reference_t<T>&& move(T&& rhs)
{
    return static_cast<remove_reference_t<T>&&>(rhs);
}

// forward
template <typename T>
T&& forward(remove_reference_t<T>& rhs)
{
    return static_cast<T&&>(rhs);
}

namespace forward_test {
    template <typename T>
    decltype(auto) forward_tester(T&& arg) // cannot use auto as return type, instead, one must use decltype(auto) (c++14)
    {
        return forward<T>(arg);
    }

    static_assert(not is_same_v<int, decltype(forward_tester(5))>);
    static_assert(not is_same_v<int&, decltype(forward_tester(5))>);
    static_assert(is_same_v<int&&, decltype(forward_tester(5))>);

    int a = 0;
    static_assert(not is_same_v<int, decltype(forward_tester(a))>);
    static_assert(is_same_v<int&, decltype(forward_tester(a))>);
    static_assert(not is_same_v<int&&, decltype(forward_tester(a))>);

    const int ca = a;
    static_assert(not is_same_v<int&, decltype(forward_tester(ca))>);
    static_assert(is_same_v<const int&, decltype(forward_tester(ca))>);

    int& ra = a;
    static_assert(is_same_v<int&, decltype(forward_tester(ra))>);

    const int& cra = a;
    static_assert(is_same_v<const int&, decltype(forward_tester(cra))>);
}

struct Int {
    explicit Int(int _data)
        : data(_data)
    { puts("explicit Int(int)"); }

    ~Int() { printf("~Int()(%d)\n", data); }

    // both copy- and move- constructors are declared to test perfect forwarding
    Int(const Int& _rhs)        { this->data = _rhs.data; puts("Int(const Int&)"); }
    Int(Int&& _rhs)             { this->data = _rhs.data; puts("Int(Int&&)"); }

    Int& operator=(const Int&)  = delete;
    Int& operator=(Int&&)       = delete;

    operator char() const       { return static_cast<char>(data) + 48; }

    int data;
};

template <typename T, typename = enable_if_t<is_move_constructible_v<T> && is_copy_constructible_v<T>, T>>
struct S {

    explicit S(T&& _data)       // this is rvalue reference, not universal reference
        : data(move(_data)),    // so we should use move, not forward
          m_name()
    { puts("explicit S(T&&)"); }

    S(const char _name[6], T&& _data)
        : data(move(_data)),
          m_name()
    {
        for (int i = 0; i < 6; ++i) m_name[i] = _name[i];
        puts("S(char[6], T&&)");
    }

    S(const char _name[6], const T& _data)
        : data(_data),
          m_name()
    {
        for (int i = 0; i < 6; ++i) m_name[i] = _name[i];
        puts("S(char[6], const T&)");
    }

    ~S() { puts("~S()"); }

    S(const S& _rhs)            = delete;
    S(S&& _rhs)                 = delete;

    S& operator=(const S& _rhs) = delete;
    S& operator=(S&& _rhs)      = delete;

    T& get()                    { return data; }
    const T& get() const        { return data; }

    const char* name()          { return m_name; }
    const char* name() const    { return m_name; }

private:
    T data;
    char m_name[6];
};

// Minimal implementation of a type that implements an exclusive ownership over a resource
template <typename T>
class unique_ptr
{
public:
    unique_ptr()
        : m_data(nullptr)
    {
        //puts("unique_ptr()");
    }

    explicit unique_ptr(T* _data)
        : m_data(_data)
    {
        //puts("explicit unique_ptr(T*)");
    }

    ~unique_ptr()                               { release();             /*puts("~unique_ptr()");*/ }

    // Copy operations are explicitly deleted
    // The copy constructor is deleted if a move operation is declared.
    // The copy assignment operator is deleted if a move operation is declared.
    unique_ptr(const unique_ptr&)               = delete;
    unique_ptr& operator=(const unique_ptr&)    = delete;

    // YOU MUST NOT FORGET TO RELEASE() IN THE MOVE ASSIGNMENT OPERATOR,
    // WHILE YOU DON'T NEED SUCH A CALL IN THE MOVE CONSTRUCTOR!!!
    unique_ptr(unique_ptr&& _rhs)               {                                         grab(_rhs);   puts("unique_ptr(unique_ptr&& _rhs)"); }
    unique_ptr& operator=(unique_ptr&& _rhs)    { if (m_data != _rhs.m_data) { release(); grab(_rhs); } puts("unique_ptr& operator=(unique_ptr&& _rhs)"); return *this; }

    T* get() { return m_data; }

private:
    void release()                              { if (m_data) delete m_data; }
    void grab(unique_ptr& _rhs)                 { this->m_data = _rhs.m_data; _rhs.m_data = nullptr; }

    T* m_data;
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    //return unique_ptr<T>(new T(forward<Args...>(args...)));   // THIS DOES NOT WORK!!!
    return unique_ptr<T>(new T(forward<Args>(args)...));        // THIS IS THE RIGHT SYNTAX!!!
}

namespace alternative
{
    // Minimal implementation of a type that implements an exclusive ownership over a resource
    // - (Resharper) noexcept specification in move operations
    // - (Resharper) deleting null pointer has no effect
    template <typename T>
    class unique_ptr
    {
    public:
        unique_ptr()
            : m_data(nullptr)
        {}

        explicit unique_ptr(T* _data)
            : m_data(_data)
        {}

        ~unique_ptr()
        {
            delete m_data;
        }

        unique_ptr(const unique_ptr&)               = delete;
        unique_ptr& operator=(const unique_ptr&)    = delete;

        unique_ptr(unique_ptr&& _rhs) noexcept
        {
            puts("unique_ptr(unique_ptr&& _rhs)");
            m_data = _rhs.release();
        }

        unique_ptr& operator=(unique_ptr&& _rhs) noexcept
        {
            puts("unique_ptr& operator=(unique_ptr&& _rhs)");
            if (m_data != _rhs.m_data)
                reset(_rhs.release());
            return *this;
        }

        T* get() { return m_data; }

        T* release()
        {
            return exchange(nullptr);
        }

        void reset(T* new_data)
        {
            T* old = exchange(new_data);
            delete old;
        }

        void swap(unique_ptr& _rhs) noexcept
        {
            _rhs.m_data = exchange(_rhs.m_data);
        }

    private:
        T* exchange(T* new_data)
        {
            T* ret = m_data;
            m_data = new_data;
            return ret;
        }

        T* m_data;
    };
}

template <typename T>
class shared_ptr
{
public:
    shared_ptr()
        : m_data(nullptr), m_counter(nullptr)
    {
        //puts("shared_ptr()");
    }

    explicit shared_ptr(T* data)
        : m_data(data), m_counter(new unsigned)
    {
        //puts("explicit shared_ptr(T*)");
        *m_counter = 1;
    }

    ~shared_ptr()
    {
        //puts("~shared_ptr()");
        if (m_data && not --*m_counter)
        {
            delete m_data;
            delete m_counter;
        }
    }

    shared_ptr(const shared_ptr& _rhs)
        : m_data(_rhs.m_data), m_counter(_rhs.m_counter)
    {
        puts("shared_ptr(const shared_ptr& _rhs)");
        if (_rhs.m_counter)
            ++*(_rhs.m_counter);
    }

    shared_ptr(shared_ptr&& _rhs)
        : m_data(_rhs.m_data), m_counter(_rhs.m_counter)
    {
        puts("shared_ptr(shared_ptr&& _rhs)");
        _rhs.m_data = nullptr;
        _rhs.m_counter = nullptr;
    }

    shared_ptr& operator=(const shared_ptr& _rhs)
    {
        puts("shared_ptr& operator=(const shared_ptr& _rhs)");
        if (this->m_data)
            if (not --*m_counter)
            {
                delete m_data;
                delete m_counter;
            }

        this->m_data = _rhs.m_data;
        this->m_counter = _rhs.m_counter;
        ++*(this->m_counter);

        return *this;
    }

    shared_ptr& operator=(shared_ptr&& _rhs)
    {
        puts("shared_ptr& operator=(shared_ptr&& _rhs)");
        if (this->m_data)
            if (not --*m_counter)
            {
                delete m_data;
                delete m_counter;
            }

        this->m_data = _rhs.m_data;
        this->m_counter = _rhs.m_counter;

        _rhs.m_data = nullptr;
        _rhs.m_counter = nullptr;

        return *this;
    }

    const T* get() const        { return m_data; }
    unsigned use_count() const  { return *m_counter; }

private:
    T* m_data;
    unsigned* m_counter;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args)
{
    return shared_ptr<T>(new T(forward<Args>(args)...));
}

#include <iostream>

struct uptr_tag {};
struct sptr_tag {};

template <typename T>
void print_helper(uptr_tag, const S<T>* const ptr)
{
    printf("%p", ptr);
    if (ptr)
        std::cout << " [" << ptr->name() << "](" << ptr->get() << ")\n";
    else
        std::cout << "\n";
}

template <typename T>
void print_helper(sptr_tag, T&& ptr)
{
    printf("%p", ptr.get());
    if (ptr.get())
        std::cout << " [" << ptr.get()->name() << "](" << ptr.get()->get() << ") count:" << ptr.use_count() << "\n";
    else
        std::cout << "\n";
}

template <typename PtrT, typename T, typename... Ts>
void _print(PtrT t, T&& arg, Ts&&... args)
{
    print_helper(t, forward<T>(arg));
    if constexpr(sizeof...(args) > 0)
        _print(t, forward<Ts>(args)...);
}

template <typename PtrT, typename... Ts>
void print(PtrT t, Ts&&... args)
{
    std::cout << "\n";
    _print(t, forward<Ts>(args)...);
    std::cout << "\n";
}

void test_unique_ptr()
{
    unique_ptr<S<Int>> uptr1{new S("uptr1", Int{1})};
    print(uptr_tag{}, uptr1.get());

    auto uptr2 = make_unique<S<Int>>("uptr2", Int{2});
    print(uptr_tag{}, uptr1.get(), uptr2.get());

    unique_ptr<S<Int>> uptr3;
    {
        Int tmp{3};
        uptr3 = make_unique<S<Int>>("uptr3", tmp);
    }
    print(uptr_tag{}, uptr1.get(), uptr2.get(), uptr3.get());

    uptr1 = move(uptr2);
    print(uptr_tag{}, uptr1.get(), uptr2.get(), uptr3.get());

    auto uptr4(move(uptr3));
    print(uptr_tag{}, uptr1.get(), uptr2.get(), uptr3.get(), uptr4.get());

    uptr1 = move(uptr1);
    print(uptr_tag{}, uptr1.get(), uptr2.get(), uptr3.get(), uptr4.get());
}

void test_shared_ptr()
{
    shared_ptr<S<Int>> sptr1{new S("sptr1", Int{1})};
    print(sptr_tag{}, sptr1);

    auto sptr2 = make_shared<S<Int>>("sptr2", Int{2});
    print(sptr_tag{}, sptr1, sptr2);

    shared_ptr<S<Int>> sptr3;
    {
        Int tmp{3};
        sptr3 = make_shared<S<Int>>("sptr3", tmp);
    }
    print(sptr_tag{}, sptr1, sptr2, sptr3);

    sptr1 = sptr2;
    print(sptr_tag{}, sptr1, sptr2, sptr3);

    sptr1 = move(sptr3);
    print(sptr_tag{}, sptr1, sptr2, sptr3);

    auto sptr4(move(sptr1));
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4);

    sptr3 = sptr4;
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4);

    sptr1 = move(sptr3);
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4);

    sptr1 = move(sptr2);
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4);

    sptr1 = move(sptr4);
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4);

    auto sptr5(sptr1);
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4, sptr5);

    sptr1 = sptr1;
    print(sptr_tag{}, sptr1, sptr2, sptr3, sptr4, sptr5);
}

#include <chrono>
#include <memory>

void test_performance()
{
    for (int i = 0; i < 1000000; ++i)
    {
        int* tmp(new int(i));
        delete tmp;
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        int* tmp(new int(i));
        delete tmp;
    }
    auto end = std::chrono::high_resolution_clock::now();
    printf("new: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        unique_ptr<int> tmp(new int(i));
    }
    end = std::chrono::high_resolution_clock::now();
    printf("unique_ptr: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        auto tmp = make_unique<int>(i);
    }
    end = std::chrono::high_resolution_clock::now();
    printf("make_unique: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        shared_ptr<int> tmp(new int(i));
    }
    end = std::chrono::high_resolution_clock::now();
    printf("shared_ptr: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        auto tmp = make_shared<int>(i);
    }
    end = std::chrono::high_resolution_clock::now();
    printf("make_shared: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        std::unique_ptr<int> tmp(new int(i));
    }
    end = std::chrono::high_resolution_clock::now();
    printf("std::unique_ptr: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        auto tmp = std::make_unique<int>(i);
    }
    end = std::chrono::high_resolution_clock::now();
    printf("std::make_unique: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        std::shared_ptr<int> tmp(new int(i));
    }
    end = std::chrono::high_resolution_clock::now();
    printf("std::shared_ptr: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i)
    {
        auto tmp = std::make_shared<int>(i);
    }
    end = std::chrono::high_resolution_clock::now();
    printf("std::make_shared: %f ms\n", std::chrono::duration<float, std::milli>(end - start).count());
}

int main()
{
    test_unique_ptr();

    std::cout << "\n";

    test_shared_ptr();

    std::cout << "\n";

    test_performance();
}
