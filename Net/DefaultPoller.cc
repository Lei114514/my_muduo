#include <stdlib.h>

#include "Poller.h"
#include "EPollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        // return new PollPoller(loop); 
        return nullptr;  //這裡沒有實現PollPoller
    }
    else  
    {
        return new EPollPoller(loop);
    }
}