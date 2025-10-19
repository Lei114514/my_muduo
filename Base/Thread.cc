#include <semaphore.h>

#include "Thread.h"
#include "CurrentThread.h"


std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    : func_(std::move(func))
    , name_(name)
    , tid_(0)
    , started_(false)
    , joined_(false)
{   
    setDefaultName();
}
void Thread::setDefaultName()
{
    int num=++numCreated_;
    if(name_.empty())
    {
        char buf[32]={0};
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_=buf;
    }
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }    
}

void Thread::start()
{
    started_=true;
    sem_t sem;
    sem_init(&sem,false,0);

    thread_ = std::make_shared<std::thread>([this,&sem]()->void{
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    });
    sem_wait(&sem);
}
void Thread::join()
{
    joined_=true;
    thread_->join();
}


/*
// g++ Thread.cc CurrentThread.cc -I ./ 
#include <iostream>
#include <unistd.h>

int main()
{
    std::function<void()> func=[]()->void{
        for(int i=1;i<=5;i++)
        {
            std::cout<<i<<'\n';
            sleep(1);
        }
    };
    Thread t(func);
    
    std::cout<<"name="<<t.name()<<'\n';
    std::cout<<"thread thread\n";
    t.start();
    t.join();
    std::cout<<"funinsh\n";
    
    return 0;
}
*/