#pragma once
#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;

class Acceptor: public noncopyable
{
public:
    using NewConnectionCallback=std::function<void(int sockfd,const InetAddress &)>;

    Acceptor(EventLoop* loop,const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void listen();
    bool listenning() const { return listenning_; }

    void setNewConnectionCallback(const NewConnectionCallback& cb) { NewConnectionCallback_ = cb; }
private:
    void handleRead();
    
    Socket acceptSocket_;
    EventLoop* loop_;
    Channel acceptChannel_;

    NewConnectionCallback NewConnectionCallback_;

    bool listenning_;
};