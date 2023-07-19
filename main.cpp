#include "delegate.h"
#include <iostream>

using namespace dlgt;
using namespace std;

DECLARE_FUNCTION_MULTICAST_DELEGATE(FOnAdd, int, int)

class A
{
public:

    virtual void Add(int a, int b)
    {
        cout << a + b + c << endl;
    }

private:
    int c = 6;
};

class B : public A
{
public:
    virtual void Add(int a, int b)
    {
        cout << a * b << endl;
    }
};

void Add(int a, int b)
{
    cout << a - b << endl;

}

int main(int argc, const char * argv[]) {
    auto a = std::make_shared<A>();
    auto b = new B();
    FOnAdd d;
    d.AddFunction(&Add);
    d.AddSafeObj(a, &A::Add);
    d.AddObject(b, &B::Add);
    d(5, 6);
    
    return 0;
}
