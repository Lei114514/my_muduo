#pragma once 

#include <functional>
#include <memory>

#include "Timestamp.h"
#include "noncopyable.h"

class EventLoop;

class Channel: public noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    int fd() const {return fd_;}
    EventLoop* ownerLoop() {return loop_;}
    void remove();
    
    //獲得event事件, 修改event事件需要使用對定的函數, 不容許自行修改
    int events() const {return event_;}
    void set_revents(int revent) {revent_ = revent;}
    int index() const {return index_;}
    void set_index(int index) {index_ = index;}

    void handleEvent(Timestamp receiveTime);

    void tie(const std::shared_ptr<void*> &);

    void enableReading() {event_ |= kReadEvent; update();}
    void enableWriting() {event_ |= kWriteEvent; update();}
    void disableReading() {event_ &= ~kReadEvent; update();}
    void disableWriting() {event_ &= ~kWriteEvent; update();}
    void disableAll() {event_ = kNonoEvent; update();}

    bool isReading() const {return event_ & kReadEvent;};
    bool isWriting() const {return event_ & kWriteEvent;};
    bool isNoneEvent() const {return event_ == kNonoEvent;}

    void setReadCallback(ReadEventCallback& cb) {readCallback_=move(cb);}
    void setWriteCallback(EventCallback& cb) {writeCallback_=move(cb);}
    void setCloseCallback(EventCallback& cb) {closeCallback_=move(cb);}
    void setErrorCallback(EventCallback& cb) {errorCallback_=move(cb);}

private:
    static const int kNonoEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    const int fd_;
    EventLoop* loop_; 

    int event_;
    int revent_;
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;
    
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;  
    EventCallback errorCallback_;
};