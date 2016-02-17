#include<iostream>
#include<vector>
#include<stdio.h>
#include<string>
#include<thread>

#include "ThreadPool.h"

using namespace std;

class TaskWork : public Task
{
public:
    //Task() {}
    virtual int Run()
    {
        printf("%s\n",(char *)this->TaskData);
        std::this_thread::sleep_for(std::chrono::seconds(5)); 
        return 0;
    }
};

int main()
{
    TaskWork task;
    char obj[] = "abc";
    task.SetData( obj );
    task.showdata();
    ThreadPool<TaskWork> thread_pool(10);

    for( int i = 0; i < 20; i++ )    //line 33
        thread_pool.PushTask( &task );

    while( 1 ) { 
        if( thread_pool.GetTaskNum() == 0 && thread_pool.StopAll() == -1 )     { 
            cout << "exit the main.\n";
            exit(0);
        }
    }   
    std::this_thread::sleep_for(std::chrono::seconds(2)); 

    return 0;
}
