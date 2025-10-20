#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(ThreadInitCallback cb,
                const std::string& name)
    : loop_()
    , thread_([this](){threadFunc();},name)
    , exiting_(false)
    , mutex_()
    , cond_()
    , threadInitCallback_(std::move(cb))
{
}
EventLoopThread::~EventLoopThread()
{
    exiting_=true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();
    EventLoop* loop=nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this]()->bool{return loop_!=nullptr;});
        loop=loop_;
    }
    LOG_DEBUG("startLoop finish");
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop{};

    if(threadInitCallback_)
    {
        threadInitCallback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_one();
    }
    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
}