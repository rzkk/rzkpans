#include<iostream>
#include<thread>
#include<vector>
#include<mutex>
#include<chrono>
#include<atomic>

using std::cout;
using std::endl;

// int gs_your_money = 0;
std::atomic<int> gs_your_money;
int thread_count = 2;
int N = 1000000; //每个线程执行次数

std::mutex gs_mutex;
void add_money(){
    for(int i = 0;i< N ;++i){
        // std::lock_guard<std::mutex> lock(gs_mutex);
        ++gs_your_money;
    }
}

int main(){
    std::vector<std::thread> threads;
    for(int i = 0 ;i < thread_count ; ++i){
        threads.emplace_back(add_money);
    }
    for(auto & t : threads){
        t.join();
    }

    cout<<"--------------" <<gs_your_money<<endl;
    return 0;
}