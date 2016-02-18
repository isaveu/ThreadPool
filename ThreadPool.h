#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>


class Task
{
    protected:
        char* TaskName;
        void* TaskData;
    public:
        Task();
        virtual ~Task();
        Task( char * TName );
        virtual int Run() = 0;
        void SetData( void * TData );
        void showdata();
};

Task::Task() { TaskName = NULL; TaskData = NULL; }

Task::Task( char * TName )
{
    TaskName = TName;
    TaskData = NULL;
}

Task::~Task()
{
    if (TaskName)
    {
        delete TaskName;
        TaskName = NULL;
    }
    if (TaskData)
    {
        delete TaskData;
        TaskData = NULL;
    }
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
    int AddTask(TASK* task);
    int StopAll();
    int Create();
    void ThreadFun();

private:
    int thread_num_;
    bool shutdown_;
    std::vector<std::thread> threads_; 
    std::queue<TASK*> task_queue_;
    std::mutex task_queue_mutex_;
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
    return task_queue_.size();
}

template<class TASK>
int ThreadPool<TASK>::AddTask(TASK* task)
{
    std::unique_lock<std::mutex> locker(task_queue_mutex_);
    task_queue_.emplace(task);
    condition_.notify_one();
    return 0;
}

template<class TASK>
void ThreadPool<TASK>::ThreadFun()
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
        TASK* task = task_queue_.front();
        task_queue_.pop();
        locker.unlock( );
        task->Run();
        /*delete task;
        task = NULL;*/
        printf("%lu idle.\n",tid);
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