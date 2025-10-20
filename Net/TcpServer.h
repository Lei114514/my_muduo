#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <atomic>
#include <functional>

#include "Callbacks.h"

class EventLoop;
class TcpConnection;
class Socket;
class Channel;
class Acceptor;  
class InetAddress;
class EventLoopThreadPool;

class TcpServer
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum class Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop* loop
              ,const InetAddress &listenAddr
              ,const std::string &name
              ,Option option = Option::kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback& cb){ threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);

    void start();
private:
    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;

    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConntionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionMap connections_;
    const std::string ipPort_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ThreadInitCallback threadInitCallback_;
    int numThreads_;

    int nextConnId_;

    const std::string name_;
    std::atomic_int started_;
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};