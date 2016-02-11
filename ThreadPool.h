#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<vector>
#include<thread>
#include<pthread.h>
#include<stdlib.h>
#include<mutex>
#include<string.h>
#include<condition_variable>

using namespace std;

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

template<class TASK>
class ThreadPool
{
    private:
        int thread_num;
        vector<TASK *> TaskList;
        vector<thread> threads;
        bool shutdown;

        mutex _x;
        condition_variable  _cond;

    public:
        ThreadPool( int ThN = 10 );
        int GetTaskNum();
        int PushTask( TASK * task );
        int StopAll();
        int Create();

        void * ThreadFun();
};

template<class TASK>
int ThreadPool<TASK>::Create()
{
    for( int i = 0; i < thread_num; i++ )
        threads.push_back( thread( &ThreadPool<TASK>::ThreadFun ) );//line54
        return 0;
}

template<class TASK>
ThreadPool<TASK>::ThreadPool( int ThN ):thread_num(ThN)
{
    shutdown = false;
    cout << thread_num  << " numbers threads will be create." << endl;
    Create();    //line 63
}

template<class TASK>
int ThreadPool<TASK>::GetTaskNum()
{
    return TaskList.size();
}

template<class TASK>
int ThreadPool<TASK>::PushTask( TASK * task )
{
    unique_lock<mutex> locker( _x );
    TaskList.push_back( task );
    _x.unlock();
    _cond.notify_one();
    return 0;
}

template<class TASK>
void * ThreadPool<TASK>::ThreadFun()
{
    thread::id tid = std::this_thread::get_id();
    while(1) {
        unique_lock<mutex> locker( _x );
        while( TaskList.size() == 0 && !shutdown )
            _cond.wait( locker );
        if( shutdown ) {
            locker.unlock( );
            printf("Thread %lu will exit.\n",tid);
            return (void *)0;
        }
        typename vector<TASK>::iterator iter = TaskList.begin();
        if( iter != TaskList .end() ) {
            TASK task = *iter;
            TaskList.erase( iter );
            locker.unlock( );
            task.Run();
            printf("%lu idle.\n",tid);
        }
     }
     return (void *)0;
}

template<class TASK>
int ThreadPool<TASK>::StopAll()
{
    if( shutdown ) return -1;
    cout << "All thread will stop." << endl;
    shutdown = true;
    _cond.notify_all();
    for( int i = 0; i < thread_num; i++ )
    threads[i].join();

    threads.clear();
    cout << "The Threadpool is stop." << endl;
    return 0;
}

Task::Task() { TaskName = NULL; TaskData = NULL; }

Task::Task( char * TName )
{
    TaskName = TName;
    TaskData = NULL;
}

void Task::SetData( void * TData )
{
    TaskData = TData;
}

void Task::showdata()
{
    cout << TaskData << endl;
}

#endif