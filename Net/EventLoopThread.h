#pragma once 
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

#include "noncopyable.h"
#include "Thread.h"

class EventLoop;

class EventLoopThread: private noncopyable
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    EventLoopThread(ThreadInitCallback cb=ThreadInitCallback(),
                    const std::string& name=std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();    

    EventLoop* loop_;
    Thread thread_;

    bool exiting_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback threadInitCallback_;
};