#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<stdio.h>
#include<iostream>
#include<vector>
#include<thread>
#include<stdlib.h>
#include<mutex>
#include<string.h>
#include<condition_variable>


class Task
{
    protected:
        char * TaskName;
        void * TaskData;
    public:
        Task();
        Task( char * TName );
        virtual int Run() = 0;
        void SetData( void * TData );
        void showdata();
        virtual ~Task() {};
};

Task::Task() { TaskName = NULL; TaskData = NULL; }

Task::Task( char * TName )
{
    TaskName = TName;
    TaskData = NULL;
}

void Task::SetData(void* TData)
{
    TaskData = TData;
}

void Task::showdata()
{
    std::cout << TaskData << std::endl;
}

template<class TASK>
class ThreadPool
{
public:
    ThreadPool(int thread_num = 10);
    int GetTaskNum();
    int AddTask(TASK * task);
    int StopAll();
    int Create();
    void ThreadFun();

private:
    int thread_num_;
    bool shutdown_;
    std::vector<std::thread> threads_;
    std::vector<TASK*> task_list_;
    std::mutex task_list_mutex_;
    std::condition_variable condition_;
};

template<class TASK>
int ThreadPool<TASK>::Create()
{
    for( int i = 0; i < thread_num_; i++ )
        threads_.push_back(std::thread( &ThreadPool<TASK>::ThreadFun, this));
    return 0;
}

template<class TASK>
ThreadPool<TASK>::ThreadPool(int thread_num):thread_num_(thread_num)
{
    shutdown_ = false;
    std::cout << thread_num_  << " threads will be create." << std::endl;
    Create();
}

template<class TASK>
int ThreadPool<TASK>::GetTaskNum()
{
    return task_list_.size();
}

template<class TASK>
int ThreadPool<TASK>::AddTask(TASK * task)
{
    std::unique_lock<std::mutex> locker(task_list_mutex_);
    task_list_.push_back(task);
    condition_.notify_one();
    return 0;
}

template<class TASK>
void ThreadPool<TASK>::ThreadFun()
{
    thread::id tid = std::this_thread::get_id();
    while(1) {
        std::unique_lock<std::mutex> locker(task_list_mutex_);
        while (task_list_.size() == 0 && !shutdown_)
            condition_.wait( locker );
        if (shutdown_) {
            locker.unlock( );
            printf("Thread %lu will exit.\n",tid);
            return;
        }
        typename vector<TASK*>::iterator iter = task_list_.begin();
        if( iter != task_list_.end() ) {
            TASK* task = *iter;
            task_list_.erase( iter );
            locker.unlock( );
            task->Run();
            printf("%lu idle.\n",tid);
        }
     }
}

template<class TASK>
int ThreadPool<TASK>::StopAll()
{
    if(shutdown_) return -1;
    cout << "All thread will stop." << endl;
    shutdown_ = true;
    condition_.notify_all();
    for( int i = 0; i < thread_num_; i++ )
        threads_[i].join();

    threads_.clear();
    cout << "The Threadpool is stop." << endl;
    return 0;
}

#endif