#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNonoEvent=0;
const int Channel::kReadEvent=EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent=EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd)
    :fd_(fd)
    ,loop_(loop)
    ,events_(kNonoEvent)
    ,revents_(kNonoEvent)
    ,index_(-1)
    ,tied_(false)
{

}

Channel::~Channel()
{
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        std::shared_ptr<void> guard=tie_.lock();
        if(guard!=nullptr)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_DEBUG("channel handleEvent revents:%d", revents_);

    if(revents_ & EPOLLERR)
    {
        if(errorCallback_) 
        {
            errorCallback_();
        }
    }

    if(revents_ & EPOLLHUP && !(revents_& EPOLLIN)) //對端關閉
    {
        if(closeCallback_) 
        {
            closeCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback_) 
        {
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_) 
        {
            writeCallback_();
        }
    }
}

void Channel::tie(const std::shared_ptr<void> &tie)
{
    tie_ = tie;
    tied_ = true;
}

void Channel::remove() 
{
    loop_->removeChannel(this);
}

void Channel::update()  
{
    loop_->updateChannel(this);
}


