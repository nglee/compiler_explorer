#include <cstdio>

// Takeaways
//
// 1. ptr and lref are the same in assembly
// 2. lref, rref, clref, and crref are all the same in assembly

void ptr(int* a)
{
    printf("%i\n", *a);
}

void lref(int& a)
{
    printf("%i\n", a);
}

void rref(int&& a)
{
    printf("%i\n", a);
}

void clref(const int& a)
{
    printf("%i\n", a);
}

void crref(const int&& a)
{
    printf("%i\n", a);
}

void pointer_reference()
{
    int a = 5;
 
    ptr(&a);

    // argument is lvalue
    lref(a);
    clref(a);
    //rref(a); // cannot bind rvalue reference to lvalue
    //crref(a); // cannot bind rvalue reference to lvalue

    // argument is rvalue
    //lref(5); // cannot bind non-const lvalue reference to rvalue
    clref(5);
    rref(5);
    crref(5);
}