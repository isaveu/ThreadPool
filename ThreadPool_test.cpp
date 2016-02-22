#include<iostream>
#include<vector>
#include<stdio.h>
#include<string>
#include<thread>

#include "ThreadPool.h"

using namespace std;

class TaskWork : public ITask
{
public:
    int Run() override
    {
        printf("thread id:%lu\n", std::this_thread::get_id());
        //std::this_thread::sleep_for(std::chrono::seconds(5)); 
        return 0;
    }
};

void test_ThreadPool()
{
    TaskWork task;
    ThreadPool thread_pool(10);

    for( int i = 0; i < 20; i++ )
        thread_pool.AddTask(&task);

    while( 1 ) { 
        if( thread_pool.GetTaskNum() == 0 && thread_pool.StopAll() == -1 )     { 
            cout << "exit the main.\n";
            exit(0);
        }
    }   
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
}

int main()
{
    test_ThreadPool();

    return 0;
}
