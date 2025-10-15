#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "InetAddress.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"
#include "Buffer.h"

class EventLoop;
class Channel;
class Socket;

class TcpConnection : private noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop
                , const std::string &nameArg
                , int sockfd
                , const InetAddress &localAddr
                , const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {return loop_;}
    const std::string &name() const {return name_;}
    const InetAddress& localAddress() const {return localAddr_;}
    const InetAddress& peerAddress() const {return peerAddr_;}

    bool connected() const { return state_ == StateE::kConnected; }

    void send(const std::string &buf);
    void sendFile(int fd,off_t offset,size_t count);

    void shutdown();

    void connectEstablished();
    void connectDestoryed();

    void setConnectionCallback(ConnectionCallback &cb) {connectionCallback_ = cb; }
    void setMessageCallback(MessageCallback &cb) { messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

private:
    enum class StateE
    {
        kDisconnectied,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void setState(StateE state) { state_ = state; }

    struct FileInfo{
        int fd;
        off_t offset;
        size_t count;
    };
    std::unique_ptr<FileInfo> fileInfo_;

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data,size_t len);
    void shutdownInLoop();
    void sendFileInLoop(int fd,off_t offset,size_t count);

    EventLoop* loop_;   
    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Socket> socket_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;   
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    
    std::atomic<StateE> state_;
    const std::string name_;
    bool reading_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
};