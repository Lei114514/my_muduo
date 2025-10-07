#include <sys/eventfd.h>
#include <unistd.h>

#include "EventLoop.h"
#include "Poller.h"
#include "EPollPoller.h"
#include "Channel.h"
#include "Logger.h"

thread_local EventLoop* t_loopInthisThread = nullptr;
const int kPollTimeMS=10000;

int createEventfd()
{
    int eventFd=::eventfd(0,EFD_CLOEXEC|EFD_NONBLOCK);
    if(eventFd<0)
    {
        LOG_FATAL("errno=%d",errno);
    }
    return eventFd;
}

EventLoop::EventLoop()
    : threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , looping_(false)
    , quit_(false)
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this,wakeupFd_))
    , callingPendingFunctors_(false)
{
    LOG_DEBUG("EventLoop create %p in thread %d",this,threadId_);
    if(t_loopInthisThread!=nullptr)
    {
        LOG_FATAL("another EventLoop %p exists in this thread %d",t_loopInthisThread,threadId_);
    }
    else 
    {
        t_loopInthisThread=this;
    }

    wakeupChannel_->setReadCallback([this](Timestamp)
    {
        this->handleRead();
    });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInthisThread=nullptr;
}

void EventLoop::loop()
{
    LOG_INFO("EventLoop %p start looping",this);
    looping_=true;
    quit_=false;

    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMS,&activeChannels_);
       
        for(Channel* channel:activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping",this);
    looping_=false;
}

void EventLoop::quit()
{
    quit_=true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else 
    {
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if(!isInLoopThread()||callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one=1;
    ssize_t n = write(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR("writes %lu bytes instead of 8",n);
    }
}
void EventLoop::handleRead()
{
    uint64_t one=1;
    ssize_t n =read(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR("read %lu bytes instead of 8",n);
    }
    
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_=true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(Functor functor:functors)
    {
        functor();
    }
 
    callingPendingFunctors_=false;
}
