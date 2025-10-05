#pragma once

#include <unordered_map>
#include <vector>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

class Poller: public noncopyable
{
public:
    using ChannelList=std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    bool hasChannel(Channel* channel) const; 

    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    using channelMap=std::unordered_map<int,Channel*>;
    channelMap channels_;

private:
    EventLoop* ownerLoop_;
};