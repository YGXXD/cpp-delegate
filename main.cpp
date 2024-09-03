//  MIT License
//
//  Copyright (c) 2021-2022 有个小小杜
//
//  Created by 有个小小杜
//

#include "delegate.h"
#include <iostream>

using namespace xxd;
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

int main(int argc, const char * argv[]) 
{
    auto a = std::make_shared<A>();
    auto b = new B();
    FOnAdd d;
    d.add_function(&Add);
    d.add_safe_obj(a, &A::Add);
    d.add_object(b, &B::Add);
    d(5, 6);
    
    return 0;
}
