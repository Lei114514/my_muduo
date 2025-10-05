#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.h"

const int Channel::kNonoEvent=0;
const int Channel::kReadEvent=EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent=EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd):
    fd_(fd),
    loop_(loop),
    event_(kNonoEvent),
    revent_(kNonoEvent),
    index_(-1),
    tied_(false)
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
    if(revent_ & EPOLLERR)
    {
        if(errorCallback_) 
        {
            errorCallback_();
        }
    }

    if(revent_ & EPOLLHUP && !(revent_& EPOLLIN)) //對端關閉
    {
        if(closeCallback_) 
        {
            closeCallback_();
        }
    }

    if(revent_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback_) 
        {
            readCallback_(receiveTime);
        }
    }

    if(revent_ & EPOLLOUT)
    {
        if(writeCallback_) 
        {
            writeCallback_();
        }
    }
}

void Channel::tie(const std::shared_ptr<void*> &tie)
{
    tie_ = tie;
    tied_ = true;
}

void Channel::remove()  //待實現, 因為需要通過EventLoop來實現
{
    
}

void Channel::update()  //同上
{

}


