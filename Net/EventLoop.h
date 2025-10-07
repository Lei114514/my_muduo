#pragma once 

#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Poller;
class Channel;

class EventLoop: public noncopyable
{
public:
    using Functor=std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void runInLoop(Functor cb);         
    void queueInLoop(Functor cb);
    
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    
    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    Timestamp pollReturnTime() const { return pollReturnTime_; }

private:
    using ChannelList = std::vector<Channel*>;

    void handleRead();
    void doPendingFunctors();

    const pid_t threadId_;
    std::unique_ptr<Poller> poller_;

    Timestamp pollReturnTime_;
    ChannelList activeChannels_;

    std::atomic_bool looping_;
    std::atomic_bool quit_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
    
};