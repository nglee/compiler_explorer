// Topic: Implicit Conversion
//
// In 200923_string_literals_are_lvalues.cpp, I have verified that string literals are lvalues.
// But let's take a look at the following code snippet.
//
// #include <string>
// std::string& r = "hi";   // error: cannot bind non-const lvalue reference of type 'std::string&'
//                          // to an rvalue of type 'std::string'
// std::string&& rr = "hi"; // OK
//
// So what's happening here? The compiler message goes on:
//
//                          // note: after user-defined conversion
//                          // basic_string(const _CharT* __s, const _Alloc& __a = _Alloc())
//
// Remember that the type of the string literal "hi" is 'const char[3]',
// and when it is used to initialize a reference('std::string&'),
// "hi" is implicitly converted to 'std::string' before initialization.
//
// From https://en.cppreference.com/w/cpp/language/reference_initialization
// > const std::string& rs = "abc"; // rs refers to temporary copy-initialized from char array
//
// Let's verify this in this code.

const char(&r)[3] = "hi";     // OK
//const char(&&rr)[3] = "hi"; // cannot bind rvalue reference of type 'const char(&&)[3]'
                              // to lvalue of type 'const char[3]'

#include <algorithm> // std::copy
//#include <utility> // included in algorithm?
//#include <cstdio>  // included in algorithm?

template <typename T>
void test(T&&)
{
    if (std::is_reference<T>::value)
        puts("lvalue");
    else
        puts("rvalue");
}

void oldtest()
{
    test("hi");            // prints "lvalue"
    test(std::move("hi")); // prints "rvalue"
}

class MyString
{
public:
    MyString(const char* cstr) // no explicit is intended
    {
        const char* const start = cstr;
        std::size_t len = 0;
        while(*cstr++) ++len;
        data = new char[len];
        std::copy_n(start, len, data);

        puts("MyString(const char* cstr)");
    }

    ~MyString() { delete[] data; };

    void print() const
    {
        printf("%s\n", data);
    }
private:
    char* data;
};

void newtest()
{
    MyString o1{"o1"};
    MyString o2 = "o2";  // Compiler error if constructor is explicit
    //MyString& r = "r"; // error: cannot bind non-const lvalue reference of type 'MyString&' to an rvalue of type 'MyString'
                         // note: after user-defined conversion: 'MyString::MyString(const char*)'
    const MyString& cr = "cr";
    MyString&& rr = "rr";

    o1.print();
    o2.print();
    cr.print();
    rr.print();

    // string literal really is an lvalue
    const char(*tmp)[54] = &"we can take the address if it is an lvalue expression";
}

int main()
{
    oldtest(); // 200923_string_literals_are_lvalues.cpp
    newtest();
}