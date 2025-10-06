#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

#include "EPollPoller.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"

const int kNew=-1;
const int kAdded=1;
const int kDeleted=2;


EPollPoller::EPollPoller(EventLoop* loop)
    :Poller(loop)
    ,events_(kInitEventListSize)
    ,epollfd_(::epoll_create1(EPOLL_CLOEXEC))
{
    if(epollfd_<0)
    {
        LOG_FATAL("epoll_create1 error:%d\n",errno);
    }
}
EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs,ChannelList* activeChannels)
{
    LOG_DEBUG("epollfd=%d, total count:%d",epollfd_,events_.size());

    int numEvents=epoll_wait(epollfd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
    Timestamp now=Timestamp::now();
    int saveErrno=errno;
    if(numEvents>0)
    {   
        LOG_DEBUG("%d events happend",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if (numEvents==0)
    {
        LOG_INFO("timeout!");
    }
    else 
    {
        if(saveErrno!=EINTR)
        {
            errno=saveErrno;
            LOG_ERROR("EPollPoller::poll() error:%d\n",errno);
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    const int index=channel->index();
    LOG_DEBUG("epollfd=%d events=%d index=%d",channel->fd(),channel->events(),index);

    if(index==kNew || index==kDeleted)
    {
        if(index==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }
        else 
        {

        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else if(index==kAdded)
    {
        int fd=channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else 
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}
void EPollPoller::removeChannel(Channel* channel) 
{
    if(!hasChannel(channel))
    {
        LOG_ERROR("removeChannel: channel not exist!");
        return;
    }

    int fd=channel->fd();
    channels_.erase(fd);

    LOG_DEBUG("channel fd=%d",fd);

    int index=channel->index();
    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const
{
    for(int i=0;i<numEvents;++i)
    {
        Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation,Channel* channel)
{
    struct epoll_event event;
    ::memset(&event,0,sizeof(event));

    event.events=channel->events();
    event.data.ptr=channel;

    if(epoll_ctl(epollfd_,operation,channel->fd(),&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {
            LOG_INFO("epoll_ctl del error:%d\n",errno);
        }   
        else 
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
        }
    }
}
