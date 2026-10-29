#include<iostream>
#include<vector>
#include<thread>
#include <pans/singleton.h>

using namespace std;


class A : public pans::Singleton<A>{

public:
    void log(const std::string& msg)
    {
        std::cout << "[LOG]: " << msg << " data=" <<m_data << std::endl;
    }
    friend class pans::Singleton<A>; // Singleton 访问A的私有成员构造函数
private:
    A()
    {
        std::puts("A::A() -- only once");
        m_data++;
    }
    ~A()
    {
        std::puts("A::~A() -- only once");
    }
    int  m_data{0};
};
//静态对象在程序装载时就开辟空间， 实际创建是在构造函数时发生， 并且构造时底层会加上一把锁
//同时 欲使编译器加锁，必须产生一个编译器无法忽略的行为
/*
   A()
    {
        std::puts("A::A() -- only once");
        m_data++;
    }
    ~A()
    {
        std::puts("A::~A() -- only once");
    }
有这种无法忽略的行为，编译器就会加锁 -》 锁保护的是首次初始化
如果 是编译器默认 ctor ,编译器会将其当作静态初始化对象，构造函数里面啥也没有， 不存在多线程去抢的资源
程序启动即可用

*/


int main(){

    std::vector<std::thread> threads;
    threads.reserve(8);

    for (int i = 0; i < 8; ++i)
    {
        threads.emplace_back([] {
            A::Instance().log("Hello Meyers Singleton.3");
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    return 0;
}