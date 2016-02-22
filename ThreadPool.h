#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>


class ITask
{
public:
    virtual ~ITask() {};
    virtual int Run() = 0;
};


class ThreadPool
{
public:
    ThreadPool(int thread_num = 10);
    int GetTaskNum();
    int AddTask(ITask* task);
    int StopAll();
    int Create();
    void ThreadFun();

private:
    int thread_num_;
    bool shutdown_;
    std::vector<std::thread> threads_; 
    std::queue<ITask*> task_queue_;
    std::mutex task_queue_mutex_;
    std::condition_variable condition_;
};

int ThreadPool::Create()
{
    for (int i = 0; i < thread_num_; i++ )
        threads_.push_back(std::thread( &ThreadPool::ThreadFun, this));
    return 0;
}

ThreadPool::ThreadPool(int thread_num):thread_num_(thread_num)
{
    shutdown_ = false;
    std::cout << thread_num_  << " threads will be create." << std::endl;
    Create();
}

int ThreadPool::GetTaskNum()
{
    return task_queue_.size();
}

int ThreadPool::AddTask(ITask* task)
{
    std::unique_lock<std::mutex> locker(task_queue_mutex_);
    task_queue_.emplace(task);
    condition_.notify_one();
    return 0;
}

void ThreadPool::ThreadFun()
{
    std::thread::id tid = std::this_thread::get_id();
    while(1) {
        std::unique_lock<std::mutex> locker(task_queue_mutex_);
        while (task_queue_.size() == 0 && !shutdown_)
            condition_.wait( locker );
        if (shutdown_) {
            locker.unlock( );
            printf("Thread %lu will exit.\n",tid);
            return;
        }
        ITask* task = task_queue_.front();
        task_queue_.pop();
        locker.unlock( );
        task->Run();
        printf("%lu idle.\n",tid);
     }
}

int ThreadPool::StopAll()
{
    if(shutdown_) return -1;
	std::cout << "All thread will stop." << std::endl;
    shutdown_ = true;
    condition_.notify_all();
    for( int i = 0; i < thread_num_; i++ )
        threads_[i].join();

    threads_.clear();
	std::cout << "The Threadpool is stop." << std::endl;
    return 0;
}

#endif